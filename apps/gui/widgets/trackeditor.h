#pragma once

#include <QMainWindow>
#include <QStandardItemModel>

#include "trackview.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TrackEditor; }
QT_END_NAMESPACE

class TrackEditor : public QMainWindow
{
    Q_OBJECT

public:
    TrackEditor(QWidget *parent = nullptr);
    ~TrackEditor();

    bool
    loadTrackFromVideo(
        const std::string &filepath);

    bool
    loadTrackFromYAML(
        const std::string &filepath);

    bool
    saveTrackToYAML(
        const std::string &filepath);

    void
    setTrack(
            gpo::Track *track);

signals:
    void
    trackModified();

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
    bool
    filterStartGate(
        size_t pathIdx);
    
    bool
    filterFinishGate(
        size_t pathIdx);
    
    bool
    filterSectorEntryGate(
        size_t pathIdx);
    
    bool
    filterSectorExitGate(
        size_t pathIdx);
    
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
    Ui::TrackEditor *ui;
    TrackView *trackView_;

    bool iOwnTrack_;
    gpo::Track *track_;

    QStandardItemModel *sectorTableModel_;

    std::string filepathToSaveTo_;

    // when building a new sector this variable stores the entry
    // gate's index. we won't add the sector until the user places
    // the exit gate.
    size_t sectorEntryIdx_;

};
