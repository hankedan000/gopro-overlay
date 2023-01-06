#ifndef PLOTDIALOG_H
#define PLOTDIALOG_H

#include <QDialog>

#include "qcustomplot.h"

namespace Ui {
class PlotDialog;
}

class PlotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlotDialog(QWidget *parent = nullptr);
    ~PlotDialog();

    QCustomPlot *
    plot();

private:
    Ui::PlotDialog *ui;
};

#endif // PLOTDIALOG_H
