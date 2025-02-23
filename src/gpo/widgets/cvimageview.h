#ifndef CVIMAGEVIEW_H
#define CVIMAGEVIEW_H

#include <QLabel>

#include <opencv2/core/mat.hpp>

namespace Ui {
class CvImageView;
}

class CvImageView : public QLabel
{
    Q_OBJECT

public:
    explicit CvImageView(QWidget *parent = nullptr);

    ~CvImageView() override;

    void
    setImage(
            cv::UMat img);

    void
    mouseMoveEvent(
            QMouseEvent *event) override;

    void
    mousePressEvent(
            QMouseEvent *event) override;

    void
    mouseReleaseEvent(
            QMouseEvent *event) override;

    void
    enterEvent(
            QEvent *event) override;

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
