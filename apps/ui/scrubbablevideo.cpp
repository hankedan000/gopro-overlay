#include "scrubbablevideo.h"
#include "ui_scrubbablevideo.h"

#include <QMouseEvent>

ScrubbableVideo::ScrubbableVideo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScrubbableVideo),
    imgView_(new CvImageView(this)),
    engine_(nullptr),
    focusedEntity_(nullptr)
{
    ui->setupUi(this);
    ui->mainLayout->layout()->addWidget(imgView_);

    // connect button signals
    connect(ui->nextFrameButton, &QToolButton::pressed, this, [this]{
        if ( ! engine_)
        {
            return;
        }

        engine_->getSeeker()->nextAll();
        engine_->render();
        showImage(engine_->getFrame());
    });
    connect(ui->prevFrameButton, &QToolButton::pressed, this, [this]{
        if ( ! engine_)
        {
            return;
        }

        engine_->getSeeker()->prevAll();
        engine_->render();
        showImage(engine_->getFrame());
    });

    // CvImageView mouse event handlers
    connect(imgView_, &CvImageView::onMouseMove, this, [this](QMouseEvent *event){
        if (engine_ == nullptr)
        {
            return;
        }

        // video preview displays a scaled version of the engine's rendered image.
        // for the mouse event location to be comparable, we need to map it into
        // render image's coordinate space.
        auto renderSize = engine_->getRenderSize();
        double scale = (double)(renderSize.height) / frameSize_.height;
        auto evtPosMapped = QPoint(scale * event->x(), scale * event->y());

        bool mouseGrabbed = false;
        auto prevFocusedEntity = focusedEntity_;
        // iterate backwards because we entity that are rendered above others
        // should grab the mouse first
        for (int e=(engine_->entityCount()-1); e>=0; e--)
        {
            auto &entity = engine_->getEntity(e);
            if ( ! mouseGrabbed &&
                evtPosMapped.x() >= entity.rPos.x && evtPosMapped.x() <= (entity.rPos.x + entity.rSize.width) &&
                evtPosMapped.y() >= entity.rPos.y && evtPosMapped.y() <= (entity.rPos.y + entity.rSize.height))
            {
                focusedEntity_ = &entity;
                entity.rObj->setBoundingBoxVisible(true);
                mouseGrabbed = true;
            }
            else
            {
                entity.rObj->setBoundingBoxVisible(false);
            }
        }

        if ( ! mouseGrabbed)
        {
            focusedEntity_ = nullptr;
        }
        // if focuse changed, trigger a re-render
        if (prevFocusedEntity != focusedEntity_)
        {
            engine_->render();
            showImage(engine_->getFrame());
        }
    });
    connect(imgView_, &CvImageView::onMousePress, this, [this](QMouseEvent *event){
    });
    connect(imgView_, &CvImageView::onMouseRelease, this, [this](QMouseEvent *event){
    });

    setSize(cv::Size(640,480));// default size
}

ScrubbableVideo::~ScrubbableVideo()
{
    delete ui;
}

void
ScrubbableVideo::setSize(
        cv::Size size)
{
    frameSize_ = size;
    frameBuffer_.create(size.height,size.width,CV_8UC3);

    if (isVisible() && engine_)
    {
        showImage(engine_->getFrame());
    }
}

void
ScrubbableVideo::showImage(
        const cv::Mat &img)
{
    cv::Size imgSize = img.size();
    // width over height (WOH) ratio:
    //  * if WOH is <1.0 then image is taller than it is wide
    //  * if WOH is >1.0 then image is wider than it is tall
    double imgWOH = (double)(imgSize.width) / imgSize.height;
    double frameWOH = (double)(frameSize_.width) / frameSize_.height;
    cv::Size newSize;
    if (imgWOH > frameWOH)
    {
        newSize.width = frameSize_.width;
        newSize.height = frameSize_.width / imgWOH;
    }
    else
    {
        newSize.width = frameSize_.height * imgWOH;
        newSize.height = frameSize_.height;
    }

    cv::resize(img,frameBuffer_,newSize);
    imgView_->setImage(frameBuffer_);
}

void
ScrubbableVideo::setEngine(
        gpo::RenderEnginePtr engine)
{
    engine_ = engine;
}
