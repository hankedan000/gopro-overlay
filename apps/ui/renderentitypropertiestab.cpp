#include "renderentitypropertiestab.h"
#include "ui_renderentitypropertiestab.h"

RenderEntityPropertiesTab::RenderEntityPropertiesTab(QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::RenderEntityPropertiesTab),
    entity_(nullptr)
{
    ui->setupUi(this);
    setEntity(entity_);

    connect(ui->entityPositionX_spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int newValue){
        if (entity_)
        {
            entity_->rPos.x = newValue;
            emit propertyChanged();
        }
    });
    connect(ui->entityPositionY_spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int newValue){
        if (entity_)
        {
            entity_->rPos.y = newValue;
            emit propertyChanged();
        }
    });
    connect(ui->entitySizeWidth_spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int newValue){
        if (entity_)
        {
            entity_->rSize.width = newValue;
            emit propertyChanged();
        }
    });
    connect(ui->entitySizeHeight_spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int newValue){
        if (entity_)
        {
            entity_->rSize.height = newValue;
            emit propertyChanged();
        }
    });
}

RenderEntityPropertiesTab::~RenderEntityPropertiesTab()
{
    delete ui;
}

void
RenderEntityPropertiesTab::setEntity(
        gpo::RenderEngine::RenderedEntity *entity)
{
    ui->frictionCircle_GroupBox->hide();
    ui->lapTimer_GroupBox->hide();
    ui->speedometer_GroupBox->hide();
    ui->telemetryPrintout_GroupBox->hide();
    ui->textObject_GroupBox->hide();
    ui->trackMap_GroupBox->hide();
    ui->video_GroupBox->hide();

    // null it first so we don't emit propertyChanged() signals when we update the fields programatically
    entity_ = nullptr;
    // ------------------------------------------------------------------
    // BEGIN - region where propertyChanged() signals won't fire
    // ------------------------------------------------------------------
    if (entity)
    {
        ui->entityName_LineEdit->setText(entity->name().c_str());
        ui->entityPositionX_spin->setValue(entity->rPos.x);
        ui->entityPositionY_spin->setValue(entity->rPos.y);
        ui->entitySizeWidth_spin->setValue(entity->rSize.width);
        ui->entitySizeHeight_spin->setValue(entity->rSize.height);

        std::string objectTypeName = entity->rObj->typeName();
        if (objectTypeName == "FrictionCircleObject")
        {
            ui->frictionCircle_GroupBox->show();
        }
        else if (objectTypeName == "LapTimerObject")
        {
            ui->lapTimer_GroupBox->show();
        }
        else if (objectTypeName == "SpeedometerObject")
        {
            ui->speedometer_GroupBox->show();
        }
        else if (objectTypeName == "TelemetryPrintoutObject")
        {
            ui->telemetryPrintout_GroupBox->show();
        }
        else if (objectTypeName == "TextObject")
        {
            ui->textObject_GroupBox->show();
        }
        else if (objectTypeName == "TrackMapObject")
        {
            ui->trackMap_GroupBox->show();
        }
        else if (objectTypeName == "VideoObject")
        {
            ui->video_GroupBox->show();
        }
    }
    else
    {
        ui->entityName_LineEdit->setText("");
        ui->entityPositionX_spin->setValue(0);
        ui->entityPositionY_spin->setValue(0);
        ui->entitySizeWidth_spin->setValue(0);
        ui->entitySizeHeight_spin->setValue(0);
    }
    entity_ = entity;
    // ------------------------------------------------------------------
    // END - region where propertyChanged() signals won't fire
    // ------------------------------------------------------------------
}
