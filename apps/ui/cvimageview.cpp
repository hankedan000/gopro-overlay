#include "cvimageview.h"
#include "ui_cvimageview.h"

#include <QMouseEvent>

CvImageView::CvImageView(QWidget *parent) :
    QLabel(parent),
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
    setPixmap(QPixmap::fromImage(imdisplay));//display the image in label that is created earlier
}

void
CvImageView::mouseMoveEvent(
        QMouseEvent *event)
{
    emit onMouseMove(event);
}

void
CvImageView::mousePressEvent(
        QMouseEvent *event)
{
    emit onMousePress(event);
}

void
CvImageView::mouseReleaseEvent(
        QMouseEvent *event)
{
    emit onMouseRelease(event);
}

void
CvImageView::enterEvent(
        QEvent *event)
{
    setMouseTracking(true);
}

void
CvImageView::leaveEvent(
        QEvent *event)
{
    setMouseTracking(false);
}
