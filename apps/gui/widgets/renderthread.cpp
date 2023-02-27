#include "renderthread.h"

#include <filesystem>
#include "GoProOverlay/graphics/RenderEngine.h"
#include <spdlog/spdlog.h>

const std::filesystem::path RENDER_TMP_DIR = "render_tmp/";
const std::filesystem::path RAW_RENDER_FILENAME = "raw_render.mp4";

// init static members
const std::string RenderThread::DEFAULT_EXPORT_DIR = "/tmp/gopro_overlay_render/";
const std::string RenderThread::DEFAULT_EXPORT_FILENAME = "render.mp4";

RenderThread::RenderThread(
        gpo::RenderProject *project,
        QString exportDir,
        QString exportFilename,
        double fps)
 : project_(project)
 , exportDir_(exportDir.toStdString())
 , exportFilename_(exportFilename)
 , vWriter_()
 , renderFPS_(fps)
 , stop_(false)
{
}

void
RenderThread::run()
{
    if (project_ == nullptr)
    {
        return;
    }

    auto engine = project_->getEngine();
    auto gSeeker = engine->getSeeker();

    // seek to render alignment point first
    gSeeker->seekToAlignmentInfo(project_->getAlignmentInfo());
    // start render a little bit before the alignment point (lead-in)
    gSeeker->seekAllRelativeTime(project_->getLeadInSeconds() * -1.0);// -1 to go backwards in time
    // disable bounding boxes prior to render
    for (size_t ee=0; ee<engine->entityCount(); ee++)
    {
        engine->getEntity(ee).rObj->setBoundingBoxVisible(false);
    }

    const auto seekerCount = gSeeker->seekerCount();
    std::unordered_map<std::string, double> startTimesBySource;
    spdlog::info("--- Start times by source");
    for (size_t ss=0; ss<seekerCount; ss++)
    {
        auto seeker = gSeeker->getSeeker(ss);
        auto sourceName = seeker->getDataSourceName();
        auto startTime_sec = seeker->getTimeAt(seeker->seekedIdx());
        startTimesBySource.insert({sourceName,startTime_sec});
        spdlog::info("  {0}: {1:0.6f}s",sourceName.c_str(),startTime_sec);
    }

    // create temporary directory and open export video file
    const std::filesystem::path tmpDir = exportDir_ / RENDER_TMP_DIR;
    const std::filesystem::path ffmpegLogFile = tmpDir / "ffmpeg_log.txt";
    std::filesystem::create_directories(tmpDir);
    const std::filesystem::path rawRenderFilePath = tmpDir / RAW_RENDER_FILENAME;
    vWriter_.open(
        rawRenderFilePath.c_str(),
        cv::VideoWriter::fourcc('M','P','4','V'),
        renderFPS_,
        engine->getRenderSize(),
        true);// isColor
    
    // get new limits after lead-in seeking
    auto seekLimits = gSeeker->relativeSeekLimits();
    qulonglong progress = 0;
    qulonglong total = seekLimits.second;
    while (vWriter_.isOpened() && ! stop_ && progress < total)
    {
        engine->render();
        vWriter_.write(engine->getFrame());
        gSeeker->nextAll();
        emit progressChanged(progress++,total);
    }
    vWriter_.release();

    // export final video with audio
    const std::filesystem::path finalExportFile = exportDir_ / exportFilename_.toStdString();
    switch (project_->getAudioExportApproach())
    {
        case gpo::AudioExportApproach_E::eAEA_SingleSource:
            exportAudioSingleSource(
                gSeeker,
                startTimesBySource,
                tmpDir,
                ffmpegLogFile,
                rawRenderFilePath,
                finalExportFile);
            break;
        case gpo::AudioExportApproach_E::eAEA_MultiSourceSplit:
            exportAudioMultiSourceLR(
                gSeeker,
                startTimesBySource,
                tmpDir,
                ffmpegLogFile,
                rawRenderFilePath,
                finalExportFile);
            break;
    }

    // cleanup temporary files
    std::filesystem::remove_all(tmpDir);
}

void
RenderThread::stopRender()
{
    stop_ = true;
}

bool
RenderThread::exportAudioSingleSource(
    gpo::GroupedSeekerPtr gSeeker,
    const std::unordered_map<std::string, double> &startTimesBySource,
    const std::filesystem::path &tmpDir,
    const std::filesystem::path &ffmpegLogFile,
    const std::filesystem::path &rawRenderFilePath,
    const std::filesystem::path &finalExportFile)
{
    bool audioOkay = true;
    char ffmpegCmd[10000];
    spdlog::info("exporting single-source audio...");

    const auto seekerCount = gSeeker->seekerCount();
    if (seekerCount <= 0)
    {
        spdlog::error("not enough sources to get audio");
        return false;
    }

    auto sourceForAudio = gSeeker->getSeeker(seekerCount - 1)->getDataSourceName();
    auto sourceStartTime_sec = startTimesBySource.at(sourceForAudio);
    spdlog::debug("dumping audio from source '{}'", sourceForAudio.c_str());
    auto dataSource = project_->dataSourceManager().getSourceByName(sourceForAudio);
    auto audioSourceFile = dataSource->getOrigin();
    spdlog::debug("audioSourceFile = '{}'", audioSourceFile.c_str());

    // extract audio from seeked source file using ffmpeg
    const std::filesystem::path tmpAudioPath = tmpDir / "tmp_audio.wav";
    sprintf(ffmpegCmd,"ffmpeg -ss %0.6fs -i %s -y %s > %s 2>&1",
        sourceStartTime_sec,
        audioSourceFile.c_str(),
        tmpAudioPath.c_str(),
        ffmpegLogFile.c_str());
    spdlog::debug("extracting audio...\ncmd = {}",ffmpegCmd);
    if (audioOkay && system(ffmpegCmd) != 0)
    {
        spdlog::error("failed to extract audio. ffmpegCmd = '{}'",ffmpegCmd);
        audioOkay = false;
    }

    // remux audio into raw render
    sprintf(ffmpegCmd,"ffmpeg -i %s -i %s -map 0:v:0 -map 1:a:0 -c:v copy -y %s > %s 2>&1",
        rawRenderFilePath.c_str(),
        tmpAudioPath.c_str(),
        finalExportFile.c_str(),
        ffmpegLogFile.c_str());
    spdlog::debug("remuxing audio...\ncmd = {}",ffmpegCmd);
    if (audioOkay && system(ffmpegCmd) != 0)
    {
        spdlog::error("failed to produce final export with audio. ffmpegCmd = '{}'",ffmpegCmd);
        audioOkay = false;
    }

    return audioOkay;
}

