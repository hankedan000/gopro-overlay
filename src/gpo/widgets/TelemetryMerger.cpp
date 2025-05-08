#include "TelemetryMerger.h"
#include "ui_TelemetryMerger.h"

#include <algorithm>
#include <QFileDialog>
#include <spdlog/spdlog.h>
#include <string>

TelemetryMerger::TelemetryMerger(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TelemetryMerger),
    alignPlotWindow_(this),
    alignPlot_(&alignPlotWindow_),
    tableModel_(this),
    sources_(),
    preferredSaveDir_(),
    state_()
{
    ui->setupUi(this);
    ui->sources_TableView->setModel(&tableModel_);
    alignPlotWindow_.setCentralWidget(&alignPlot_);
    updateMergeButtons();

    tableModel_.setHorizontalHeaderLabels({
        "Source Name","Data Rate (Hz)"
    });

    connect(ui->sources_TableView, &QTableView::activated, this, [this](const QModelIndex & /* index */){
        updateMergeButtons();
    });
    connect(ui->itemDown_ToolButton, &QToolButton::pressed, this, [this]{
        auto selectionModel = ui->sources_TableView->selectionModel();
        auto selectedIndexs = selectionModel->selectedIndexes();
        if (selectedIndexs.empty())
        {
            return;
        }

        int row = selectedIndexs.at(0).row();
        moveSourceDown(row,true);
    });
    connect(ui->itemUp_ToolButton, &QToolButton::pressed, this, [this]{
        auto selectionModel = ui->sources_TableView->selectionModel();
        auto selectedIndexs = selectionModel->selectedIndexes();
        if (selectedIndexs.empty())
        {
            return;
        }

        int row = selectedIndexs.at(0).row();
        moveSourceUp(row,true);
    });
    connect(ui->start_Button, &QPushButton::pressed, this, [this]{
        startMerge();
    });
    connect(ui->continue_Button, &QPushButton::pressed, this, [this]{
        continueMerge();
    });
    connect(ui->abort_Button, &QPushButton::pressed, this, [this]{
        abortMerge();
    });
}

TelemetryMerger::~TelemetryMerger()
{
    delete ui;
}

bool
TelemetryMerger::addSourceFromFile(
        const std::filesystem::path &sourcePath)
{
    return addDataSourceInternal(gpo::DataSource::loadDataFromFile(sourcePath));
}

bool
TelemetryMerger::addDataSource(
        gpo::DataSourcePtr dSrc)
{
    if (dSrc == nullptr)
    {
        spdlog::warn(
            "{} - dSrc is null. ignoring it.",
            __func__);
        return false;
    }

    return addDataSourceInternal(dSrc->duplicate());
}

bool
TelemetryMerger::isMergePending() const
{
    return state_.mergedSrc != nullptr && state_.currSrcIdx < sources_.size();
}

bool
TelemetryMerger::isMergeComplete() const
{
    return state_.mergedSrc != nullptr && state_.currSrcIdx >= sources_.size();
}

void
TelemetryMerger::setPreferredSaveDir(
    std::filesystem::path dir)
{
    if ( ! std::filesystem::is_directory(dir))
    {
        spdlog::error("'{}' isn't a directory", dir.c_str());
        return;
    }

    preferredSaveDir_ = dir;
}

bool
TelemetryMerger::addDataSourceInternal(
        gpo::DataSourcePtr dSrc)
{
    if (dSrc == nullptr)
    {
        spdlog::warn(
            "{} - dSrc is null. ignoring it.",
            __func__);
        return false;
    }

    // make sure we don't already have that source added
    for (const auto &existingSrc : sources_)
    {
        if (existingSrc->getOrigin() == dSrc->getOrigin())
        {
            spdlog::warn(
                "{} - source from path '{}' is already added. ignoring it.",
                __func__,
                dSrc->getOrigin());
            return false;
        }
    }

    // just in case it had a back saved already
    dSrc->deleteTelemetryBackup();

    // append the new source to the bottom of the list
    sources_.push_back(dSrc);
    tableModel_.appendRow(makeTableRow(dSrc));

    updateMergeButtons();
    return true;
}

QList<QStandardItem *>
TelemetryMerger::makeTableRow(
        gpo::DataSourcePtr dSrc) const
{
    QList<QStandardItem *> row;

    QStandardItem *nameItem = new QStandardItem;
    nameItem->setText(dSrc->getSourceName().c_str());
    nameItem->setToolTip(dSrc->getOrigin().c_str());
    nameItem->setEditable(false);
    row.append(nameItem);

    QStandardItem *rateItem = new QStandardItem;
    rateItem->setText(QString().asprintf("%0.3f",dSrc->getTelemetryRate_hz()));
    rateItem->setEditable(false);
    row.append(rateItem);

    return row;
}

void
TelemetryMerger::moveSourceUp(
        int sourceRow,
        bool moveIsFromSelection)
{
    int destRow = sourceRow - 1;
    if (sourceRow < 0 || (size_t)(sourceRow) >= sources_.size())
    {
        spdlog::warn("{} - sourceRow {} references invalid row",
            __func__,
            sourceRow);
        return;
    }
    else if (destRow < 0)
    {
        spdlog::warn("{} - sourceRow {} is already at the top of the list",
            __func__,
            sourceRow);
        return;
    }

    spdlog::debug("{} - moving '{}' from row {} to row {}",
        __func__,
        sources_.at(sourceRow)->getSourceName(),
        sourceRow,
        destRow);

    // move source by swapping with the other
    auto tmp = sources_.at(destRow);
    sources_.at(destRow) = sources_.at(sourceRow);
    sources_.at(sourceRow) = tmp;

    auto takenRows = tableModel_.takeRow(sourceRow);
    tableModel_.insertRow(destRow,takenRows);
    if (moveIsFromSelection)
    {
        ui->sources_TableView->selectRow(destRow);
        updateMergeButtons();
    }
}

