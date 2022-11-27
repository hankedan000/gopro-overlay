#include "scrubbablevideo.h"
#include "ui_scrubbablevideo.h"

ScrubbableVideo::ScrubbableVideo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScrubbableVideo),
    imgView_(new CvImageView(this))
{
    ui->setupUi(this);
    ui->mainLayout->layout()->addWidget(imgView_);
}

ScrubbableVideo::~ScrubbableVideo()
{
    delete ui;
}
