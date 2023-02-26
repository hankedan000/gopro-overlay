#pragma once

#include <QKeyEvent>
#include <QMainWindow>
#include <QStandardItemModel>

#include "GoProOverlay/data/ModifiableObject.h"
#include "trackview.h"
#include "utils/QModifiableObjectObserver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TrackEditor; }
QT_END_NAMESPACE

class TrackEditor : public QMainWindow, private gpo::ModifiableObjectObserver
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

    void
    setTrack(
            gpo::Track *track);

    void
    setMenuBarVisible(
        bool visible);

private slots:
    void
    setStartToggled(
        bool checked);

    void
    setFinishToggled(
        bool checked);

    void
    onApplyClicked();

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
    onModified(
        gpo::ModifiableObject *modifiable) override;
        
    void
    onModificationsApplied(
        gpo::ModifiableObject *modifiable) override;
        
private:
    // sector table column indices
    static constexpr int SECTOR_TABLE_COL_IDX_NAME = 0;
    static constexpr int SECTOR_TABLE_COL_IDX_ENTRY = 1;
    static constexpr int SECTOR_TABLE_COL_IDX_EXIT = 2;

    Ui::TrackEditor *ui;
    QModifiableObjectObserver trackObserver_;

    bool iOwnTrack_;
    gpo::Track *track_;

    QStandardItemModel sectorTableModel_;

    // when building a new sector this variable stores the entry
    // gate's index. we won't add the sector until the user places
    // the exit gate.
    size_t sectorEntryIdx_;

    // used to ignore events when we internal edit the sector table
    bool ignoreInternalTableEdits_;

};