void
TelemetryMerger::moveSourceDown(
        int sourceRow,
        bool moveIsFromSelection)
{
    int destRow = sourceRow + 1;
    if (sourceRow < 0 || (size_t)(sourceRow) >= sources_.size())
    {
        spdlog::warn("{} - sourceRow {} references invalid row",
            __func__,
            sourceRow);
        return;
    }
    else if ((size_t)(destRow) >= sources_.size())
    {
        spdlog::warn("{} - sourceRow {} is already at the bottom of the list",
            __func__,
            sourceRow);
        return;
    }

    spdlog::debug("{} - moving '{}' from row {} to row {}",
        __func__,
        sources_.at(sourceRow)->getSourceName(),
        sourceRow,
        destRow);

    // move source by swapping with the other
    auto tmp = sources_.at(destRow);
    sources_.at(destRow) = sources_.at(sourceRow);
    sources_.at(sourceRow) = tmp;

    auto takenRows = tableModel_.takeRow(sourceRow);
    tableModel_.insertRow(destRow,takenRows);
    if (moveIsFromSelection)
    {
        ui->sources_TableView->selectRow(destRow);
        updateMergeButtons();
    }
}

void
TelemetryMerger::updateMergeButtons()
{
    const bool mergePending = isMergePending();

    // update the enable state of the up/down buttons
    auto selectionModel = ui->sources_TableView->selectionModel();
    auto selectedIndexs = selectionModel->selectedIndexes();
    if (mergePending || selectedIndexs.empty())
    {
        ui->itemUp_ToolButton->setEnabled(false);
        ui->itemDown_ToolButton->setEnabled(false);
    }
    else
    {
        int row = selectedIndexs.at(0).row();
        ui->itemUp_ToolButton->setEnabled(row > 0);
        ui->itemDown_ToolButton->setEnabled(row < (tableModel_.rowCount() - 1));
    }

    // update start/stop/continue merge buttons
    ui->start_Button->setVisible( ! mergePending);
    ui->start_Button->setEnabled(sources_.size() > 0);
    ui->continue_Button->setVisible(mergePending);
    ui->abort_Button->setVisible(mergePending);
}

void
TelemetryMerger::startMerge()
{
    state_.reset();
    if (sources_.empty())
    {
        spdlog::warn("no sources to merge");
        return;
    }

    state_.mergedSrc = sources_.at(0)->duplicate();
    state_.currSrcIdx++;

    if (state_.currSrcIdx < sources_.size())
    {
        setupMerge();
    }
    else
    {
        saveMergedResult();
    }
    updateMergeButtons();
}

void
TelemetryMerger::continueMerge()
{
    alignPlotWindow_.hide();

    auto mergedSrc = state_.mergedSrc;
    auto currSrc = sources_.at(state_.currSrcIdx);

    // compute source/destination merge starting indices
    size_t srcStartIdx = 0;
    size_t dstStartIdx = 0;
    const size_t mergedAlignIdx = mergedSrc->seeker->getAlignmentIdx();
    const size_t currSrcAlignIdx = currSrc->seeker->getAlignmentIdx();
    if (mergedAlignIdx < currSrcAlignIdx)
    {
        srcStartIdx = currSrcAlignIdx - mergedAlignIdx;
    }
    else
    {
        dstStartIdx = mergedAlignIdx - currSrcAlignIdx;
    }

    mergedSrc->mergeTelemetryIn(
        currSrc,
        srcStartIdx,
        dstStartIdx,
        false);// don't grow
    state_.currSrcIdx++;

    if (state_.currSrcIdx < sources_.size())
    {
        setupMerge();
    }
    else
    {
        saveMergedResult();
    }
    updateMergeButtons();
}

void
TelemetryMerger::setupMerge()
{
    // highlight the row we're merging next
    ui->sources_TableView->selectRow(state_.currSrcIdx);

    // sample rates need to match, so resample current source to match
    auto currSrc = sources_.at(state_.currSrcIdx);
    currSrc->backupTelemetry();
    currSrc->resampleTelemetry(state_.mergedSrc->getTelemetryRate_hz());

    alignPlot_.setSourceA(state_.mergedSrc->telemSrc);
    alignPlot_.setSourceB(currSrc->telemSrc);
    alignPlotWindow_.show();
}

bool
TelemetryMerger::saveMergedResult()
{
    if ( ! state_.mergedSrc)
    {
        spdlog::error("mergedSrc is null");
        return false;
    }
    
    // default dialog's directory to user's home dir
    QString suggestedDir = QDir::homePath();
    if ( ! preferredSaveDir_.empty())
    {
        suggestedDir = preferredSaveDir_.c_str();
    }

    // open "Save" dialog
    std::string filepath = QFileDialog::getSaveFileName(
        this,
        "Save Merged Telemetry CSV",
        suggestedDir,
        "CSV (*.csv)").toStdString();
    if (filepath.empty())
    {
        // dialog was closed without selecting a path
        return false;
    }

    return state_.mergedSrc->writeTelemetryToCSV(filepath);
}

void
TelemetryMerger::abortMerge()
{
    state_.reset();
    alignPlotWindow_.hide();
    for (auto &dSrc : sources_)
    {
        if (dSrc->hasBackup())
        {
            dSrc->restoreTelemetry();
            dSrc->deleteTelemetryBackup();
        }
    }
    updateMergeButtons();
}