bool
RenderThread::exportAudioMultiSourceLR(
    gpo::GroupedSeekerPtr gSeeker,
    const std::unordered_map<std::string, double> &startTimesBySource,
    const std::filesystem::path &tmpDir,
    const std::filesystem::path &ffmpegLogFile,
    const std::filesystem::path &rawRenderFilePath,
    const std::filesystem::path &finalExportFile)
{
    bool audioOkay = true;
    char ffmpegCmd[10000];
    spdlog::info("exporting multi-source split audio...");

    const auto seekerCount = gSeeker->seekerCount();
    if (seekerCount < 2)
    {
        spdlog::error("not enough sources to get audio. need at least 2.");
        return false;
    }

    auto sourceForLeftAudio = gSeeker->getSeeker(0)->getDataSourceName();
    auto leftStartTime_sec = startTimesBySource.at(sourceForLeftAudio);
    spdlog::debug("dumping audio from source '{}'", sourceForLeftAudio.c_str());
    auto leftSource = project_->dataSourceManager().getSourceByName(sourceForLeftAudio);
    auto leftSourceFile = leftSource->getOrigin();
    spdlog::debug("audioSourceFile = '{}'", leftSourceFile.c_str());

    // extract left audio from seeked source file using ffmpeg
    const std::filesystem::path tmpLeftAudioPath = tmpDir / "tmp_left_audio.wav";
    sprintf(ffmpegCmd,"ffmpeg -ss %0.6fs -i %s -y %s > %s 2>&1",
        leftStartTime_sec,
        leftSourceFile.c_str(),
        tmpLeftAudioPath.c_str(),
        ffmpegLogFile.c_str());
    spdlog::debug("extracting audio...\ncmd = {}",ffmpegCmd);
    if (audioOkay && system(ffmpegCmd) != 0)
    {
        spdlog::error("failed to extract left audio. ffmpegCmd = '{}'",ffmpegCmd);
        audioOkay = false;
    }

    auto sourceForRightAudio = gSeeker->getSeeker(1)->getDataSourceName();
    auto rightStartTime_sec = startTimesBySource.at(sourceForRightAudio);
    spdlog::debug("dumping audio from source '{}'", sourceForRightAudio.c_str());
    auto rightSource = project_->dataSourceManager().getSourceByName(sourceForRightAudio);
    auto rightSourceFile = rightSource->getOrigin();
    spdlog::debug("audioSourceFile = '{}'", rightSourceFile.c_str());

    // extract right audio from seeked source file using ffmpeg
    const std::filesystem::path tmpRightAudioPath = tmpDir / "tmp_right_audio.wav";
    sprintf(ffmpegCmd,"ffmpeg -ss %0.6fs -i %s -y %s > %s 2>&1",
        rightStartTime_sec,
        rightSourceFile.c_str(),
        tmpRightAudioPath.c_str(),
        ffmpegLogFile.c_str());
    spdlog::debug("extracting audio...\ncmd = {}",ffmpegCmd);
    if (audioOkay && system(ffmpegCmd) != 0)
    {
        spdlog::error("failed to extract right audio. ffmpegCmd = '{}'",ffmpegCmd);
        audioOkay = false;
    }

    // merge audio sources into one
    const std::filesystem::path tmpMergedAudioPath = tmpDir / "tmp_audio_lr_merge.wav";
    sprintf(ffmpegCmd,"ffmpeg -i %s -i %s -filter_complex \"amerge=inputs=2,pan=stereo|c0<c0+c1|c1<c2+c3\" -y %s > %s 2>&1",
        tmpLeftAudioPath.c_str(),
        tmpRightAudioPath.c_str(),
        tmpMergedAudioPath.c_str(),
        ffmpegLogFile.c_str());
    spdlog::debug("merge left/right into one audio file...\ncmd = {}",ffmpegCmd);
    if (audioOkay && system(ffmpegCmd) != 0)
    {
        spdlog::error("failed to merge left/right audio. ffmpegCmd = '{}'",ffmpegCmd);
        audioOkay = false;
    }

    // remux audio into raw render
    sprintf(ffmpegCmd,"ffmpeg -i %s -i %s -map 0:v:0 -map 1:a:0 -c:v copy -y %s > %s 2>&1",
        rawRenderFilePath.c_str(),
        tmpMergedAudioPath.c_str(),
        finalExportFile.c_str(),
        ffmpegLogFile.c_str());
    spdlog::debug("remuxing audio...\ncmd = {}",ffmpegCmd);
    if (audioOkay && system(ffmpegCmd) != 0)
    {
        spdlog::error("failed to produce final export with audio. ffmpegCmd = '{}'",ffmpegCmd);
        audioOkay = false;
    }

    return audioOkay;
}
