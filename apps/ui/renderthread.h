#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <opencv2/opencv.hpp>
#include <QThread>

#include "GoProOverlay/graphics/RenderEngine.h"

class RenderThread : public QThread
{
    Q_OBJECT

public:
    RenderThread(
            gpo::RenderEnginePtr engine,
            QString exportFile,
            double fps);

    virtual
    void
    run() override;

    void
    stopRender();

signals:
    void
    progressChanged(
            qulonglong progress,
            qulonglong total);

private:
    gpo::RenderEnginePtr engine_;
    cv::VideoWriter vWriter_;

    bool stop_;

};

#endif // RENDERTHREAD_H
