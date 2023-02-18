#ifndef TELEMETRYMERGER_H
#define TELEMETRYMERGER_H

#include <QMainWindow>

#include <filesystem>
#include <QStandardItemModel>
#include <vector>

#include "GoProOverlay/data/DataSource.h"

namespace Ui {
class TelemetryMerger;
}

class TelemetryMerger : public QMainWindow
{
    Q_OBJECT

public:
    explicit TelemetryMerger(QWidget *parent = nullptr);
    ~TelemetryMerger();

    /**
     * @brief addSourceFromFile
     * @param sourcePath
     * @return
     * true if the source was added. false otherwise.
     */
    bool
    addSourceFromFile(
            const std::filesystem::path &sourcePath);

    /**
     * @brief addDataSource
     * @param dSrc
     * @return
     * true if the source was added. false otherwise.
     */
    bool
    addDataSource(
            gpo::DataSourcePtr dSrc);

    bool
    isMergePending() const;

    bool
    isMergeComplete() const;

private:
    /**
     * @brief addDataSourceInternal
     * 
     * @param dSrc
     * A DataSource that this class will own. All methods that call this
     * should make sure they are passing in either newly constructed
     * DataSourcePtr, or a duplicated pointer that way the changes made
     * in this class won't effect any original user-provided data sources.
     * 
     * @return
     * true if the source was added. false otherwise.
     */
    bool
    addDataSourceInternal(
            gpo::DataSourcePtr dSrc);

    QList<QStandardItem *>
    makeTableRow(
            gpo::DataSourcePtr dSrc) const;

    void
    moveSourceUp(
            int sourceIdx,
            bool moveIsFromSelection);

    void
    moveSourceDown(
            int sourceIdx,
            bool moveIsFromSelection);

    void
    updateMergeButtons();

    void
    startMerge();

    void
    continueMerge();

    /**
     * Call this to setup merging on the state_.currSrcIdx.
     * This method will resample the current source to match that of the merged
     * source. It will also prepare and show the TelemetryAlignPlot.
     */
    void
    setupMerge();

    void
    abortMerge();

private:
    Ui::TelemetryMerger *ui;

    QStandardItemModel tableModel_;
    std::vector<gpo::DataSourcePtr> sources_;

    struct MergeState
    {
        MergeState()
         : mergedSrc(nullptr)
         , currSrcIdx(0)
        {}

        void
        reset()
        {
            mergedSrc = nullptr;
            currSrcIdx = 0;
        }

        // the merged result
        gpo::DataSourcePtr mergedSrc;

        // index of the current source we're trying to merge
        size_t currSrcIdx;
    };

    MergeState state_;

};

#endif // TELEMETRYMERGER_H
