#ifndef TELEMETRYPLOTDIALOG_H
#define TELEMETRYPLOTDIALOG_H

#include <QWidget>
#include <vector>

#include "PlotDialog.h"

#include "GoProOverlay/data/DataSource.h"

class TelemetryPlotDialog : public PlotDialog
{
    Q_OBJECT

public:
    enum TelemetryComponent
    {
        eTC_Unknown,
        eTC_Time,
        eTC_AcclX,eTC_AcclY,eTC_AcclZ,
        eTC_GyroX,eTC_GyroY,eTC_GyroZ
    };

private:
    static constexpr Qt::GlobalColor DEFAULT_COLORS[] = {
        Qt::red,
        Qt::green,
        Qt::blue,
        Qt::yellow,
        Qt::magenta,
        Qt::cyan,
        Qt::black,
        Qt::gray,
    };
    static constexpr size_t N_DEFAULT_COLORS = sizeof(DEFAULT_COLORS) / sizeof(DEFAULT_COLORS[0]);

    struct SourceObjects
    {
        gpo::TelemetrySourcePtr telemSrc;

        // QCustomPlot's x/y data
        QVector<double> xData;
        QVector<double> yData;

        // QCustomPlot graph for this telemetry data source
        QCPGraph *graph;
    };

public:
    explicit TelemetryPlotDialog(QWidget *parent = nullptr);
    ~TelemetryPlotDialog();

    void
    addSource(
            gpo::TelemetrySourcePtr telemSrc,
            bool replot = true);

    void
    removeSource(
            size_t idx,
            bool replot = true);

    void
    clear(
          bool replot = true);

    gpo::TelemetrySourcePtr
    getSource(
            size_t idx);

    size_t
    numSources() const;

    void
    setTelemetryComponent(
            TelemetryComponent comp,
            bool replot = true);

private:
    void
    setX_Data(
            SourceObjects &sourceObjs,
            bool isTime);

    void
    setY_Data(
            SourceObjects &sourceObjs,
            TelemetryComponent comp);

private:
    TelemetryComponent telemComponent_;
    std::vector<SourceObjects> sources_;

};

#endif // TELEMETRYPLOTDIALOG_H
