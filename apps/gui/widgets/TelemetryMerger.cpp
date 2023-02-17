#include "TelemetryMerger.h"
#include "ui_TelemetryMerger.h"

#include <algorithm>
#include <spdlog/spdlog.h>
#include <string>

TelemetryMerger::TelemetryMerger(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TelemetryMerger),
    tableModel_(this),
    sources_()
{
    ui->setupUi(this);
    ui->itemDown_ToolButton->setEnabled(false);
    ui->itemUp_ToolButton->setEnabled(false);

    tableModel_.setColumnCount(4);
    tableModel_.setHorizontalHeaderLabels({
        "Source Name","Data Rate (Hz)"
    });
    ui->sources_TableView->setModel(&tableModel_);

    connect(ui->sources_TableView, &QTableView::activated, this, [this](const QModelIndex &index){
        updateMoveButtonEnables();
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
}

TelemetryMerger::~TelemetryMerger()
{
    delete ui;
}

bool
TelemetryMerger::addSourceFromFile(
        const std::filesystem::path &sourcePath)
{
    return addDataSource(gpo::DataSource::loadDataFromFile(sourcePath));
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

    // append the new source to the bottom of the list
    sources_.push_back(dSrc);
    tableModel_.appendRow(makeTableRow(dSrc));

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
        updateMoveButtonEnables();
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
        updateMoveButtonEnables();
    }
}

void
TelemetryMerger::updateMoveButtonEnables()
{
    auto selectionModel = ui->sources_TableView->selectionModel();
    auto selectedIndexs = selectionModel->selectedIndexes();
    if (selectedIndexs.empty())
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
}
