#include "scrubbablevideo.h"
#include "ui_scrubbablevideo.h"

ScrubbableVideo::ScrubbableVideo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScrubbableVideo),
    imgView_(new CvImageView(this)),
    engine_(nullptr)
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
