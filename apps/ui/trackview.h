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
            size_t pathIdx);

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

    PlacementMode
    getPlacementMode() const;

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

    void
    drawSector(
            QPainter &painter,
            const gpo::TrackSector *sector,
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

    struct MouseLocationInfo
    {
        // (x,y) location of mouse on widget
        QPoint loc_px;
        // (lat,lon) location on map
        cv::Vec2d loc_coord;
        // (x,y) location to nearest track path location
        QPoint path_loc_px;
        // nearest track path index
        size_t path_idx;
        // a detection gate based from 'path_idx'
        gpo::DetectionGate gate;
    };
    MouseLocationInfo mli_;
    bool mliValid_;// true if 'mli_' fields are valid

    PlacementMode pMode_;

    QColor startGateColor_;
    QColor finishGateColor_;

};

#endif // TRACKEDITOR_H
