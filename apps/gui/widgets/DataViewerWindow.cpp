#include "DataViewerWindow.h"
#include "ui_DataViewerWindow.h"

#include <QFileDialog>
#include <spdlog/spdlog.h>

#include "utils/ProjectSettings.h"

DataViewerWindow::DataViewerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DataViewerWindow),
    topCompareTableModel_(new QStandardItemModel(this)),
    sectorDetailsTableModel_(new QStandardItemModel(this))
{
    ui->setupUi(this);
    ui->topSplitter->setSizes(QList<int>({75000, 25000}));
    ui->centralSplitter->setSizes(QList<int>{25000,75000});
    ui->topCompare_TableView->setModel(topCompareTableModel_);
    ui->sectorDetails_TableView->setModel(sectorDetailsTableModel_);

    populateRecentProjects();

    connect(ui->actionLoad_Project, &QAction::triggered, this, [this]{
        std::string filepath = QFileDialog::getExistingDirectory(
                    this,
                    "Open Project",
                    QDir::homePath()).toStdString();

        loadProject(filepath);
    });
}

DataViewerWindow::~DataViewerWindow()
{
    delete ui;
}

void
DataViewerWindow::loadProject(
    const std::string &projectDir)
{
    closeProject();
    spdlog::info("loading project '{}'",projectDir);
    
    bool loadOkay = proj_.load(projectDir);
    if (loadOkay)
    {
        // update recent projects history and refresh menu
        gpo::ProjectSettings::setMostRecentProject(projectDir.c_str());
        populateRecentProjects();

        ui->trackView->setTrack(proj_.getTrack());
    }
    else
    {
        spdlog::error("Failed to open project at path '{}'",projectDir);
    }

    configureMenuActions();
    populateTopCompareTable();
}

void
DataViewerWindow::closeProject()
{
    if (isProjectOpened())
    {
        spdlog::info("closing project '{}'",proj_.getSavePath().c_str());
        proj_.clear();
        configureMenuActions();
        populateTopCompareTable();
    }
}

bool
DataViewerWindow::isProjectOpened() const
{
    return ! proj_.getSavePath().empty();
}

void
DataViewerWindow::populateRecentProjects()
{
    QStringList recentProjects = gpo::ProjectSettings::getRecentProjects();
    ui->menuLoad_Recent_Project->clear();
    ui->menuLoad_Recent_Project->setEnabled(recentProjects.size() > 0);

    // populate recent projects dropdown menu
    for (const auto &projectPath : recentProjects)
    {
        auto action = new QAction(this);
        action->setText(projectPath);
        connect(action,&QAction::triggered,this,[this,projectPath]{ loadProject(projectPath.toStdString()); });
        ui->menuLoad_Recent_Project->addAction(action);
    }
}

void
DataViewerWindow::configureMenuActions()
{
    // TODO add these to the UI
    // ui->actionSave_Project->setEnabled(isProjectOpened());
    // ui->actionSave_Project_as->setEnabled(true);
    ui->actionLoad_Project->setEnabled(true);
}

void
DataViewerWindow::populateTopCompareTable()
{
    topCompareTableModel_->clear();

    const auto &dsm = proj_.dataSourceManager();
    for (size_t ss=0; ss<dsm.sourceCount(); ss++)
    {
        const auto &dSrc = dsm.getSource(ss);

        QList<QStandardItem *> row;

        QStandardItem *nameItem = new QStandardItem;
        nameItem->setText(dSrc->getSourceName().c_str());
        row.append(nameItem);

        topCompareTableModel_->appendRow(row);
    }
}
