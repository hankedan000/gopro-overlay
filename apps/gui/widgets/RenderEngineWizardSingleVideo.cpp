#include "RenderEngineWizardSingleVideo.h"
#include "ui_RenderEngineWizardSingleVideo.h"

RenderEngineWizardSingleVideo::RenderEngineWizardSingleVideo(
        QWidget *parent,
        gpo::RenderProject *rPro) :
    QDialog(parent),
    ui(new Ui::RenderEngineWizardSingleVideo),
    rProj_(rPro)
{
    ui->setupUi(this);

    connect(this, &QDialog::accepted, this, [this]{
        auto &dsm = rProj_->dataSourceManager();
        QString sourceName = ui->video_ComboBox->currentText();
        gpo::DataSourcePtr data = dsm.getSourceByName(sourceName.toStdString());

        if (data)
        {
            auto newEngine = gpo::RenderEngineFactory::singleVideo(data);
            emit created(newEngine);
        }
    });
}

RenderEngineWizardSingleVideo::~RenderEngineWizardSingleVideo()
{
    delete ui;
}

void
RenderEngineWizardSingleVideo::showEvent(
        QShowEvent* event)
{
    QWidget::showEvent(event);

    ui->video_ComboBox->clear();
    const auto &dsm = rProj_->dataSourceManager();
    for (size_t ss=0; ss<dsm.sourceCount(); ss++)
    {
        auto source = dsm.getSource(ss);
        if (source->hasTelemetry() && source->hasVideo())
        {
            QString sourceName(dsm.getSourceName(ss).c_str());

            ui->video_ComboBox->addItem(sourceName);
        }
    }
}
