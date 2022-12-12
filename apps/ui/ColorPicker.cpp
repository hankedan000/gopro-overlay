#include "ColorPicker.h"
#include "ui_ColorPicker.h"

#define QCOLOR_TO_CV_SCALAR(COLOR_) cv::Scalar(COLOR_.blue(),COLOR_.green(),COLOR_.red(),COLOR_.alpha())
#define CV_SCALAR_TO_QCOLOR(COLOR_) QColor(COLOR_[2],COLOR_[1],COLOR_[0],COLOR_[3])

ColorPicker::ColorPicker(QWidget *parent) :
    QPushButton(parent),
    ui(new Ui::ColorPicker),
    colorDialog_(new QColorDialog(this))
{
    ui->setupUi(this);
    updateButtonColor(getColor());

    connect(colorDialog_, &QColorDialog::currentColorChanged, this, [this](const QColor newColor){
        updateButtonColor(newColor);
        emit picked(newColor);
        emit pickedCV(QCOLOR_TO_CV_SCALAR(newColor));
    });
    connect(this, &QPushButton::clicked, this, [this]{
        colorDialog_->exec();
    });
}

ColorPicker::~ColorPicker()
{
    delete ui;
}

void
ColorPicker::setColor(
        QColor newColor)
{
    colorDialog_->setCurrentColor(newColor);
    updateButtonColor(newColor);
}

void
ColorPicker::setColorCV(
        cv::Scalar newColor)
{
    setColor(CV_SCALAR_TO_QCOLOR(newColor));
}

QColor
ColorPicker::getColor() const
{
    return colorDialog_->currentColor();
}

cv::Scalar
ColorPicker::getColorCV() const
{
    auto color = getColor();
    return QCOLOR_TO_CV_SCALAR(color);
}

void
ColorPicker::updateButtonColor(
        QColor newColor)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Button, newColor);
    setAutoFillBackground(true);
    setPalette(pal);
    update();
}
