#include "trackview.h"
#include "ui_trackview.h"

#include <QMouseEvent>
#include <QPainter>
#include <spdlog/spdlog.h>

static constexpr double DEFAULT_GATE_WIDTH_M = 10.0;// 10m gate width

TrackView::TrackView(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::TrackView),
    track_(nullptr),
    mliValid_(false),
    pan_(),
    pMode_(PlacementMode::ePM_None),
    startGateColor_(Qt::green),
    finishGateColor_(Qt::red),
    sectorPathColor_(Qt::cyan),
    sectorGateColor_(Qt::cyan),
    placementWarnColor_(QRgb(0xfc655a)),
    spi_(),
    startGateFilter_(nullptr),
    finishGateFilter_(nullptr),
    sectorEntryFilter_(nullptr),
    sectorExitFilter_(nullptr),
    placementValid_(true)
{
    ui->setupUi(this);
    ui->mousePosition_Label->setVisible(ui->showCoord_ToolButton->isChecked());
    setMouseTracking(true);
    installEventFilter(this);

    connect(ui->fitView_ToolButton, &QToolButton::pressed, this, [this]{
        fitTrackToView();
    });
    connect(ui->showCoord_ToolButton, &QToolButton::clicked, this, [this]{
        ui->mousePosition_Label->setVisible(ui->showCoord_ToolButton->isChecked());
    });
}

TrackView::~TrackView()
{
    delete ui;
}

void
TrackView::paintEvent(
        QPaintEvent *event)
{
    if ( ! track_)
    {
        return;
    }

    QPainter painter(this);

    drawTrackPath(painter,track_);

    // draw start/finish gates
    if (pMode_ != PlacementMode::ePM_StartGate)
    {
        drawDetectionGate(painter,track_->getStart().getEntryGate(),startGateColor_);
    }
    if (pMode_ != PlacementMode::ePM_FinishGate)
    {
        drawDetectionGate(painter,track_->getFinish().getEntryGate(),finishGateColor_);
    }
    if (pMode_ == PlacementMode::ePM_SectorExit &&
        spi_.entryValid && mliValid_ && track_ != nullptr)
    {
        const auto pathColor = placementValid_ ? sectorPathColor_ : placementWarnColor_;
        gpo::TrackSector tmpSector(
            track_.get(),
            "tmpSector",
            spi_.entryIdx,
            mli_.path_idx);
        drawSector(painter,tmpSector,pathColor,sectorGateColor_);
    }

    // draw all track sectors
    for (size_t ss=0; ss<track_->sectorCount(); ss++)
    {
        auto sector = track_->getSector(ss);
        drawSector(painter,*sector,sectorPathColor_,sectorGateColor_);
    }

    if (mliValid_ && pMode_ != PlacementMode::ePM_None)
    {
        QColor c = Qt::white;
        switch (pMode_)
        {
            case PlacementMode::ePM_StartGate:
                c = startGateColor_;
                break;
            case PlacementMode::ePM_FinishGate:
                c = finishGateColor_;
                break;
            default:
                c = Qt::white;
                break;
        }

        drawDetectionGate(painter,mli_.gate,c);
    }
}

