#include "TelemetryPlotDialog.h"
#include "ui_TelemetryPlotDialog.h"

TelemetryPlotDialog::TelemetryPlotDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TelemetryPlotDialog)
{
    ui->setupUi(this);
    plot()->applyDarkTheme();
}

TelemetryPlotDialog::~TelemetryPlotDialog()
{
    delete ui;
}

QTelemetryPlot *
TelemetryPlotDialog::plot()
{
    return ui->plot;
}
