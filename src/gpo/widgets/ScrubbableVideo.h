#ifndef SCRUBBABLEVIDEO_H
#define SCRUBBABLEVIDEO_H

#include <QWidget>

#include "cvimageview.h"
#include "GoProOverlay/graphics/RenderEngine.h"

namespace Ui {
class ScrubbableVideo;
}

class ScrubbableVideo : public QWidget, private gpo::ModifiableDrawObjectObserver
{
    Q_OBJECT

public:
    static constexpr unsigned int DEFAULT_WIDTH = 960;
    static constexpr unsigned int DEFAULT_HEIGHT = 540;

    explicit ScrubbableVideo(QWidget *parent = nullptr);

    ~ScrubbableVideo();

    void
    setSize(
            cv::Size size);

    cv::Size
    getSize() const;

    void
    showImage(
            const cv::UMat &img);

    void
    setEngine(
            gpo::RenderEnginePtr engine);

signals:
    void
    onEntitySelected(
            gpo::RenderedEntity *entity);

    void
    onEntityMoved(
            gpo::RenderedEntity *entity,
            QPoint moveVector);

private:
    void
    onNeedsRedraw(
            gpo::ModifiableDrawObject *drawable) override;

private:
    Ui::ScrubbableVideo *ui;
    cv::UMat frameBuffer_;
    CvImageView *imgView_;
    gpo::RenderEnginePtr engine_;

    // entity the mouse is currently floating over
    gpo::RenderedEntityPtr focusedEntity_;
    // entity the mouse is grabbing (clicked and held)
    gpo::RenderedEntityPtr grabbedEntity_;
    // local mouse position when mouse press event occurred
    QPoint mousePosWhenGrabbed_;
    // the entities original render location when mouse press event occurred
    QPoint entityPosWhenGrabbed_;

};

#endif // SCRUBBABLEVIDEO_H
