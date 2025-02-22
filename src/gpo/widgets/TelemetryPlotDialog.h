#ifndef TELEMETRYPLOTDIALOG_H
#define TELEMETRYPLOTDIALOG_H

#include <QDialog>

#include "GoProOverlay/graphics/TelemetryPlot.h"

namespace Ui {
class TelemetryPlotDialog;
}

class TelemetryPlotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TelemetryPlotDialog(
            QWidget *parent = nullptr);

    ~TelemetryPlotDialog();

    TelemetryPlot *
    plot();

private:
    Ui::TelemetryPlotDialog *ui;
};

#endif