bool
TrackView::eventFilter(
        QObject *obj,
        QEvent *event)
{
    bool takeEvent = false;
    if (track_ == nullptr)
    {
        return takeEvent;
    }

    if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        mli_.loc_px = mouseEvent->pos();
        mli_.loc_coord = pxToCoord(mli_.loc_px);

        // handle panning logic
        if (pan_.active)
        {
            QPoint delta_px = mouseEvent->pos() - pan_.mouseDownLoc_px;
            cv::Vec2d delta_coord = pxToCoordRel(delta_px);
            spdlog::debug("delta_px: [{},{}] ({},{})",
                delta_px.x(),delta_px.y(),
                delta_coord[0],delta_coord[1]);
            viewUL_coord_ = pan_.viewUL_Began_coord - delta_coord;
        }

        // find mouse location on the track
        auto findRes = track_->findClosestPointWithIdx(mli_.loc_coord);
        if (std::get<0>(findRes))
        {
            mli_.path_loc_px = coordToPx(std::get<1>(findRes));
            mli_.path_idx = std::get<2>(findRes);
            mli_.gate = track_->getDetectionGate(mli_.path_idx,DEFAULT_GATE_WIDTH_M);
            mliValid_ = true;
        }

        // update textual display of mouse position/coordinate
        char mousePosStr[1024];
        sprintf(mousePosStr,"[%d,%d] (%0.6f,%0.6f)",
            mli_.loc_px.x(),mli_.loc_px.y(),
            mli_.loc_coord[0],mli_.loc_coord[1]);
        ui->mousePosition_Label->setText(mousePosStr);

        // apply user filters to determine if gate is valid
        placementValid_ = true;
        switch (pMode_)
        {
            case PlacementMode::ePM_None:
                placementValid_ = false;
                break;
            case PlacementMode::ePM_StartGate:
                if (startGateFilter_)
                {
                    placementValid_ = startGateFilter_(mli_.path_idx);
                }
                break;
            case PlacementMode::ePM_FinishGate:
                if (finishGateFilter_)
                {
                    placementValid_ = finishGateFilter_(mli_.path_idx);
                }
                break;
            case PlacementMode::ePM_SectorEntry:
                if (sectorEntryFilter_)
                {
                    placementValid_ = sectorEntryFilter_(mli_.path_idx);
                }
                break;
            case PlacementMode::ePM_SectorExit:
                if (sectorExitFilter_)
                {
                    placementValid_ = sectorExitFilter_(mli_.path_idx);
                }
                break;
        }

        update();// request repaint
        takeEvent = true;
    }
    else if (event->type() == QEvent::MouseButtonPress)
    {
        if (pannable())
        {
            spdlog::debug("panning began");
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            pan_.mouseDownLoc_px = mouseEvent->pos();
            pan_.mouseDownLoc_coord = pxToCoord(pan_.mouseDownLoc_px);
            pan_.viewUL_Began_coord = viewUL_coord_;
            pan_.active = true;
        }
        else if (placementValid_)
        {
            emit gatePlaced(pMode_,mli_.path_idx);
        }
        takeEvent = true;
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        if (pan_.active)
        {
            spdlog::debug("panning stopped");
            pan_.active = false;
        }
        takeEvent = true;
    }
    return takeEvent;
}

void
TrackView::setToolbarVisible(
    bool visible)
{
    ui->toolbar->setVisible(visible);
}

bool
TrackView::getToolbarVisible() const
{
    return ui->toolbar->isVisible();
}

void
TrackView::setPanningEnabled(
    bool enabled)
{
    pan_.enabled = enabled;
}

bool
TrackView::getPanningEnabled() const
{
    return pan_.enabled;
}

void
TrackView::setTrack(
    std::shared_ptr<const gpo::Track> track)
{
    track_ = track;
    mliValid_ = false;
    ui->fitView_ToolButton->setEnabled(track_ != nullptr);

    if (track_)
    {
        trackUL_coord_[0] = -10000;
        trackLR_coord_[0] = +10000;
        trackUL_coord_[1] = +10000;
        trackLR_coord_[1] = -10000;
        for (size_t i=0; i<track->pathCount(); i++)
        {
            auto point = track->getPathPoint(i);
            if (point[0] > trackUL_coord_[0])
            {
                trackUL_coord_[0] = point[0];
            }
            if (point[1] < trackUL_coord_[1])
            {
                trackUL_coord_[1] = point[1];
            }
            if (point[0] < trackLR_coord_[0])
            {
                trackLR_coord_[0] = point[0];
            }
            if (point[1] > trackLR_coord_[1])
            {
                trackLR_coord_[1] = point[1];
            }
        }
    }

    fitTrackToView(true);// redraw
}

void
TrackView::fitTrackToView(
    bool redraw)
{
    // snap upper left to where the track is located
    viewUL_coord_[0] = trackUL_coord_[0];
    viewUL_coord_[1] = trackUL_coord_[1];

    // zoom to fit track within view
    double deltaLat = trackUL_coord_[0] - trackLR_coord_[0];
    double deltaLon = trackLR_coord_[1] - trackUL_coord_[1];
    pxPerDeg_ = 1.0;
    if (deltaLat != 0.0 && deltaLat > deltaLon)
    {
        // track is taller than it is wide
        pxPerDeg_ = (height() - PX_MARGIN * 2) / deltaLat;
    }
    else if (deltaLon != 0.0)
    {
        // track is wider than it is tall
        pxPerDeg_ = (width() - PX_MARGIN * 2) / deltaLon;
    }

    if (redraw)
    {
        update();
    }
}

void
TrackView::setStartGateColor(
        QColor c)
{
    startGateColor_ = c;
}

void
TrackView::setFinishGateColor(
        QColor c)
{
    finishGateColor_ = c;
}

