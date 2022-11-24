#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <QFrame>

#include <GoProOverlay/data/TrackDataObjects.h>
#include <opencv2/opencv.hpp>

namespace Ui {
class TrackView;
}

class TrackView : public QFrame
{
    Q_OBJECT
public:
    enum PlacementMode
    {
        ePM_StartGate,
        ePM_FinishGate,
        ePM_SectorEntry,
        ePM_SectorExit,
        ePM_None
    };

public:
    explicit TrackView(QWidget *parent = nullptr);
    ~TrackView();

signals:
    void
    gatePlaced(
            PlacementMode pMode,
            gpo::DetectionGate gate);

public:
    void
    paintEvent(
            QPaintEvent *event);

    bool
    eventFilter(
            QObject *obj,
            QEvent *event);

    void
    setTrack(
            gpo::Track *track);

    void
    setStartGateColor(
            QColor c);

    void
    setFinishGateColor(
            QColor c);

    void
    setPlacementMode(
            PlacementMode mode);

private:
    void
    drawTrackPath(
            QPainter &painter,
            const gpo::Track *track);

    void
    drawDetectionGate(
            QPainter &painter,
            const gpo::DetectionGate &gate,
            QColor color);

    QPoint
    coordToPoint(
            const cv::Vec2d &coord);

    cv::Vec2d
    pointToCoord(
            const QPoint &qpoint);

private:
    int PX_MARGIN = 10;
    Ui::TrackView *ui;
    cv::Vec2d ulCoord_;
    cv::Vec2d lrCoord_;
    double pxPerDeg_;
    gpo::Track *track_;
    bool mouseLocationValid_;
    QPoint mouseLocation_;
    cv::Vec2d mouseCoord_;
    QPoint mouseNearestPathPoint_;
    gpo::DetectionGate mouseGate_;

    PlacementMode pMode_;

    QColor startGateColor_;
    QColor finishGateColor_;

};

#endif // TRACKEDITOR_H
