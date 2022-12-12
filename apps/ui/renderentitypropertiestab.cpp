#include "renderentitypropertiestab.h"
#include "ui_renderentitypropertiestab.h"

#include "GoProOverlay/graphics/FrictionCircleObject.h"

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

    // handlers for FrictionCircle properties
    connect(ui->frictionCircleBorder_ColorPicker, &ColorPicker::pickedCV, this, [this](cv::Scalar newColor){
        if (entity_)
        {
            auto frictionCircle = reinterpret_cast<gpo::FrictionCircleObject*>(entity_->rObj);
            frictionCircle->setBorderColor(newColor);
            emit propertyChanged();
        }
    });
    connect(ui->taillength_spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int newValue){
        if (entity_)
        {
            auto frictionCircle = reinterpret_cast<gpo::FrictionCircleObject*>(entity_->rObj);
            frictionCircle->setTailLength(newValue);
            emit propertyChanged();
        }
    });
    connect(ui->frictionCircleTail_ColorPicker, &ColorPicker::pickedCV, this, [this](cv::Scalar newColor){
        if (entity_)
        {
            auto frictionCircle = reinterpret_cast<gpo::FrictionCircleObject*>(entity_->rObj);
            frictionCircle->setTailColor(newColor);
            emit propertyChanged();
        }
    });
    connect(ui->frictionCircleDot_ColorPicker, &ColorPicker::pickedCV, this, [this](cv::Scalar newColor){
        if (entity_)
        {
            auto frictionCircle = reinterpret_cast<gpo::FrictionCircleObject*>(entity_->rObj);
            frictionCircle->setCurrentDotColor(newColor);
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
    ui->entityProperties_ScrollArea->hide();

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
        // RenderEntity UI elements
        ui->entityName_LineEdit->setText(entity->name.c_str());
        ui->entityPositionX_spin->setValue(entity->rPos.x);
        ui->entityPositionY_spin->setValue(entity->rPos.y);
        ui->entitySizeWidth_spin->setValue(entity->rSize.width);
        ui->entitySizeHeight_spin->setValue(entity->rSize.height);

        ui->entityProperties_ScrollArea->show();

        // --------------------------------

        // RenderObject UI elements
        std::string objectTypeName = entity->rObj->typeName();
        if (objectTypeName == "FrictionCircleObject")
        {
            auto frictionCircle = reinterpret_cast<gpo::FrictionCircleObject*>(entity->rObj);
            ui->frictionCircleBorder_ColorPicker->setColorCV(frictionCircle->getBorderColor());
            ui->taillength_spin->setValue(frictionCircle->getTailLength());
            ui->frictionCircleTail_ColorPicker->setColorCV(frictionCircle->getTailColor());
            ui->frictionCircleDot_ColorPicker->setColorCV(frictionCircle->getCurrentDotColor());

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
    entity_ = entity;
    // ------------------------------------------------------------------
    // END - region where propertyChanged() signals won't fire
    // ------------------------------------------------------------------
}
