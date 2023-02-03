#ifndef ALIGNMENTPLOT_H
#define ALIGNMENTPLOT_H

#include <QWidget>

#include <GoProOverlay/data/TelemetrySource.h>

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
    Ui::AlignmentPlot *ui;

    gpo::TelemetrySourcePtr srcA_;
    gpo::TelemetrySourcePtr srcB_;

};

#endif // ALIGNMENTPLOT_H
