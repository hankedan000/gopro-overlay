#include "projectwindow.h"
#include "ui_projectwindow.h"

#include <QFileDialog>

const QString RECENT_PROJECT_KEY = "RECENT_PROJECTS";
const int MAX_RECENT_PROJECTS = 10;

ProjectWindow::ProjectWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProjectWindow),
    settings(QSettings::Format::NativeFormat, QSettings::UserScope, "ProjectWindow", "GoProOverlay Render Project Application"),
    currProjectDir_(),
    proj_(),
    sourcesTableModel_(new QStandardItemModel(0,2,this)),
    trackEditor_(new TrackEditor(this)),
    reWizTopBot_(new RenderEngineWizard_TopBottom(this,&proj_))
{
    ui->setupUi(this);

    // menu actions
    connect(ui->actionSave_Project, &QAction::triggered, this, &ProjectWindow::onActionSaveProject);
    connect(ui->actionSave_Project_as, &QAction::triggered, this, &ProjectWindow::onActionSaveProjectAs);
    connect(ui->actionLoad_Project, &QAction::triggered, this, &ProjectWindow::onActionLoadProject);
    connect(ui->actionImport_Sources, &QAction::triggered, this, &ProjectWindow::onActionImportSources);
    connect(ui->actionShow_Track_Editor, &QAction::triggered, this, &ProjectWindow::onActionShowTrackEditor);

    // connect buttons
    connect(ui->topBottomWizard, &QPushButton::pressed, this, [this]{reWizTopBot_->show();});

    // attach table models
    ui->tableDataSources->setModel(sourcesTableModel_);
    clearDataSourcesTable();// calling this to set table headers

    configureMenuActions();
    populateRecentProjects();
}

ProjectWindow::~ProjectWindow()
{
    delete ui;
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

        // add project to the recent projects history
        QStringList recentProjects = settings.value(
                    RECENT_PROJECT_KEY,
                    QStringList()).toStringList();
        recentProjects.removeAll(projectDir.c_str());
        recentProjects.push_front(projectDir.c_str());
        if (recentProjects.size() >= MAX_RECENT_PROJECTS)
        {
            recentProjects.pop_back();// remove oldests
        }
        settings.setValue(RECENT_PROJECT_KEY, recentProjects);

        // update menus that change based on project
        reloadDataSourceTable();
        populateRecentProjects();

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
ProjectWindow::populateRecentProjects()
{
    QStringList recentProjects = settings.value(
                RECENT_PROJECT_KEY,
                QStringList()).toStringList();
    ui->menuLoad_Recent_Project->clear();
    ui->menuLoad_Recent_Project->setEnabled(recentProjects.size() > 0);

    for (auto projectPath : recentProjects)
    {
        auto action = new QAction(this);
        action->setText(projectPath);
        connect(action,&QAction::triggered,this,[this,projectPath]{ loadProject(projectPath.toStdString()); });
        ui->menuLoad_Recent_Project->addAction(action);
    }
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
