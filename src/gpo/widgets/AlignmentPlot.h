#ifndef ALIGNMENTPLOT_H
#define ALIGNMENTPLOT_H

#include <QAction>
#include <QComboBox>
#include <QWidget>

#include <GoProOverlay/data/TelemetrySource.h>
#include <GoProOverlay/graphics/TelemetryPlotTypes.h>

namespace Ui {
class AlignmentPlot;
}

class AlignmentPlot : public QWidget
{
    Q_OBJECT

public:
    explicit AlignmentPlot(QWidget *parent = nullptr);

    ~AlignmentPlot() override;

    void
    setSourceA(
        gpo::TelemetrySourcePtr tSrc);

    void
    setSourceB(
        gpo::TelemetrySourcePtr tSrc);

private:
    void
    populateComboBox(
        QComboBox *combobox,
        gpo::TelemetrySourcePtr tSrc) const;

    void
    setDragAndZoomEnabled(
        bool enabled);

    void
    setBestY_Components();

    /**
     * Selects item in the ComboBox corresponding to the Y_Component
     * @return
     * true if an item was selected. false otherwise.
     */
    bool
    selectComboBoxY_Comp(
        QComboBox *combobox,
        gpo::TelemetryPlot::Y_Component yComp);
    
private:
    Ui::AlignmentPlot *ui;

    gpo::TelemetrySourcePtr srcA_;
    gpo::TelemetrySourcePtr srcB_;

    gpo::TelemetrySourcePtr selectedSrc_;
    // true if 'selectedSrc_' is held by mouse currently
    bool selectedSrcHeld_;
    // location where the mouse was pressed
    QPointF mousePressPos_px_;
    // alignment index of selected source when mouse was pressed
    int64_t mousePressAlignIdx_;
    bool mousePressed_;
    bool mouseMoved_;

};

#endif // ALIGNMENTPLOT_H
