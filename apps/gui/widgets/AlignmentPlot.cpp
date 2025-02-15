#include "AlignmentPlot.h"
#include "ui_AlignmentPlot.h"

#include <QSpinBox>
#include <spdlog/spdlog.h>

#define GET_COMBOBOX_Y_COMP(COMBOBOX_PTR,INDEX) (TelemetryPlot::Y_Component)COMBOBOX_PTR->itemData(INDEX).toULongLong()

AlignmentPlot::AlignmentPlot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AlignmentPlot),
    srcA_(nullptr),
    srcB_(nullptr),
    selectedSrc_(nullptr),
    selectedSrcHeld_(false),
    mousePressPos_px_(),
    mousePressAlignIdx_(0),
    mouseMoved_(false)
{
    ui->setupUi(this);
    ui->plot->setInteraction(QCP::Interaction::iSelectPlottables, true);

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
        auto yComp = GET_COMBOBOX_Y_COMP(ui->aData_ComboBox,index);
        ui->plot->setY_Component(yComp,true);
    });
    connect(ui->bData_ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
        auto yComp = GET_COMBOBOX_Y_COMP(ui->bData_ComboBox,index);
        ui->plot->setY_Component2(yComp,true);
    });

    // plot signal handler
    connect(ui->plot, &QCustomPlot::mousePress, this,
        [this](QMouseEvent *event){
        spdlog::trace("mousePress");
        mousePressPos_px_.setX(event->x());
        mousePressPos_px_.setY(event->y());
        mousePressed_ = true;
        mouseMoved_ = false;
        if ( ! selectedSrc_)
        {
            return;
        }
        mousePressAlignIdx_ = selectedSrc_->seeker()->getAlignmentIdx();

        QCPAbstractPlottable *plottable = ui->plot->plottableAt(mousePressPos_px_, true);
        if ( ! plottable)
        {
            return;
        }

        // check if the plottable we clicked is the current selected source
        auto tSrc = ui->plot->getSource((QCPGraph*)plottable);
        if (selectedSrc_ && selectedSrc_ == tSrc)
        {
            spdlog::debug("holding '{}'", selectedSrc_->getDataSourceName());
            selectedSrcHeld_ = true;
            setDragAndZoomEnabled(false);
        }
    });
    connect(ui->plot, &QCustomPlot::mouseRelease, this, [this]{
        spdlog::trace("mouseRelease");
        selectedSrcHeld_ = false;
        mousePressed_ = false;
        setDragAndZoomEnabled(true);
        if ( ! mouseMoved_ && selectedSrc_)
        {
            spdlog::debug("releasing '{}'", selectedSrc_->getDataSourceName());
            selectedSrc_ = nullptr;
        }
    });
    connect(ui->plot, &QCustomPlot::mouseMove, this,
        [this](QMouseEvent *event){
        spdlog::trace("mouseMove");
        mouseMoved_ = true;

        if (selectedSrcHeld_)
        {
            int deltaX_px = event->x() - mousePressPos_px_.x();
            int deltaX_coord =
                ui->plot->xAxis->pixelToCoord(event->x()) -
                ui->plot->xAxis->pixelToCoord(mousePressPos_px_.x());
            spdlog::debug("moving '{}'; deltaX_px = {}; deltaX_coord = {}",
                selectedSrc_->getDataSourceName(),
                deltaX_px,
                deltaX_coord);

            int64_t newIdx = mousePressAlignIdx_ - deltaX_coord;
            newIdx = std::min((int64_t)(selectedSrc_->size()) - 1,newIdx);
            newIdx = std::max((int64_t)0,newIdx);
            selectedSrc_->seeker()->setAlignmentIdx(newIdx);
            // queue the replot to avoid redundant replots.
            // will get replotted on the next event loop iteration.
            ui->plot->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
        }
    });
    connect(ui->plot, &QCustomPlot::plottableClick, this,
        [this](QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event){
        spdlog::trace("plottableClick");
        spdlog::debug("plottable = {}; dataIndex = {}",
            (void*)plottable,
            dataIndex);
        
        auto tSrc = ui->plot->getSource((QCPGraph*)plottable);
        if (tSrc == nullptr)
        {
            return;// no graph object was selected
        }
        else if (tSrc == srcA_ || tSrc == srcB_)
        {
            selectedSrc_ = tSrc;
        }
        else
        {
            spdlog::warn("unknown TelemetrySource's graph was selected");
        }
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
    if (srcA_)
    {
        // remove existing source from Telemetry plot.
        // only initiate a replot if source is null because in that case
        // we won't be calling addSource() below, but we still want the UI
        // to update after the removal.
        bool replotNow = tSrc == nullptr;
        ui->plot->removeSource(srcA_,replotNow);
    }
    srcA_ = tSrc;

    auto combobox = ui->aData_ComboBox;
    auto spinbox = ui->aAlignment_SpinBox;
    populateComboBox(combobox,tSrc);

    if (srcA_ != nullptr)
    {
        std::string name = srcA_->getDataSourceName();
        if (name.empty())
        {
            name = "Source A";
        }
        ui->aData_Label->setText(name.c_str());
        ui->plot->addSource(
            srcA_,
            name,
            Qt::red,
            TelemetryPlot::AxisSide::eAS_Side1,
            true);// replot

        for (int gg=0; gg<ui->plot->graphCount(); gg++)
        {
            auto graph = ui->plot->graph(gg);
            graph->setSelectable(QCP::SelectionType::stWhole);
        }

        spinbox->setMaximum(srcA_->size());
        spinbox->setMinimum(0);
        setBestY_Components();
    }
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
    srcB_ = tSrc;

    auto combobox = ui->bData_ComboBox;
    auto spinbox = ui->bAlignment_SpinBox;
    populateComboBox(combobox,tSrc);

    if (srcB_ != nullptr)
    {
        std::string name = srcB_->getDataSourceName();
        if (name.empty())
        {
            name = "Source B";
        }
        ui->bData_Label->setText(name.c_str());
        ui->plot->addSource(
            srcB_,
            name,
            Qt::blue,
            TelemetryPlot::AxisSide::eAS_Side2,
            true);// replot

        for (int gg=0; gg<ui->plot->graphCount(); gg++)
        {
            auto graph = ui->plot->graph(gg);
            graph->setSelectable(QCP::SelectionType::stWhole);
        }

        spinbox->setMaximum(srcB_->size());
        spinbox->setMinimum(0);
        setBestY_Components();
    }
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

void
AlignmentPlot::setDragAndZoomEnabled(
    bool enabled)
{
    ui->plot->setInteraction(QCP::Interaction::iRangeDrag, enabled);
    ui->plot->setInteraction(QCP::Interaction::iRangeZoom, enabled);
}

void
AlignmentPlot::setBestY_Components()
{
    if (srcA_ == nullptr || srcB_ == nullptr)
    {
        return;
    }

    const auto &availA = srcA_->dataAvailable();
    const auto &availB = srcB_->dataAvailable();

    // check for GPS_SPEED2D/ENGINE_SPEED
    if (availA.test(gpo::DataAvailable::eDA_GOPRO_GPS_SPEED2D) &&
        availB.test(gpo::DataAvailable::eDA_ECU_ENGINE_SPEED))
    {
        selectComboBoxY_Comp(ui->aData_ComboBox, TelemetryPlot::Y_Component::eYC_GPS_SPEED2D);
        selectComboBoxY_Comp(ui->bData_ComboBox, TelemetryPlot::Y_Component::eYC_ECU_ENGINE_SPEED);
    }
    else if (availA.test(gpo::DataAvailable::eDA_ECU_ENGINE_SPEED) &&
             availB.test(gpo::DataAvailable::eDA_GOPRO_GPS_SPEED2D))
    {
        selectComboBoxY_Comp(ui->aData_ComboBox, TelemetryPlot::Y_Component::eYC_ECU_ENGINE_SPEED);
        selectComboBoxY_Comp(ui->bData_ComboBox, TelemetryPlot::Y_Component::eYC_GPS_SPEED2D);
    }
}

bool
AlignmentPlot::selectComboBoxY_Comp(
    QComboBox *combobox,
    TelemetryPlot::Y_Component yComp)
{
    for (int i=0; i<combobox->count(); i++)
    {
        if (GET_COMBOBOX_Y_COMP(combobox,i) == yComp)
        {
            combobox->setCurrentIndex(i);
            return true;
        }
    }
    return false;
}
