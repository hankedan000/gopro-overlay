#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <opencv2/opencv.hpp>
#include <QThread>

#include "GoProOverlay/data/RenderProject.h"

class RenderThread : public QThread
{
    Q_OBJECT

public:
    RenderThread(
            gpo::RenderProject project,
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
    gpo::RenderProject project_;
    cv::VideoWriter vWriter_;
    double renderFPS_;

    bool stop_;

};

#endif // RENDERTHREAD_H
