#include "renderthread.h"

#include "GoProOverlay/graphics/RenderEngine.h"

const std::string RAW_RENDER_FILENAME = "raw_render.mp4";

RenderThread::RenderThread(
        gpo::RenderProject project,
        QString exportFile,
        double fps)
 : project_(project)
 , exportFile_(exportFile)
 , vWriter_(RAW_RENDER_FILENAME,
            cv::VideoWriter::fourcc('M','P','4','V'),
            fps,
            project.getEngine()->getRenderSize(),
            true)// isColor
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

    // get new limits after lead-in seeking
    auto seekLimits = gSeeker->relativeSeekLimits();
    qulonglong progress = 0;
    qulonglong total = seekLimits.second;
    while ( ! stop_ && progress < total)
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
        auto sourceForAudio = gSeeker->getSeeker(seekerCount - 1)->getDataSourceName();
        auto sourceStartTime_sec = startTimesBySource[sourceForAudio];
        printf("dumping audio from source '%s'\n", sourceForAudio.c_str());
        auto dataSource = project_.dataSourceManager().getSourceByName(sourceForAudio);
        auto audioSourceFile = dataSource->getOrigin();
        printf("audioSourceFile = '%s'\n", audioSourceFile.c_str());

        // extract audio from seeked source file using ffmpeg
        char ffmpegCmd[2048];
        sprintf(ffmpegCmd,"ffmpeg -ss %0.6fs -i %s -y tmp_audio.wav",
               sourceStartTime_sec,
               audioSourceFile.c_str());
        printf("extracting audio...\ncmd = %s\n",ffmpegCmd);
        system(ffmpegCmd);

        // remux audio into raw render
        sprintf(ffmpegCmd,"ffmpeg -i %s -i tmp_audio.wav -map 0:v:0 -map 1:a:0 -c:v copy -y %s",
               RAW_RENDER_FILENAME.c_str(),
               exportFile_.toStdString().c_str());
        printf("remuxing audio...\ncmd = %s\n",ffmpegCmd);
        system(ffmpegCmd);
    }
}

void
RenderThread::stopRender()
{
    stop_ = true;
}
