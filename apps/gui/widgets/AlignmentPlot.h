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

    void
    setBestY_Components();

    /**
     * Checks to see if source A/B are capable of providing the
     * specific Y_Components. If so, then it sets the components on
     * the plots.
     * 
     * @return
     * true if the sensor data was available and the Y_Components were set.
     */
    bool
    testAndSetY_Components(
        gpo::DataAvailableBitSet aAvail,
        gpo::DataAvailableBitSet bAvail);
    
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
