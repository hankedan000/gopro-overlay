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

    void
    addNewSector(
            size_t entryIdx,
            size_t exitIdx);

    void
    loadSectorsToTable();

    void
    addSectorToTable(
            const std::string &name,
            size_t entryIdx,
            size_t exitIdx);

    void
    clearSectorTable();

private:
    Ui::MainWindow *ui;
    TrackView *trackView_;
    gpo::Track *track_;

    QStandardItemModel *sectorTableModel_;

    std::string filepathToSaveTo_;

    // when building a new sector this variable stores the entry
    // gate's index. we won't add the sector until the user places
    // the exit gate.
    size_t sectorEntryIdx_;

};
#endif // MAINWINDOW_H
