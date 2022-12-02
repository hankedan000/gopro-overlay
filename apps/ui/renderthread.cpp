#include "renderthread.h"

RenderThread::RenderThread(
        gpo::RenderEnginePtr engine,
        QString exportFile,
        double fps)
 : engine_(engine)
 , vWriter_(exportFile.toStdString(),
            cv::VideoWriter::fourcc('M','P','4','V'),
            fps,
            engine->getRenderSize(),
            true)// isColor
 , stop_(false)
{
}

void
RenderThread::run()
{
    auto gSeeker = engine_->getSeeker();
    auto seekLimits = gSeeker->relativeSeekLimits();
    qulonglong progress = 0;
    qulonglong total = seekLimits.second;
    while ( ! stop_ && progress < total)
    {
        engine_->render();
        vWriter_.write(engine_->getFrame());
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
