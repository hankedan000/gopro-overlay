#ifndef ALIGNMENTPLOT_H
#define ALIGNMENTPLOT_H

#include <QAction>
#include <QComboBox>
#include <QWidget>

#include <GoProOverlay/data/TelemetrySource.h>
#include <GoProOverlay/graphics/TelemetryPlot.h>

namespace Ui {
class AlignmentPlot;
}

class AlignmentPlot : public QWidget
{
    Q_OBJECT

public:
    explicit AlignmentPlot(QWidget *parent = nullptr);
    ~AlignmentPlot();

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
    addY_CompToComboBox(
        QComboBox *combobox,
        TelemetryPlot::Y_Component yComp) const;

private:
    Ui::AlignmentPlot *ui;

    gpo::TelemetrySourcePtr srcA_;
    gpo::TelemetrySourcePtr srcB_;

};

#endif // ALIGNMENTPLOT_H
