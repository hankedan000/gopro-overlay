#ifndef CVIMAGEVIEW_H
#define CVIMAGEVIEW_H

#include <QLabel>

#include <opencv2/opencv.hpp>

namespace Ui {
class CvImageView;
}

class CvImageView : public QLabel
{
    Q_OBJECT

public:
    explicit CvImageView(QWidget *parent = nullptr);
    ~CvImageView();

    void
    setImage(
            cv::Mat img);

    virtual
    void
    mouseMoveEvent(
            QMouseEvent *event) override;

    virtual
    void
    mousePressEvent(
            QMouseEvent *event) override;

    virtual
    void
    mouseReleaseEvent(
            QMouseEvent *event) override;

    virtual
    void
    enterEvent(
            QEvent *event) override;

    virtual
    void
    leaveEvent(
            QEvent *event) override;

signals:
    void
    onMouseMove(
            QMouseEvent *event);

    void
    onMousePress(
            QMouseEvent *event);

    void
    onMouseRelease(
            QMouseEvent *event);

private:
    Ui::CvImageView *ui;

};

#endif // CVIMAGEVIEW_H
