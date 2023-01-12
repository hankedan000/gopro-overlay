#include "progressdialog.h"
#include "ui_progressdialog.h"

#include <QPushButton>

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog),
    timer_()
{
    ui->setupUi(this);

    auto *abortButton = ui->buttonBox->button(QDialogButtonBox::Abort);
    if (abortButton)
    {
        connect(abortButton, &QPushButton::clicked, this, [this]{
            emit abortPressed();
        });
    }
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void
ProgressDialog::reset()
{
    timer_.reset();
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(0);
    ui->infoTextEdit->document()->setPlainText("");
}

void
ProgressDialog::progressChanged(
        qulonglong progress,
        qulonglong total)
{
    timer_.progress(progress,total);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(total);
    ui->progressBar->setValue(progress);

    const auto &stats = timer_.stats();
    ui->infoTextEdit->document()->setPlainText(
                QStringLiteral("frames: %1/%2\nrate: %3%4\neta: %5s")
                    .arg(progress)
                    .arg(total)
                    .arg(stats.avg_rate_hz/stats.rate_div,0,'f',3)
                    .arg(stats.rate_unit.c_str())
                    .arg(stats.eta_s,0,'f',3));
}
