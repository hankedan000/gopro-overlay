#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QColorDialog>
#include <QPushButton>

#include <opencv2/core/types.hpp> // for cv::Size

namespace Ui {
class ColorPicker;
}

class ColorPicker final : public QPushButton
{
    Q_OBJECT

public:
    explicit ColorPicker(QWidget *parent = nullptr);

    ~ColorPicker() final;

    void
    setColor(
            QColor newColor);

    void
    setColorCV(
            cv::Scalar newColor);

    QColor
    getColor() const;

    cv::Scalar
    getColorCV() const;

signals:
    void
    picked(
            QColor newColor);

    void
    pickedCV(
            cv::Scalar newColor);

private:
    void
    updateButtonColor(
            QColor newColor);

private:
    Ui::ColorPicker *ui;
    QColorDialog *colorDialog_;

};

#endif // COLORPICKER_H
