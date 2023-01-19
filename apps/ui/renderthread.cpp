#include "renderthread.h"

#include <filesystem>
#include "GoProOverlay/graphics/RenderEngine.h"

const std::filesystem::path TEMP_DIR = "./render_tmp/";
const std::filesystem::path RAW_RENDER_FILENAME = "raw_render.mp4";

RenderThread::RenderThread(
        gpo::RenderProject project,
        QString exportFile,
        double fps)
 : project_(project)
 , exportFile_(exportFile)
 , vWriter_()
 , renderFPS_(fps)
 , stop_(false)
{
}

void
RenderThread::run()
{
    auto engine = project_.getEngine();
    auto gSeeker = engine->getSeeker();

    // seek to render alignment point first
    gSeeker->seekToAlignmentInfo(project_.getAlignmentInfo());
    // start render a little bit before the alignment point (lead-in)
    gSeeker->seekAllRelativeTime(project_.getLeadInSeconds() * -1.0);// -1 to go backwards in time
    // disable bounding boxes prior to render
    for (size_t ee=0; ee<engine->entityCount(); ee++)
    {
        engine->getEntity(ee).rObj->setBoundingBoxVisible(false);
    }

    const auto seekerCount = gSeeker->seekerCount();
    std::unordered_map<std::string, double> startTimesBySource;
    printf("--- Start times by source\n");
    for (size_t ss=0; ss<seekerCount; ss++)
    {
        auto seeker = gSeeker->getSeeker(ss);
        auto sourceName = seeker->getDataSourceName();
        auto startTime_sec = seeker->getTimeAt(seeker->seekedIdx());
        startTimesBySource.insert({sourceName,startTime_sec});
        printf("  %s: %0.6fs\n",sourceName.c_str(),startTime_sec);
    }

    // create temporary directory and open export video file
    std::filesystem::create_directories(TEMP_DIR);
    std::filesystem::path rawRenderFilePath = TEMP_DIR / RAW_RENDER_FILENAME;
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

    // get audio from last video source
    if (seekerCount > 0)
    {
        // 1 -> last audio source is in left & right.
        // 2 -> 1st audio source is in left. 2nd audio source is in right.
        #define AUDIO_APPROACH 1

        char ffmpegCmd[10000];
#if (AUDIO_APPROACH == 1)
        auto sourceForAudio = gSeeker->getSeeker(seekerCount - 1)->getDataSourceName();
        auto sourceStartTime_sec = startTimesBySource[sourceForAudio];
        printf("dumping audio from source '%s'\n", sourceForAudio.c_str());
        auto dataSource = project_.dataSourceManager().getSourceByName(sourceForAudio);
        auto audioSourceFile = dataSource->getOrigin();
        printf("audioSourceFile = '%s'\n", audioSourceFile.c_str());

        // extract audio from seeked source file using ffmpeg
        const std::filesystem::path tmpAudioPath = TEMP_DIR / "tmp_audio.wav";
        sprintf(ffmpegCmd,"ffmpeg -ss %0.6fs -i %s -y %s",
            sourceStartTime_sec,
            audioSourceFile.c_str(),
            tmpAudioPath.c_str());
        printf("extracting audio...\ncmd = %s\n",ffmpegCmd);
        system(ffmpegCmd);

        // remux audio into raw render
        sprintf(ffmpegCmd,"ffmpeg -i %s -i %s -map 0:v:0 -map 1:a:0 -c:v copy -y %s",
            rawRenderFilePath.c_str(),
            tmpAudioPath.c_str(),
            exportFile_.toStdString().c_str());
        printf("remuxing audio...\ncmd = %s\n",ffmpegCmd);
        system(ffmpegCmd);
# elif (AUDIO_APPROACH == 2)
        auto sourceForLeftAudio = gSeeker->getSeeker(0)->getDataSourceName();
        auto leftStartTime_sec = startTimesBySource[sourceForLeftAudio];
        printf("dumping audio from source '%s'\n", sourceForLeftAudio.c_str());
        auto leftSource = project_.dataSourceManager().getSourceByName(sourceForLeftAudio);
        auto leftSourceFile = leftSource->getOrigin();
        printf("audioSourceFile = '%s'\n", leftSourceFile.c_str());

        // extract left audio from seeked source file using ffmpeg
        const std::filesystem::path tmpLeftAudioPath = TEMP_DIR / "tmp_left_audio.wav";
        sprintf(ffmpegCmd,"ffmpeg -ss %0.6fs -i %s -y %s",
            leftStartTime_sec,
            leftSourceFile.c_str(),
            tmpLeftAudioPath.c_str());
        printf("extracting audio...\ncmd = %s\n",ffmpegCmd);
        system(ffmpegCmd);

        auto sourceForRightAudio = gSeeker->getSeeker(1)->getDataSourceName();
        auto rightStartTime_sec = startTimesBySource[sourceForRightAudio];
        printf("dumping audio from source '%s'\n", sourceForRightAudio.c_str());
        auto rightSource = project_.dataSourceManager().getSourceByName(sourceForRightAudio);
        auto rightSourceFile = rightSource->getOrigin();
        printf("audioSourceFile = '%s'\n", rightSourceFile.c_str());

        // extract right audio from seeked source file using ffmpeg
        const std::filesystem::path tmpRightAudioPath = TEMP_DIR / "tmp_right_audio.wav";
        sprintf(ffmpegCmd,"ffmpeg -ss %0.6fs -i %s -y %s",
            rightStartTime_sec,
            rightSourceFile.c_str(),
            tmpRightAudioPath.c_str());
        printf("extracting audio...\ncmd = %s\n",ffmpegCmd);
        system(ffmpegCmd);

        // merge audio sources into one
        const std::filesystem::path tmpMergedAudioPath = TEMP_DIR / "tmp_audio_lr_merge.wav";
        sprintf(ffmpegCmd,"ffmpeg -i %s -i %s -filter_complex \"amerge=inputs=2,pan=stereo|c0<c0+c1|c1<c2+c3\" -y %s",
            tmpLeftAudioPath.c_str(),
            tmpRightAudioPath.c_str(),
            tmpMergedAudioPath.c_str());
        printf("merge left/right into one audio file...\ncmd = %s\n",ffmpegCmd);
        system(ffmpegCmd);

        // remux audio into raw render
        sprintf(ffmpegCmd,"ffmpeg -i %s -i %s -map 0:v:0 -map 1:a:0 -c:v copy -y %s",
            rawRenderFilePath.c_str(),
            tmpMergedAudioPath.c_str(),
            exportFile_.toStdString().c_str());
        printf("remuxing audio...\ncmd = %s\n",ffmpegCmd);
        system(ffmpegCmd);
#endif

        // cleanup temporary files
        std::filesystem::remove_all(TEMP_DIR);
    }
}

void
RenderThread::stopRender()
{
    stop_ = true;
}
