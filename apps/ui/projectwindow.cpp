#include "projectwindow.h"
#include "ui_projectwindow.h"

#include <QFileDialog>

ProjectWindow::ProjectWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProjectWindow),
    currProjectDir_(),
    proj_(),
    sourcesTableModel_(new QStandardItemModel(0,2,this)),
    trackEditor_(new TrackEditor(this))
{
    ui->setupUi(this);

    // menu actions
    connect(ui->actionSave_Project, &QAction::triggered, this, &ProjectWindow::onActionSaveProject);
    connect(ui->actionSave_Project_as, &QAction::triggered, this, &ProjectWindow::onActionSaveProjectAs);
    connect(ui->actionLoad_Project, &QAction::triggered, this, &ProjectWindow::onActionLoadProject);
    connect(ui->actionImport_Sources, &QAction::triggered, this, &ProjectWindow::onActionImportSources);
    connect(ui->actionShow_Track_Editor, &QAction::triggered, this, &ProjectWindow::onActionShowTrackEditor);

    // attach table models
    ui->tableDataSources->setModel(sourcesTableModel_);
    clearDataSourcesTable();// calling this to set table headers

    configureMenuActions();
}

ProjectWindow::~ProjectWindow()
{
    delete ui;
    delete sourcesTableModel_;
    delete trackEditor_;
}

bool
ProjectWindow::isProjectOpened() const
{
    return ! currProjectDir_.empty();
}

void
ProjectWindow::closeProject()
{
    currProjectDir_.clear();
    configureMenuActions();
}

bool
ProjectWindow::saveProject(
        const std::string &projectDir)
{
    return proj_.save(projectDir);
}

bool
ProjectWindow::loadProject(
        const std::string &projectDir)
{
    closeProject();

    bool loadOkay = proj_.load(projectDir);
    if (loadOkay)
    {
        currProjectDir_ = projectDir;
        reloadDataSourceTable();

        if (proj_.hasTrack())
        {
            trackEditor_->setTrack(proj_.getTrack());
            trackEditor_->show();
        }
    }

    configureMenuActions();
    return loadOkay;
}

bool
ProjectWindow::importVideoSource(
        const std::string &file)
{
    auto &dsm = proj_.dataSourceManager();
    bool importOkay = dsm.addVideo(file);
    if (importOkay)
    {
        reloadDataSourceTable();
    }
    return importOkay;
}

void
ProjectWindow::configureMenuActions()
{
    ui->actionSave_Project->setEnabled(isProjectOpened());
    ui->actionSave_Project_as->setEnabled(true);
    ui->actionLoad_Project->setEnabled(true);
}

void
ProjectWindow::clearDataSourcesTable()
{
    sourcesTableModel_->clear();
    sourcesTableModel_->setHorizontalHeaderLabels({"Source Name","Path"});
}

void
ProjectWindow::reloadDataSourceTable()
{
    auto &dsm = proj_.dataSourceManager();
    clearDataSourcesTable();
    for (size_t i=0; i<dsm.sourceCount(); i++)
    {
        addSourceToTable(
                    dsm.getSourceName(i),
                    dsm.getSourceOrigin(i),
                    dsm.getSource(i));
    }
}

void
ProjectWindow::addSourceToTable(
        const std::string &name,
        const std::string &originPath,
        const gpo::DataSourcePtr dSrc)
{
    QList<QStandardItem *> row;

    QStandardItem *nameItem = new QStandardItem;
    nameItem->setText(name.c_str());
    row.append(nameItem);

    QStandardItem *originItem = new QStandardItem;
    originItem->setText(originPath.c_str());
    row.append(originItem);

    sourcesTableModel_->appendRow(row);
}

void
ProjectWindow::onActionSaveProject()
{
    saveProject(currProjectDir_);
}

void
ProjectWindow::onActionSaveProjectAs()
{
    std::string filepath = QFileDialog::getExistingDirectory(
                this,
                "Save Project As",
                QDir::homePath()).toStdString();

    if (saveProject(filepath))
    {
        currProjectDir_ = filepath;
        configureMenuActions();
    }
}

void
ProjectWindow::onActionLoadProject()
{
    std::string filepath = QFileDialog::getExistingDirectory(
                this,
                "Save Project",
                QDir::homePath()).toStdString();

    loadProject(filepath);
}

void
ProjectWindow::onActionImportSources()
{
    QFileDialog dialog(this);
    dialog.setDirectory(QDir::homePath());
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter("Videos (*.mp4 *.MP4)");
    QStringList files;
    if (dialog.exec())
    {
        files = dialog.selectedFiles();

        for (const auto &file : files)
        {
            importVideoSource(file.toStdString());
        }
    }
}

void
ProjectWindow::onActionShowTrackEditor()
{
    trackEditor_->show();
}
