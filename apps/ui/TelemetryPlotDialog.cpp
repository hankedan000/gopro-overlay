#include "TelemetryPlotDialog.h"
#include "ui_TelemetryPlotDialog.h"

TelemetryPlotDialog::TelemetryPlotDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TelemetryPlotDialog)
{
    ui->setupUi(this);
}

TelemetryPlotDialog::~TelemetryPlotDialog()
{
    delete ui;
}

TelemetryPlot *
TelemetryPlotDialog::plot()
{
    return ui->plot;
}
