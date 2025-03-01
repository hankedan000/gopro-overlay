#ifndef SCRUBBABLEVIDEO_H
#define SCRUBBABLEVIDEO_H

#include <QWidget>

#include "GoProOverlay/data/GroupedSeeker.h"
#include "GoProOverlay/data/ModifiableObject.h"
#include "cvimageview.h"
#include "GoProOverlay/graphics/RenderEngine.h"

namespace Ui {
    class ScrubbableVideo;
}

class ScrubbableVideo :
        public QWidget,
        private gpo::ModifiableDrawObjectObserver,
        private gpo::ModifiableObjectObserver
{
    Q_OBJECT

public:
    static constexpr unsigned int DEFAULT_WIDTH = 960;
    static constexpr unsigned int DEFAULT_HEIGHT = 540;

    explicit ScrubbableVideo(QWidget *parent = nullptr);

    ~ScrubbableVideo() override;

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

    void
    onModified(
        gpo::ModifiableObject *modifiable) override;

    void
    updateScrubBarRange(
        const gpo::GroupedSeekerPtr & gSeeker);

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

    // stores the value of the scrubBar from the previous onValueChanged() signal.
    // used to compute a relative offset to advance the engine's GroupedSeeker.
    int prevScrubPosition_ = 0;

    // true if the scrubBar value update is from an internal change to the scrubBar's
    // range and/or vale. this avoids a double relative seek when we sync the scrub
    // bar position within the GroupedSeeker's onModified() callback.
    bool isInternalScrubUpdate_ = false;

};

#endif // SCRUBBABLEVIDEO_H
