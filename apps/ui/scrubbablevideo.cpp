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

        printf("next!\n");
        engine_->getSeeker()->next();
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
    frameBuffer_.create(size.height,size.width,CV_8UC3);
}

void
ScrubbableVideo::showImage(
        const cv::Mat &img)
{
    cv::resize(img,frameBuffer_,frameBuffer_.size());
    imgView_->setImage(frameBuffer_);
}

void
ScrubbableVideo::setEngine(
        gpo::RenderEnginePtr engine)
{
    engine_ = engine;
}
