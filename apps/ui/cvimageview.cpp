#include "cvimageview.h"
#include "ui_cvimageview.h"

CvImageView::CvImageView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CvImageView)
{
    ui->setupUi(this);
}

CvImageView::~CvImageView()
{
    delete ui;
}

void
CvImageView::setImage(
        cv::Mat img)
{
    cv::cvtColor(img,img,cv::COLOR_BGR2RGB); //Qt reads in RGB whereas CV in BGR
    QImage imdisplay((uchar*)img.data, img.cols, img.rows, img.step, QImage::Format_RGB888); //Converts the CV image into Qt standard format
    ui->imageWidget->setPixmap(QPixmap::fromImage(imdisplay));//display the image in label that is created earlier
}
