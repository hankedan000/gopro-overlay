#ifndef SCRUBBABLEVIDEO_H
#define SCRUBBABLEVIDEO_H

#include <QWidget>

#include "cvimageview.h"
#include "GoProOverlay/graphics/RenderEngine.h"

namespace Ui {
class ScrubbableVideo;
}

class ScrubbableVideo : public QWidget
{
    Q_OBJECT

public:
    explicit ScrubbableVideo(QWidget *parent = nullptr);

    ~ScrubbableVideo();

    void
    setSize(
            cv::Size size);

    cv::Size
    getSize() const;

    void
    showImage(
            const cv::Mat &img);

    void
    setEngine(
            gpo::RenderEnginePtr engine);

private:
    Ui::ScrubbableVideo *ui;
    cv::Mat frameBuffer_;
    CvImageView *imgView_;
    gpo::RenderEnginePtr engine_;

    // entity the mouse is currently floating over
    gpo::RenderEngine::RenderedEntity *focusedEntity_;
    // entity the mouse is grabbing (clicked and held)
    gpo::RenderEngine::RenderedEntity *grabbedEntity_;
    // local mouse position when mouse press event occurred
    QPoint mousePosWhenGrabbed_;
    // the entities original render location when mouse press event occurred
    QPoint entityPosWhenGrabbed_;

};

#endif // SCRUBBABLEVIDEO_H
