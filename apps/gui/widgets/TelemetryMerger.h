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

private:
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
    updateMoveButtonEnables();

private:
    Ui::TelemetryMerger *ui;

    QStandardItemModel tableModel_;
    std::vector<gpo::DataSourcePtr> sources_;

};

#endif // TELEMETRYMERGER_H
