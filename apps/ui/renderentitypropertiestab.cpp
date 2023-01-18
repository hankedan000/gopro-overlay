#include "renderentitypropertiestab.h"
#include "ui_renderentitypropertiestab.h"

#include "GoProOverlay/graphics/FrictionCircleObject.h"
#include "GoProOverlay/graphics/TelemetryPlotObject.h"

RenderEntityPropertiesTab::RenderEntityPropertiesTab(
        QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::RenderEntityPropertiesTab),
    project_(nullptr),
    entity_(nullptr)
{
    ui->setupUi(this);
    setEntity(entity_);

    // =============================================
    // RenderedObject
    // =============================================
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
            auto newSize = entity_->rSize;
            if (ui->keepRatio_CheckBox->isChecked())
            {
                auto nativeSize = entity_->rObj->getNativeSize();
                double ratio = (double)(newValue)/nativeSize.width;
                newSize.width = nativeSize.width * ratio;
                newSize.height = nativeSize.height * ratio;
            }
            else
            {
                newSize.width = newValue;
            }
            entity_->rSize = newSize;
            emit propertyChanged();
        }
    });
    connect(ui->entitySizeHeight_spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int newValue){
        if (entity_)
        {
            auto newSize = entity_->rSize;
            if (ui->keepRatio_CheckBox->isChecked())
            {
                auto nativeSize = entity_->rObj->getNativeSize();
                double ratio = (double)(newValue)/nativeSize.height;
                newSize.width = nativeSize.width * ratio;
                newSize.height = nativeSize.height * ratio;
            }
            else
            {
                newSize.height = newValue;
            }
            entity_->rSize = newSize;
            emit propertyChanged();
        }
    });

    // =============================================
    // FrictionCircleObject
    // =============================================
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

    // =============================================
    // TelemetryPlotObject
    // =============================================
    // init y-component combobox
    for (const auto &info : TelemetryPlot::Y_COMP_ENUM_INFO)
    {
        ui->yComponent_ComboBox->addItem(info.name,(qint32)info.yComp);
    }
    connect(ui->yComponent_ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
        int yCompInt = ui->yComponent_ComboBox->itemData(index).toInt();
        auto yComp = (TelemetryPlot::Y_Component)yCompInt;
        if (entity_)
        {
            auto telemPlot = reinterpret_cast<gpo::TelemetryPlotObject*>(entity_->rObj);
            telemPlot->setY_Component(yComp);
            emit propertyChanged();
        }
    });
}

RenderEntityPropertiesTab::~RenderEntityPropertiesTab()
{
    delete ui;
}

void
RenderEntityPropertiesTab::setProject(
        gpo::RenderProject *project)
{
    project_ = project;
}

void
RenderEntityPropertiesTab::setEntity(
        gpo::RenderEngine::RenderedEntity *entity)
{
    ui->entityProperties_ScrollArea->hide();

    ui->frictionCircle_GroupBox->hide();
    ui->lapTimer_GroupBox->hide();
    ui->speedometer_GroupBox->hide();
    ui->telemetryPlot_GroupBox->hide();
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

        // common RenderedObject UI elements
        QStringList videoSourceNames;
        QStringList telemetrySourceNames;
        for (size_t ss=0; project_ != nullptr && ss<project_->dataSourceManager().sourceCount(); ss++)
        {
            auto dataSrc = project_->dataSourceManager().getSource(ss);
            const auto sourceName = dataSrc->getSourceName();
            if (dataSrc->hasVideo())
            {
                videoSourceNames.append(sourceName.c_str());
            }
            if (dataSrc->hasTelemetry())
            {
                telemetrySourceNames.append(sourceName.c_str());
            }
        }

        auto sourceReqs = entity->rObj->dataSourceRequirements();
        size_t videoSlotsToPopulate = std::max((size_t)sourceReqs.minVideos(),entity->rObj->numVideoSources());
        size_t telemSlotsToPopulate = std::max((size_t)sourceReqs.minTelemetry(),entity->rObj->numTelemetrySources());

        auto videoTable = ui->videoSources_TableView;
        videoTable->setRowCount(videoSlotsToPopulate);
        videoTable->setColumnCount(1);
        videoTable->horizontalHeader()->hide();
        videoTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        for (size_t vv=0; vv<videoSlotsToPopulate; vv++)
        {
            std::string selectedSourceName = "";// empty means not set yet
            if (vv < entity->rObj->numVideoSources())
            {
                selectedSourceName = entity->rObj->getVideoSource(vv)->getDataSourceName();
            }

            QComboBox *videoComboBox = new QComboBox(videoTable);
            videoComboBox->addItems(videoSourceNames);
            videoComboBox->setCurrentText(selectedSourceName.c_str());
            videoTable->setCellWidget(vv,0,videoComboBox);
        }

        auto telemetryTable = ui->telemetrySources_TableView;
        telemetryTable->setRowCount(telemSlotsToPopulate);
        telemetryTable->setColumnCount(1);
        telemetryTable->horizontalHeader()->hide();
        telemetryTable->horizontalHeader()->stretchLastSection();
        telemetryTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        for (size_t tt=0; tt<telemSlotsToPopulate; tt++)
        {
            std::string selectedSourceName = "";// empty means not set yet
            if (tt < entity->rObj->numTelemetrySources())
            {
                selectedSourceName = entity->rObj->getTelemetrySource(tt)->getDataSourceName();
            }

            QComboBox *telemetryComboBox = new QComboBox(telemetryTable);
            telemetryComboBox->addItems(telemetrySourceNames);
            telemetryComboBox->setCurrentText(selectedSourceName.c_str());
            telemetryTable->setCellWidget(tt,0,telemetryComboBox);
        }

        // --------------------------------

        // RenderedObject subclass UI elements
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
        else if (objectTypeName == "TelemetryPlotObject")
        {
            auto telemPlot = reinterpret_cast<gpo::TelemetryPlotObject*>(entity->rObj);
            auto plotsY_Comp = telemPlot->getY_Component();
            for (size_t i=0; i<ui->yComponent_ComboBox->count(); i++)
            {
                int yCompInt = ui->yComponent_ComboBox->itemData(i).toInt();
                auto yComp = (TelemetryPlot::Y_Component)yCompInt;
                if (yComp == plotsY_Comp)
                {
                    ui->yComponent_ComboBox->setCurrentIndex(i);
                    break;
                }
            }

            ui->telemetryPlot_GroupBox->show();
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
