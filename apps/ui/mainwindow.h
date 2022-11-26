#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>

#include "trackview.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool
    loadTrackFromVideo(
        const std::string &filepath);

    bool
    loadTrackFromYAML(
        const std::string &filepath);

    bool
    saveTrackToYAML(
        const std::string &filepath);

private slots:
    void
    setStartToggled(
        bool checked);

    void
    setFinishToggled(
        bool checked);

    void
    trackViewGatePlaced(
        TrackView::PlacementMode pMode,
            size_t pathIdx);

    void
    addSectorPressed();

    void
    onActionSaveTrack();

    void
    onActionSaveTrackAs();

    void
    onActionLoadTrack();

private:
    void
    releaseTrack();

    void
    configureFileMenuButtons();

private:
    Ui::MainWindow *ui;
    TrackView *trackView_;
    gpo::Track *track_;

    QStandardItemModel *sectorTableModel_;

    std::string filepathToSaveTo_;

};
#endif // MAINWINDOW_H
