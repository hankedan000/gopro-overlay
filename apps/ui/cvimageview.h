#ifndef CVIMAGEVIEW_H
#define CVIMAGEVIEW_H

#include <QWidget>

#include <opencv2/opencv.hpp>

namespace Ui {
class CvImageView;
}

class CvImageView : public QWidget
{
    Q_OBJECT

public:
    explicit CvImageView(QWidget *parent = nullptr);
    ~CvImageView();

    void
    setImage(
            cv::Mat img);

private:
    Ui::CvImageView *ui;
};

#endif // CVIMAGEVIEW_H
