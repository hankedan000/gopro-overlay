#include "AlignmentPlot.h"
#include "ui_AlignmentPlot.h"

AlignmentPlot::AlignmentPlot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AlignmentPlot)
{
    ui->setupUi(this);
    ui->plot->setY_Component(TelemetryPlot::Y_Component::eYC_GPS_Speed2D);
    ui->plot->setY_Component2(TelemetryPlot::Y_Component::eYC_Veh_EngineSpeed);
}

AlignmentPlot::~AlignmentPlot()
{
    delete ui;
}

void
AlignmentPlot::setSourceA(
    gpo::TelemetrySourcePtr tSrc)
{
    if (srcA_)
    {
        // remove existing source from Telemetry plot.
        // only initiate a replot if source is null because in that case
        // we won't be calling addSource() below, but we still want the UI
        // to update after the removal.
        bool replotNow = tSrc == nullptr;
        ui->plot->removeSource(srcA_,replotNow);
    }

    if (tSrc != nullptr)
    {
        ui->plot->addSource(
            tSrc,
            "Data A",
            Qt::red,
            TelemetryPlot::AxisSide::eAS_Side1,
            true);// replot
    }
    srcA_ = tSrc;
}

void
AlignmentPlot::setSourceB(
    gpo::TelemetrySourcePtr tSrc)
{
    if (srcB_)
    {
        // remove existing source from Telemetry plot.
        // only initiate a replot if source is null because in that case
        // we won't be calling addSource() below, but we still want the UI
        // to update after the removal.
        bool replotNow = tSrc == nullptr;
        ui->plot->removeSource(srcB_,replotNow);
    }

    if (tSrc != nullptr)
    {
        ui->plot->addSource(
            tSrc,
            "Data B",
            Qt::blue,
            TelemetryPlot::AxisSide::eAS_Side2,
            true);// replot
    }
    srcB_ = tSrc;
}
