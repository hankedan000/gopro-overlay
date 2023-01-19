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
        auto frameSize = getSize();
        double scale = (double)(renderSize.height) / frameSize.height;
        auto evtPosMapped = QPoint(scale * event->x(), scale * event->y());
        if (false)
        {
            printf("=======================\n");
            printf("frameSize_: w = %d; h = %d;\n",frameSize.width,frameSize.height);
            printf("renderSize: w = %d; h = %d;\n",renderSize.width,renderSize.height);
            printf("scale = %f;\n",scale);
            printf("evtPosMapped: x = %d; y = %d\n",evtPosMapped.x(),evtPosMapped.y());
            printf("event: x = %d; y = %d\n",event->x(),event->y());
            printf("=======================\n");
        }

        // this is computed such that the bounding box will be at least 1px
        // thick even in the scaled preview image size.
        unsigned int boundingBoxThickness = std::ceil(scale * 1);

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
                entity.rObj->setBoundingBoxThickness(boundingBoxThickness);
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

    setSize(cv::Size(960,540));// default size
}

ScrubbableVideo::~ScrubbableVideo()
{
    delete ui;
}

void
ScrubbableVideo::setSize(
        cv::Size size)
{
    frameBuffer_.create(size.height,size.width,CV_8UC3);

    if (isVisible() && engine_)
    {
        showImage(engine_->getFrame());
    }
}

cv::Size
ScrubbableVideo::getSize() const
{
    return cv::Size(frameBuffer_.cols,frameBuffer_.rows);
}

void
ScrubbableVideo::showImage(
        const cv::Mat &img)
{
    cv::resize(img,frameBuffer_,getSize());
    imgView_->setImage(frameBuffer_);
}

void
ScrubbableVideo::setEngine(
        gpo::RenderEnginePtr engine)
{
    engine_ = engine;
}
