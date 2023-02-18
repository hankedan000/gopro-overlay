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
    setDragAndZoomEnabled(
        bool enabled);
    
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
