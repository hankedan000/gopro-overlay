#ifndef RENDERENGINEWIZARDSINGLEVIDEO_H
#define RENDERENGINEWIZARDSINGLEVIDEO_H

#include <QDialog>

#include "GoProOverlay/data/RenderProject.h"
#include "GoProOverlay/graphics/RenderEngine.h"

namespace Ui {
class RenderEngineWizardSingleVideo;
}

class RenderEngineWizardSingleVideo : public QDialog
{
    Q_OBJECT

public:
    explicit RenderEngineWizardSingleVideo(
            QWidget *parent,
            gpo::RenderProject *rProj);

    ~RenderEngineWizardSingleVideo();

    virtual
    void
    showEvent(
            QShowEvent* event) override;

signals:
    void
    created(
            gpo::RenderEnginePtr newEngine);

private:
    Ui::RenderEngineWizardSingleVideo *ui;
    gpo::RenderProject *rProj_;
};

#endif // RENDERENGINEWIZARDSINGLEVIDEO_H
