#ifndef RENDERENTITYPROPERTIESTAB_H
#define RENDERENTITYPROPERTIESTAB_H

#include <QTabWidget>

#include <GoProOverlay/graphics/RenderEngine.h>

namespace Ui {
class RenderEntityPropertiesTab;
}

class RenderEntityPropertiesTab : public QTabWidget
{
    Q_OBJECT

public:
    explicit RenderEntityPropertiesTab(QWidget *parent = nullptr);
    ~RenderEntityPropertiesTab();

    void
    setEntity(
            gpo::RenderEngine::RenderedEntity *entity);

signals:
    void
    propertyChanged();

private:
    Ui::RenderEntityPropertiesTab *ui;

    gpo::RenderEngine::RenderedEntity *entity_;

};

#endif // RENDERENTITYPROPERTIESTAB_H
