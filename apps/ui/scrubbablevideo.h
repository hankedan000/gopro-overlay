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
    gpo::RenderEngine::RenderedEntity *focusedEntity_;

};

#endif // SCRUBBABLEVIDEO_H
