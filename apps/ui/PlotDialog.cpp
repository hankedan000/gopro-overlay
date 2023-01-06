#include "PlotDialog.h"
#include "ui_PlotDialog.h"

PlotDialog::PlotDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlotDialog)
{
    ui->setupUi(this);
}

PlotDialog::~PlotDialog()
{
    delete ui;
}

QCustomPlot *
PlotDialog::plot()
{
    return ui->plot;
}
