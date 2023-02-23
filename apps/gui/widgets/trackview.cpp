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
    setMouseTracking(true);
    installEventFilter(this);

    connect(ui->fitView_ToolButton, &QToolButton::pressed, this, [this]{
        fitTrackToView();
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
    if (track_ == nullptr)
    {
        return;
    }

    QPainter painter(this);

    drawTrackPath(painter,track_);

    // draw start/finish gates
    if (pMode_ != PlacementMode::ePM_StartGate)
    {
        drawDetectionGate(painter,track_->getStart()->getEntryGate(),startGateColor_);
    }
    if (pMode_ != PlacementMode::ePM_FinishGate)
    {
        drawDetectionGate(painter,track_->getFinish()->getEntryGate(),finishGateColor_);
    }
    if (pMode_ == PlacementMode::ePM_SectorExit &&
        spi_.entryValid && mliValid_ && track_ != nullptr)
    {
        auto pathColor = sectorPathColor_;
        auto gateColor = sectorGateColor_;
        if ( ! placementValid_)
        {
            pathColor = placementWarnColor_;
        }
        gpo::TrackSector tmpSector(
            track_,
            "tmpSector",
            spi_.entryIdx,
            mli_.path_idx);
        drawSector(painter,&tmpSector,pathColor,gateColor);
    }

    // draw all track sectors
    for (size_t ss=0; ss<track_->sectorCount(); ss++)
    {
        auto sector = track_->getSector(ss);
        drawSector(painter,sector,sectorPathColor_,sectorGateColor_);
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
    if (track_ == nullptr)
    {
        return false;
    }

    if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        mli_.loc_px = mouseEvent->pos();
        mli_.loc_coord = pointToCoord(mli_.loc_px);
        auto findRes = track_->findClosestPointWithIdx(mli_.loc_coord);
        if (std::get<0>(findRes))
        {
            mli_.path_loc_px = coordToPoint(std::get<1>(findRes));
            mli_.path_idx = std::get<2>(findRes);
            mli_.gate = track_->getDetectionGate(mli_.path_idx,DEFAULT_GATE_WIDTH_M);
            mliValid_ = true;
        }

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
    }
    else if (event->type() == QEvent::MouseButtonPress)
    {
        if (placementValid_)
        {
            emit gatePlaced(pMode_,mli_.path_idx);
        }
    }
    return false;
}

void
TrackView::setToolbarVisible(
    bool visible)
{
    ui->toolbar->setVisible(visible);
}

void
TrackView::setTrack(
        gpo::Track *track)
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
        const gpo::Track *track)
{
    painter.setPen(Qt::white);

    QPoint prevPoint;
    for (size_t i=0; i<track->pathCount(); i++)
    {
        auto pathPoint = track->getPathPoint(i);
        auto currPoint = coordToPoint(pathPoint);

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
    auto pA = coordToPoint(gate.a());
    auto pB = coordToPoint(gate.b());
    painter.setPen(color);
    painter.drawLine(pA,pB);
}

void
TrackView::drawSector(
        QPainter &painter,
        const gpo::TrackSector *sector,
        QColor pathColor,
        QColor gateColor)
{
    // draw entry/exit gates
    drawDetectionGate(painter,sector->getEntryGate(),gateColor);
    drawDetectionGate(painter,sector->getExitGate(),gateColor);

    // highlight path between gates
    QPen pen;
    pen.setWidth(4);
    pen.setColor(pathColor);
    pen.setCapStyle(Qt::PenCapStyle::FlatCap);
    painter.setPen(pen);

    size_t entryIdx = sector->getEntryIdx();
    size_t exitIdx = sector->getExitIdx();
    size_t startIdx = std::min(entryIdx,exitIdx);
    size_t endIdx = std::max(entryIdx,exitIdx);
    QPoint prevPoint;
    for (size_t i=startIdx; i<=endIdx; i++)
    {
        auto pathPoint = track_->getPathPoint(i);
        auto currPoint = coordToPoint(pathPoint);

        if (i != startIdx)
        {
            painter.drawLine(prevPoint,currPoint);
        }

        prevPoint = currPoint;
    }
}

QPoint
TrackView::coordToPoint(
        const cv::Vec2d &coord)
{
    return QPoint(
                PX_MARGIN + (coord[1] - trackUL_coord_[1]) * pxPerDeg_,
                PX_MARGIN + (coord[0] - trackUL_coord_[0]) * -pxPerDeg_);
}

cv::Vec2d
TrackView::pointToCoord(
        const QPoint &qpoint)
{
    return cv::Vec2d(
                (qpoint.y() - PX_MARGIN) / -pxPerDeg_ + trackUL_coord_[0],
                (qpoint.x() - PX_MARGIN) / pxPerDeg_ + trackUL_coord_[1]);
}
