#include "renderenginewizard_topbottom.h"
#include "ui_renderenginewizard_topbottom.h"

RenderEngineWizard_TopBottom::RenderEngineWizard_TopBottom(
        QWidget *parent,
        gpo::RenderProject *rProj) :
    QDialog(parent),
    ui(new Ui::RenderEngineWizard_TopBottom),
    rProj_(rProj)
{
    ui->setupUi(this);

    connect(this, &QDialog::accepted, this, [this]{
        auto &dsm = rProj_->dataSourceManager();
        QString topSourceName = ui->topComboBox->currentText();
        QString botSourceName = ui->botComboBox->currentText();
        gpo::DataSourcePtr topData = dsm.getSourceByName(topSourceName.toStdString());
        gpo::DataSourcePtr botData = dsm.getSourceByName(botSourceName.toStdString());

        if (topData && botData)
        {
            auto newEngine = gpo::RenderEngineFactory::topBottomAB_Compare(topData,botData);
            emit created(newEngine);
        }
    });
}

RenderEngineWizard_TopBottom::~RenderEngineWizard_TopBottom()
{
    delete ui;
}

void
RenderEngineWizard_TopBottom::showEvent(
        QShowEvent* event)
{
    QWidget::showEvent(event);

    ui->topComboBox->clear();
    ui->botComboBox->clear();
    const auto &dsm = rProj_->dataSourceManager();
    for (size_t ss=0; ss<dsm.sourceCount(); ss++)
    {
        auto source = dsm.getSource(ss);
        if (source->hasTelemetry() && source->hasVideo())
        {
            QString sourceName(dsm.getSourceName(ss).c_str());

            ui->topComboBox->addItem(sourceName);
            ui->botComboBox->addItem(sourceName);
        }
    }
}
