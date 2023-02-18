#include "AlignmentPlot.h"
#include "ui_AlignmentPlot.h"

#include <QSpinBox>

AlignmentPlot::AlignmentPlot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AlignmentPlot)
{
    ui->setupUi(this);
    ui->plot->setY_Component(TelemetryPlot::Y_Component::eYC_GPS_SPEED2D);
    ui->plot->setY_Component2(TelemetryPlot::Y_Component::eYC_ECU_ENGINE_SPEED);

    connect(ui->aAlignment_SpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value){
        auto seeker = srcA_->seeker();
        seeker->setAlignmentIdx(value);
        // queue the replot to avoid redundant replots.
        // will get replotted on the next event loop iteration.
        ui->plot->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
    });
    connect(ui->bAlignment_SpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value){
        auto seeker = srcB_->seeker();
        seeker->setAlignmentIdx(value);
        // queue the replot to avoid redundant replots.
        // will get replotted on the next event loop iteration.
        ui->plot->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
    });
    connect(ui->aData_ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
        auto yComp = (TelemetryPlot::Y_Component)ui->aData_ComboBox->itemData(index).toULongLong();
        ui->plot->setY_Component(yComp,true);
    });
    connect(ui->bData_ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
        auto yComp = (TelemetryPlot::Y_Component)ui->bData_ComboBox->itemData(index).toULongLong();
        ui->plot->setY_Component2(yComp,true);
    });
}

AlignmentPlot::~AlignmentPlot()
{
    delete ui;
}

void
AlignmentPlot::setSourceA(
    gpo::TelemetrySourcePtr tSrc)
{
    auto combobox = ui->aData_ComboBox;
    auto spinbox = ui->aAlignment_SpinBox;

    if (srcA_)
    {
        // remove existing source from Telemetry plot.
        // only initiate a replot if source is null because in that case
        // we won't be calling addSource() below, but we still want the UI
        // to update after the removal.
        bool replotNow = tSrc == nullptr;
        ui->plot->removeSource(srcA_,replotNow);
    }

    populateComboBox(combobox,tSrc);
    if (tSrc != nullptr)
    {
        std::string name = tSrc->getDataSourceName();
        if (name.empty())
        {
            name = "Source A";
        }
        ui->aData_Label->setText(name.c_str());
        ui->plot->addSource(
            tSrc,
            name,
            Qt::red,
            TelemetryPlot::AxisSide::eAS_Side1,
            true);// replot

        spinbox->setMaximum(tSrc->size());
        spinbox->setMinimum(0);
    }
    srcA_ = tSrc;
}

void
AlignmentPlot::setSourceB(
    gpo::TelemetrySourcePtr tSrc)
{
    auto combobox = ui->bData_ComboBox;
    auto spinbox = ui->bAlignment_SpinBox;

    if (srcB_)
    {
        // remove existing source from Telemetry plot.
        // only initiate a replot if source is null because in that case
        // we won't be calling addSource() below, but we still want the UI
        // to update after the removal.
        bool replotNow = tSrc == nullptr;
        ui->plot->removeSource(srcB_,replotNow);
    }

    populateComboBox(combobox,tSrc);
    if (tSrc != nullptr)
    {
        std::string name = tSrc->getDataSourceName();
        if (name.empty())
        {
            name = "Source B";
        }
        ui->bData_Label->setText(name.c_str());
        ui->plot->addSource(
            tSrc,
            name,
            Qt::blue,
            TelemetryPlot::AxisSide::eAS_Side2,
            true);// replot

        spinbox->setMaximum(tSrc->size());
        spinbox->setMinimum(0);
    }
    srcB_ = tSrc;
}

void
AlignmentPlot::populateComboBox(
    QComboBox *combobox,
    gpo::TelemetrySourcePtr tSrc) const
{
    combobox->clear();
    if (tSrc == nullptr)
    {
        return;
    }

    auto availY_Comps = TelemetryPlot::getAvailY_ComponentInfo(tSrc);
    for (const auto &yInfo : availY_Comps)
    {
        combobox->addItem(yInfo->name,(qulonglong)(yInfo->yComp));
    }
}
