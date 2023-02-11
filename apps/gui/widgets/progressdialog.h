#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>

#include "ProgressTimer.h"

namespace Ui {
class ProgressDialog;
}

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = nullptr);
    ~ProgressDialog();

    void
    reset();

signals:
    void
    abortPressed();

public slots:
    void
    progressChanged(
            qulonglong progress,
            qulonglong total);

private:
    Ui::ProgressDialog *ui;
    ProgressTimer timer_;
    
};

#endif // PROGRESSDIALOG_H
