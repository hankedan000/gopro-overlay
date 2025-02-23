#ifndef TELEMETRYPLOTDIALOG_H
#define TELEMETRYPLOTDIALOG_H

#include <QDialog>

#include "GoProOverlay/graphics/QTelemetryPlot.h"

namespace Ui {
class TelemetryPlotDialog;
}

class TelemetryPlotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TelemetryPlotDialog(
            QWidget *parent = nullptr);

    ~TelemetryPlotDialog() override;

    QTelemetryPlot *
    plot();

private:
    Ui::TelemetryPlotDialog *ui;
};

#endif
