#ifndef RENDERENTITYPROPERTIESTAB_H
#define RENDERENTITYPROPERTIESTAB_H

#include <QStandardItemModel>
#include <QTabWidget>

#include <GoProOverlay/data/RenderProject.h>
#include <GoProOverlay/graphics/RenderEngine.h>

namespace Ui {
class RenderEntityPropertiesTab;
}

class RenderEntityPropertiesTab : public QTabWidget
{
    Q_OBJECT

public:
    explicit RenderEntityPropertiesTab(
            QWidget *parent = nullptr);
    ~RenderEntityPropertiesTab();

    void
    setProject(
            gpo::RenderProject *project);

    void
    setEntity(
            gpo::RenderedEntityPtr entity);

signals:
    void
    propertyChanged();

private:
    Ui::RenderEntityPropertiesTab *ui;

    gpo::RenderProject *project_;

    gpo::RenderedEntityPtr entity_;

};

#endif // RENDERENTITYPROPERTIESTAB_H
