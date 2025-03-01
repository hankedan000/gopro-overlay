#include "ScrubbableVideo.h"
#include "cvimageview.h"
#include "ui_ScrubbableVideo.h"

#include <opencv2/imgproc.hpp>
#include <QMouseEvent>

ScrubbableVideo::ScrubbableVideo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScrubbableVideo),
    imgView_(new CvImageView(this))
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
    });
    connect(ui->prevFrameButton, &QToolButton::pressed, this, [this]{
        if ( ! engine_)
        {
            return;
        }

        engine_->getSeeker()->prevAll();
    });

    connect(ui->scrubSlider, &QSlider::valueChanged, this, [this](int newScrubPosition){
        if (isInternalScrubUpdate_)
        {
            isInternalScrubUpdate_ = false;
            return;
        }
        else if ( ! engine_)
        {
            return;
        }

        // compute seek direction and # of frames to seek by
        auto seekDir = gpo::SeekDirection::Forward;
        size_t seekAmount = 0;
        if (newScrubPosition > prevScrubPosition_)
        {
            seekAmount = newScrubPosition - prevScrubPosition_;
        }
        else
        {
            seekDir = gpo::SeekDirection::Backward;
            seekAmount = prevScrubPosition_ - newScrubPosition;
        }

        // seek to new position (rerender will automatically be queued upon modification)
        engine_->getSeeker()->seekAllRelative(seekAmount, seekDir);
        prevScrubPosition_ = newScrubPosition;
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
        const auto renderSize = engine_->getRenderSize();
        const auto frameSize = getSize();
        const double scaleFactorRoF = static_cast<double>(renderSize.height) / frameSize.height;
        const auto evtPosMapped = QPoint(
            static_cast<int>(scaleFactorRoF * event->x()),
            static_cast<int>(scaleFactorRoF * event->y()));

        bool rerender = false;

        if (grabbedEntity_ == nullptr)
        {
            // not holding an entity yet, so perform mouse focusing logic
            bool focusAcquired = false;
            const auto prevFocusedEntity = focusedEntity_;
            // iterate backwards because entities that are rendered above others
            // should grab the mouse first
            for (auto e=static_cast<ssize_t>(engine_->entityCount()-1); e>=0; e--)
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
                    // this is computed such that the bounding box will be at least 1px
                    // thick even in the scaled preview image size.
                    const auto thickness = static_cast<unsigned int>(std::ceil(scaleFactorRoF * 1));
                    
                    focusedEntity_ = entity;
                    entity->renderObject()->setBoundingBoxVisible(true);
                    entity->renderObject()->setBoundingBoxThickness(thickness);
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
            const QPoint mouseDelta = event->pos() - mousePosWhenGrabbed_;
            const QPoint entityNewPos = entityPosWhenGrabbed_ + mouseDelta * scaleFactorRoF;
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
            // determine if the entity was moved while it was being held
            QPoint moveVector(grabbedEntity_->renderPosition().x,grabbedEntity_->renderPosition().y);
            moveVector -= entityPosWhenGrabbed_;
            if (moveVector.manhattanLength() > 0)
            {
                emit onEntityMoved(grabbedEntity_.get(),moveVector);
            }

            grabbedEntity_ = nullptr;
        }
    });

    setSize(cv::Size(DEFAULT_WIDTH,DEFAULT_HEIGHT));
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
        // no longer observe the old engine and its seeker
        engine_->removeObserver(static_cast<gpo::ModifiableDrawObjectObserver *>(this));
        engine_->getSeeker()->removeObserver(this);
    }

    engine_ = engine;
    // release focused/grabbed entities that belonged to old engine
    focusedEntity_ = nullptr;
    grabbedEntity_ = nullptr;

    if (engine_)
    {
        // observe new engine for changes so we can rerender when it's modified
        engine_->addObserver(static_cast<gpo::ModifiableDrawObjectObserver *>(this));

        // observe new GroupedSeeker so we can update our scrubBar's range
        auto gSeeker = engine_->getSeeker();
        gSeeker->addObserver(this);
        updateScrubBarRange(gSeeker);// force update to scrubBar now
    }
}

void
ScrubbableVideo::onNeedsRedraw(
    gpo::ModifiableDrawObject *drawable)
{
    showImage(engine_->getFrame());
}

void
ScrubbableVideo::onModified(
    gpo::ModifiableObject * /*modifiable*/)
{
    // only modifiable object we observe is the GroupedSeeker, so update
    // the scrubBar's range to match any potential modifications
    updateScrubBarRange(engine_->getSeeker());
}

void
ScrubbableVideo::updateScrubBarRange(
    const gpo::GroupedSeekerPtr & gSeeker)
{
    const auto limits = gSeeker->relativeSeekLimits();
    const auto newRange = limits.totalRange();

    // update with new range and scrub position
    isInternalScrubUpdate_ = true;
    ui->scrubSlider->setRange(0, static_cast<int>(newRange));
    ui->scrubSlider->setValue(static_cast<int>(limits.backwards));
    prevScrubPosition_ = static_cast<int>(limits.backwards);
}