void
TrackView::setPlacementMode(
        PlacementMode mode)
{
    pMode_ = mode;
    switch (pMode_)
    {
        case PlacementMode::ePM_None:
            break;
        case PlacementMode::ePM_StartGate:
            break;
        case PlacementMode::ePM_FinishGate:
            break;
        case PlacementMode::ePM_SectorEntry:
            spi_.entryValid = false;
            break;
        case PlacementMode::ePM_SectorExit:
            break;
    }
}

TrackView::PlacementMode
TrackView::getPlacementMode() const
{
    return pMode_;
}

void
TrackView::setSectorEntryIdx(
        size_t pathIdx)
{
    if (track_ == nullptr)
    {
        spdlog::warn("track is null. ignoring {}()",__func__);
        return;
    }
    spi_.entryIdx = pathIdx;
    spi_.entryGate = track_->getDetectionGate(pathIdx,DEFAULT_GATE_WIDTH_M);
    spi_.entryValid = true;
}

void
TrackView::setStartGateFilter(
    PlacementFilter filter)
{
    startGateFilter_ = filter;
}

void
TrackView::setFinishGateFilter(
    PlacementFilter filter)
{
    finishGateFilter_ = filter;
}

void
TrackView::setSectorEntryFilter(
    PlacementFilter filter)
{
    sectorEntryFilter_ = filter;
}

void
TrackView::setSectorExitFilter(
    PlacementFilter filter)
{
    sectorExitFilter_ = filter;
}

void
TrackView::drawTrackPath(
        QPainter &painter,
        std::shared_ptr<const gpo::Track> track)
{
    painter.setPen(Qt::white);

    QPoint prevPoint;
    for (size_t i=0; i<track->pathCount(); i++)
    {
        auto pathPoint = track->getPathPoint(i);
        auto currPoint = coordToPx(pathPoint);

        if (i != 0)
        {
            painter.drawLine(prevPoint,currPoint);
        }

        prevPoint = currPoint;
    }
}

void
TrackView::drawDetectionGate(
        QPainter &painter,
        const gpo::DetectionGate &gate,
        QColor color)
{
    auto pA = coordToPx(gate.a());
    auto pB = coordToPx(gate.b());
    painter.setPen(color);
    painter.drawLine(pA,pB);
}

void
TrackView::drawSector(
        QPainter &painter,
        const gpo::TrackSector &sector,
        QColor pathColor,
        QColor gateColor)
{
    // draw entry/exit gates
    drawDetectionGate(painter,sector.getEntryGate(),gateColor);
    drawDetectionGate(painter,sector.getExitGate(),gateColor);

    // highlight path between gates
    QPen pen;
    pen.setWidth(4);
    pen.setColor(pathColor);
    pen.setCapStyle(Qt::PenCapStyle::FlatCap);
    painter.setPen(pen);

    size_t entryIdx = sector.getEntryIdx();
    size_t exitIdx = sector.getExitIdx();
    size_t startIdx = std::min(entryIdx,exitIdx);
    size_t endIdx = std::max(entryIdx,exitIdx);
    QPoint prevPoint;
    for (size_t i=startIdx; i<=endIdx; i++)
    {
        auto pathPoint = track_->getPathPoint(i);
        auto currPoint = coordToPx(pathPoint);

        if (i != startIdx)
        {
            painter.drawLine(prevPoint,currPoint);
        }

        prevPoint = currPoint;
    }
}

QPoint
TrackView::coordToPx(
        const cv::Vec2d &coord)
{
    return QPoint(
                PX_MARGIN + (coord[1] - viewUL_coord_[1]) * pxPerDeg_,
                PX_MARGIN + (coord[0] - viewUL_coord_[0]) * -pxPerDeg_);
}

QPoint
TrackView::coordToPxRel(
        const cv::Vec2d &coord)
{
    return QPoint(
                coord[1] * pxPerDeg_,
                coord[0] * -pxPerDeg_);
}

cv::Vec2d
TrackView::pxToCoord(
        const QPoint &qpoint)
{
    return cv::Vec2d(
                (qpoint.y() - PX_MARGIN) / -pxPerDeg_ + viewUL_coord_[0],
                (qpoint.x() - PX_MARGIN) / pxPerDeg_ + viewUL_coord_[1]);
}

cv::Vec2d
TrackView::pxToCoordRel(
        const QPoint &qpoint)
{
    return cv::Vec2d(
                qpoint.y() / -pxPerDeg_,
                qpoint.x() / pxPerDeg_);
}

bool
TrackView::pannable() const
{
    return pan_.enabled && pMode_ == PlacementMode::ePM_None;
}
