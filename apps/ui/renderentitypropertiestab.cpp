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
    entity_ = entity;

    if (entity_)
    {
        ui->entityName_LineEdit->setText(entity_->name().c_str());
        ui->entityPositionX_spin->setValue(entity_->rPos.x);
        ui->entityPositionY_spin->setValue(entity_->rPos.y);
        ui->entitySizeWidth_spin->setValue(entity_->rSize.width);
        ui->entitySizeHeight_spin->setValue(entity_->rSize.height);
    }
    else
    {
        ui->entityName_LineEdit->setText("");
        ui->entityPositionX_spin->setValue(0);
        ui->entityPositionY_spin->setValue(0);
        ui->entitySizeWidth_spin->setValue(0);
        ui->entitySizeHeight_spin->setValue(0);
    }
}
