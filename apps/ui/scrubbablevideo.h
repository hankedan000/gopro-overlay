#ifndef SCRUBBABLEVIDEO_H
#define SCRUBBABLEVIDEO_H

#include <QWidget>

#include "cvimageview.h"

namespace Ui {
class ScrubbableVideo;
}

class ScrubbableVideo : public QWidget
{
    Q_OBJECT

public:
    explicit ScrubbableVideo(QWidget *parent = nullptr);
    ~ScrubbableVideo();

private:
    Ui::ScrubbableVideo *ui;
    CvImageView *imgView_;
};

#endif // SCRUBBABLEVIDEO_H
