#include "trackview.h"
#include "ui_trackview.h"

#include <QMouseEvent>
#include <QPainter>

TrackView::TrackView(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::TrackView),
    track_(nullptr),
    mliValid_(false),
    pMode_(PlacementMode::ePM_None),
    startGateColor_(Qt::green),
    finishGateColor_(Qt::red)
{
    ui->setupUi(this);
    setMouseTracking(true);
    installEventFilter(this);
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

    // draw all track sectors
    for (size_t ss=0; ss<track_->sectorCount(); ss++)
    {
        auto sector = track_->getSector(ss);
        drawSector(painter,sector,Qt::cyan);
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
            mli_.gate = track_->getDetectionGate(mli_.path_idx,10.0);// 10m gate width
            mliValid_ = true;
        }
        update();// request repaint
    }
    else if (event->type() == QEvent::MouseButtonPress)
    {
        emit gatePlaced(pMode_,mli_.path_idx);
    }
    return false;
}

void
TrackView::setTrack(
        gpo::Track *track)
{
    track_ = track;
    mliValid_ = false;

    if (track_)
    {
        ulCoord_[0] = -10000;
        lrCoord_[0] = +10000;
        ulCoord_[1] = +10000;
        lrCoord_[1] = -10000;
        for (size_t i=0; i<track->pathCount(); i++)
        {
            auto point = track->getPathPoint(i);
            if (point[0] > ulCoord_[0])
            {
                ulCoord_[0] = point[0];
            }
            if (point[1] < ulCoord_[1])
            {
                ulCoord_[1] = point[1];
            }
            if (point[0] < lrCoord_[0])
            {
                lrCoord_[0] = point[0];
            }
            if (point[1] > lrCoord_[1])
            {
                lrCoord_[1] = point[1];
            }
        }

        double deltaLat = ulCoord_[0] - lrCoord_[0];
        double deltaLon = lrCoord_[1] - ulCoord_[1];
        pxPerDeg_ = 1.0;
        if (deltaLat > deltaLon)
        {
            // track is taller than it is wide
            pxPerDeg_ = (height() - PX_MARGIN * 2) / deltaLat;
        }
        else
        {
            // track is wider than it is tall
            pxPerDeg_ = (width() - PX_MARGIN * 2) / deltaLon;
        }
    }

    update();// request redraw
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
}

TrackView::PlacementMode
TrackView::getPlacementMode() const
{
    return pMode_;
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
        QColor color)
{
    // draw entry/exit gates
    drawDetectionGate(painter,sector->getEntryGate(),color);
    drawDetectionGate(painter,sector->getExitGate(),color);

    // highlight path between gates
    QPen pen;
    pen.setWidth(4);
    pen.setColor(color);
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
                PX_MARGIN + (coord[1] - ulCoord_[1]) * pxPerDeg_,
                PX_MARGIN + (coord[0] - ulCoord_[0]) * -pxPerDeg_);
}

cv::Vec2d
TrackView::pointToCoord(
        const QPoint &qpoint)
{
    return cv::Vec2d(
                (qpoint.y() - PX_MARGIN) / -pxPerDeg_ + ulCoord_[0],
                (qpoint.x() - PX_MARGIN) / pxPerDeg_ + ulCoord_[1]);
}
