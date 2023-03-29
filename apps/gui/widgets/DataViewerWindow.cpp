#include "DataViewerWindow.h"
#include "ui_DataViewerWindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <spdlog/spdlog.h>

#include "utils/ProjectSettings.h"

enum TopTableColumns
{
    eINCLUDE = 0,
    eNAME = 1,
    eLAP_TIME = 2
};

DataViewerWindow::DataViewerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DataViewerWindow),
    topCompareTableModel_(new QStandardItemModel(this)),
    sectorDetailsTableModel_(new QStandardItemModel(this)),
    proj_(),
    projectObserver_()
{
    ui->setupUi(this);
    ui->topSplitter->setSizes(QList<int>({75000, 25000}));
    ui->centralSplitter->setSizes(QList<int>{25000,75000});
    ui->topCompare_TableView->setModel(topCompareTableModel_);
    ui->sectorDetails_TableView->setModel(sectorDetailsTableModel_);

    projectObserver_.bindModifiable(&proj_);
    projectObserver_.bindWidget(this, "Data Viewer");

    populateRecentProjects();
    configureMenuActions();

    connect(ui->actionSave_Project, &QAction::triggered, this, [this]{
        saveProject();
    });
    connect(ui->actionSave_Project_as, &QAction::triggered, this, [this]{
        saveProjectAs();
    });
    connect(ui->actionLoad_Project, &QAction::triggered, this, [this]{
        std::string filepath = QFileDialog::getExistingDirectory(
                    this,
                    "Open Project",
                    QDir::homePath()).toStdString();

        loadProject(filepath);
    });
    connect(topCompareTableModel_, &QStandardItemModel::dataChanged, this, [this](
            const QModelIndex &topLeft,
            const QModelIndex &bottomRight,
            const QVector<int> &roles){
        int editWidth = bottomRight.column() - topLeft.column();
        int editHeight = topLeft.row() - bottomRight.row();
        if (editWidth != 0 || editHeight != 0)
        {
            spdlog::warn("multi-cell edit in 'topCompare_TableView' is not supported");
            return;
        }

        int row = topLeft.row();
        int col = topLeft.column();
        auto &dsm = proj_.dataSourceManager();
        QStandardItem *item = topCompareTableModel_->item(row,col);
        if (col == TopTableColumns::eINCLUDE)
        {
            ui->trackView->setSourceVisible(
                row,
                item->checkState() == Qt::CheckState::Checked,
                true);// redraw now
        }
        else if (col == TopTableColumns::eNAME)
        {
            dsm.setSourceName(row,item->text().toStdString());
        }
    });
}

DataViewerWindow::~DataViewerWindow()
{
    delete ui;
}

void
DataViewerWindow::closeEvent(
    QCloseEvent *event)
{
    if (maybeSave())
    {
        if (isProjectOpened())
        {
            saveProject();
        }
        else
        {
            saveProjectAs();
        }
    }
    QApplication::closeAllWindows();
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

        // --------------------------
        // add all entities to the trackview

        ui->trackView->setTrack(proj_.getTrack(), false);

        ui->trackView->clearSources(false);
        auto &dsm = proj_.dataSourceManager();
        for (size_t ss=0; ss<dsm.sourceCount(); ss++)
        {
            const auto &source = dsm.getSource(ss);
            if (source->hasTelemetry())
            {
                ui->trackView->addSource(source->telemSrc, false);
            }
        }

        ui->trackView->update();// redraw
    }
    else
    {
        spdlog::error("Failed to open project at path '{}'",projectDir);
    }

    configureMenuActions();
    populateTopCompareTable();
}

void
DataViewerWindow::saveProject()
{
    proj_.saveModifications(true);// true - unecessary save is ok
}

void
DataViewerWindow::saveProjectAs()
{
    // default dialog's directory to user's home dir
    QString suggestedDir = QDir::homePath();
    if (isProjectOpened())
    {
        // open dialog to dir where existing project is located
        suggestedDir = proj_.getSavePath().parent_path().c_str();
    }

    // open "Save As" dialog
    std::string filepath = QFileDialog::getExistingDirectory(
                this,
                "Save Project As",
                suggestedDir).toStdString();
    if (filepath.empty())
    {
        // dialog was close without selecting a path
        return;
    }

    if (proj_.saveModificationsAs(filepath))
    {
        configureMenuActions();
        gpo::ProjectSettings::setMostRecentProject(filepath.c_str());
    }
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
    ui->actionSave_Project->setEnabled(isProjectOpened());
    ui->actionSave_Project_as->setEnabled(isProjectOpened());
    ui->actionLoad_Project->setEnabled(true);
}

void
DataViewerWindow::populateTopCompareTable()
{
    topCompareTableModel_->clear();

    QStringList headings = {"Include", "Name", "Lap Time"};

    const auto &dsm = proj_.dataSourceManager();
    for (size_t ss=0; ss<dsm.sourceCount(); ss++)
    {
        const auto &dSrc = dsm.getSource(ss);

        QList<QStandardItem *> row;

        QStandardItem *includeItem = new QStandardItem;
        includeItem->setCheckable(true);
        includeItem->setCheckState(Qt::CheckState::Checked);
        includeItem->setToolTip("toggle inclusion in data comparison");
        row.append(includeItem);

        QStandardItem *nameItem = new QStandardItem;
        nameItem->setText(dSrc->getSourceName().c_str());
        row.append(nameItem);

        QStandardItem *lapTimeItem = new QStandardItem;
        lapTimeItem->setText("0.000");
        lapTimeItem->setEditable(false);
        row.append(lapTimeItem);

        topCompareTableModel_->appendRow(row);
    }

    topCompareTableModel_->setHorizontalHeaderLabels(headings);
}

bool
DataViewerWindow::maybeSave()
{
    if (proj_.hasSavableModifications())
    {
        auto msgBox = new QMessageBox(
                    QMessageBox::Icon::Warning,
                    "Unsaved Changes",
                    "Your project has unsaved changes!\nDo you want to save before closing?",
                    QMessageBox::Yes | QMessageBox::No);
        return msgBox->exec() == QMessageBox::Yes;
    }
    return false;
}
