#include "progressdialog.h"
#include "ui_progressdialog.h"

#include <QPushButton>

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
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
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(total);
    ui->progressBar->setValue(progress);

    ui->infoTextEdit->document()->setPlainText(
                QStringLiteral("frames: %1/%2")
                    .arg(progress)
                    .arg(total));
}
