#include "renderthread.h"

#include "GoProOverlay/graphics/RenderEngine.h"

RenderThread::RenderThread(
        gpo::RenderProject project,
        QString exportFile,
        double fps)
 : project_(project)
 , vWriter_(exportFile.toStdString(),
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
}

void
RenderThread::stopRender()
{
    stop_ = true;
}
