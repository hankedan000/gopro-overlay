#ifndef RENDERENGINEWIZARD_TOPBOTTOM_H
#define RENDERENGINEWIZARD_TOPBOTTOM_H

#include <QDialog>

#include "GoProOverlay/data/RenderProject.h"
#include "GoProOverlay/graphics/RenderEngine.h"

namespace Ui {
class RenderEngineWizard_TopBottom;
}

class RenderEngineWizard_TopBottom : public QDialog
{
    Q_OBJECT

public:
    explicit RenderEngineWizard_TopBottom(
            QWidget *parent,
            gpo::RenderProject *rProj);

    ~RenderEngineWizard_TopBottom();

    virtual
    void
    showEvent(
            QShowEvent* event) override;

signals:
    void
    created(
            gpo::RenderEnginePtr newEngine);

private:
    Ui::RenderEngineWizard_TopBottom *ui;
    gpo::RenderProject *rProj_;

};

#endif // RENDERENGINEWIZARD_TOPBOTTOM_H
