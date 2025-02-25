#include "scrubbablevideo.h"
#include "ui_scrubbablevideo.h"

#include <opencv2/imgproc.hpp>
#include <QMouseEvent>
#include <spdlog/spdlog.h>

ScrubbableVideo::ScrubbableVideo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScrubbableVideo),
    imgView_(new CvImageView(this)),
    engine_(nullptr),
    focusedEntity_(nullptr),
    grabbedEntity_(nullptr)
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
        // render image's coordinate space. (RoF - Render over Frame)
        auto renderSize = engine_->getRenderSize();
        auto frameSize = getSize();
        double scaleFactorRoF = (double)(renderSize.height) / frameSize.height;
        auto evtPosMapped = QPoint(scaleFactorRoF * event->x(), scaleFactorRoF * event->y());
        spdlog::debug("=======================");
        spdlog::debug("frameSize_: w = {}; h = {};",frameSize.width,frameSize.height);
        spdlog::debug("renderSize: w = {}; h = {};",renderSize.width,renderSize.height);
        spdlog::debug("scaleFactorRoF = {};",scaleFactorRoF);
        spdlog::debug("evtPosMapped: x = {}; y = {}",evtPosMapped.x(),evtPosMapped.y());
        spdlog::debug("event: x = {}; y = {}",event->x(),event->y());
        spdlog::debug("=======================");

        bool rerender = false;

        if (grabbedEntity_ == nullptr)
        {
            // this is computed such that the bounding box will be at least 1px
            // thick even in the scaled preview image size.
            unsigned int boundingBoxThickness = std::ceil(scaleFactorRoF * 1);

            // not holding an entity yet, so perform mouse focusing logic
            bool focusAcquired = false;
            auto prevFocusedEntity = focusedEntity_;
            // iterate backwards because entities that are rendered above others
            // should grab the mouse first
            for (int e=(engine_->entityCount()-1); e>=0; e--)
            {
                const auto &entity = engine_->getEntity(e);
                if ( ! entity->renderObject()->isVisible())
                {
                    continue;
                }

                if ( ! focusAcquired &&
                    evtPosMapped.x() >= entity->renderPosition().x && evtPosMapped.x() <= (entity->renderPosition().x + entity->renderSize().width) &&
                    evtPosMapped.y() >= entity->renderPosition().y && evtPosMapped.y() <= (entity->renderPosition().y + entity->renderSize().height))
                {
                    focusedEntity_ = entity;
                    entity->renderObject()->setBoundingBoxVisible(true);
                    entity->renderObject()->setBoundingBoxThickness(boundingBoxThickness);
                    focusAcquired = true;
                }
                else
                {
                    entity->renderObject()->setBoundingBoxVisible(false);
                }
            }

            if ( ! focusAcquired)
            {
                focusedEntity_ = nullptr;
            }

            // trigger a rerender if focus changed
            rerender = rerender || prevFocusedEntity != focusedEntity_;
        }
        else
        {
            // holding an entity, so move entity based on mouse location
            QPoint mouseDelta = event->pos() - mousePosWhenGrabbed_;
            QPoint entityNewPos = entityPosWhenGrabbed_ + mouseDelta * scaleFactorRoF;
            grabbedEntity_->setRenderPosition(entityNewPos.x(), entityNewPos.y());
            rerender = true;
        }

        // perform preview rerender
        if (rerender)
        {
            engine_->render();
            showImage(engine_->getFrame());
        }
    });
    connect(imgView_, &CvImageView::onMousePress, this, [this](QMouseEvent *event){
        if (focusedEntity_)
        {
            grabbedEntity_ = focusedEntity_;
            mousePosWhenGrabbed_ = event->pos();
            entityPosWhenGrabbed_.setX(focusedEntity_->renderPosition().x);
            entityPosWhenGrabbed_.setY(focusedEntity_->renderPosition().y);
            emit onEntitySelected(grabbedEntity_.get());
        }
    });
    connect(imgView_, &CvImageView::onMouseRelease, this, [this](QMouseEvent *event){
        if (grabbedEntity_)
        {
            // determine in the entity was moved while it was being held
            QPoint moveVector(grabbedEntity_->renderPosition().x,grabbedEntity_->renderPosition().y);
            moveVector -= entityPosWhenGrabbed_;
            if (moveVector.manhattanLength() > 0)
            {
                emit onEntityMoved(grabbedEntity_.get(),moveVector);
            }

            grabbedEntity_ = nullptr;
        }
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
        const cv::UMat &img)
{
    cv::resize(img,frameBuffer_,getSize());
    imgView_->setImage(frameBuffer_);
}

void
ScrubbableVideo::setEngine(
        gpo::RenderEnginePtr engine)
{
    if (engine_)
    {
        // no longer observe the old engine
        engine_->removeObserver(this);
    }

    engine_ = engine;
    // release focused/grabbed entities that belonged to old engine
    focusedEntity_ = nullptr;
    grabbedEntity_ = nullptr;

    if (engine_)
    {
        // begin observing the new engine
        engine_->addObserver(this);
    }
}

void
ScrubbableVideo::onNeedsRedraw(
        gpo::ModifiableDrawObject *drawable)
{
    showImage(engine_->getFrame());
}
