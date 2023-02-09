#include "AlignmentPlot.h"
#include "ui_AlignmentPlot.h"

#include <QSpinBox>

AlignmentPlot::AlignmentPlot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AlignmentPlot)
{
    ui->setupUi(this);
    ui->plot->setY_Component(TelemetryPlot::Y_Component::eYC_GPS_Speed2D);
    ui->plot->setY_Component2(TelemetryPlot::Y_Component::eYC_ECU_EngineSpeed);

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
        ui->plot->addSource(
            tSrc,
            "Data A",
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
        ui->plot->addSource(
            tSrc,
            "Data B",
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

    auto gpAvail = tSrc->gpDataAvail();
    auto ecuAvail = tSrc->ecuDataAvail();
    if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_ACCL))
    {
        addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_AcclX);
        addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_AcclY);
        addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_AcclZ);
    }
    if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_GYRO))
    {
        addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_GyroX);
        addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_GyroY);
        addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_GyroZ);
    }
    if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_GRAV))
    {
        // addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_GravX);
        // addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_GravY);
        // addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_GravZ);
    }
    if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_CORI))
    {
        // addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_CoriW);
        // addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_CoriX);
        // addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_CoriY);
        // addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_CoriZ);
    }
    if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_GPS_LATLON))
    {
        // addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_GPS_Lat);
        // addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_GPS_Lon);
    }
    if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_GPS_SPEED2D))
    {
        addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_GPS_Speed2D);
    }
    if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_GPS_SPEED3D))
    {
        addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_GPS_Speed3D);
    }
    if (bitset_is_set(ecuAvail, gpo::ECU_AVAIL_ENGINE_SPEED))
    {
        addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_ECU_EngineSpeed);
    }
    if (bitset_is_set(ecuAvail, gpo::ECU_AVAIL_TPS))
    {
        addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_ECU_TPS);
    }
    if (bitset_is_set(ecuAvail, gpo::ECU_AVAIL_BOOST))
    {
        addY_CompToComboBox(combobox,TelemetryPlot::Y_Component::eYC_ECU_Boost);
    }
}

void
AlignmentPlot::addY_CompToComboBox(
    QComboBox *combobox,
    TelemetryPlot::Y_Component yComp) const
{
    auto yCompInfo = TelemetryPlot::getY_ComponentInfo(yComp);
    combobox->addItem(yCompInfo.name,(qulonglong)(yComp));
}
