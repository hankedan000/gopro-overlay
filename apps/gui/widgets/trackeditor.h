#pragma once

#include <QKeyEvent>
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
    loadTrackFromFile(
        const std::string &filepath);

    bool
    saveTrackToYAML(
        const std::string &filepath);

    void
    setTrack(
            gpo::Track *track);

    void
    setMenuBarVisible(
        bool visible);

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
    removeSectorPressed();

    void
    onSectorTableDataChanged(
        const QModelIndex &topLeft,
        const QModelIndex &bottomRight,
        const QVector<int> &roles);

    void
    onActionSaveTrack();

    void
    onActionSaveTrackAs();

    void
    onActionLoadTrack();

protected:
    void
    keyPressEvent(
        QKeyEvent *event) override;

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

    bool
    inPlacementMode() const;

    void
    cancelPlacement();

    void
    loadSectorsToTable();

    QList<QStandardItem *>
    makeSectorTableRow(
            const std::string &name,
            size_t entryIdx,
            size_t exitIdx) const;

    void
    appendSectorToTable(
            const std::string &name,
            size_t entryIdx,
            size_t exitIdx);

    void
    insertSectorToTable(
            size_t rowIdx,
            const std::string &name,
            size_t entryIdx,
            size_t exitIdx);

    void
    clearSectorTable();

    void
    setTrackModified();

    void
    clearTrackModified();

private:
    // sector table column indices
    static constexpr int SECTOR_TABLE_COL_IDX_NAME = 0;
    static constexpr int SECTOR_TABLE_COL_IDX_ENTRY = 1;
    static constexpr int SECTOR_TABLE_COL_IDX_EXIT = 2;

    Ui::TrackEditor *ui;

    bool iOwnTrack_;
    // true if the track has been modified without a save
    // should be set by markTrackAsModified()
    // should be cleared by clearTrackModified();
    bool trackDirty_;
    gpo::Track *track_;

    QStandardItemModel sectorTableModel_;

    std::string filepathToSaveTo_;

    // when building a new sector this variable stores the entry
    // gate's index. we won't add the sector until the user places
    // the exit gate.
    size_t sectorEntryIdx_;

    // used to ignore events when we internal edit the sector table
    bool ignoreInternalTableEdits_;

};
