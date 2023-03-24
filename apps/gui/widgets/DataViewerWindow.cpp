#include "DataViewerWindow.h"
#include "ui_DataViewerWindow.h"

#include <spdlog/spdlog.h>

#include "utils/ProjectSettings.h"

DataViewerWindow::DataViewerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DataViewerWindow)
{
    ui->setupUi(this);

    populateRecentProjects();
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
        gpo::ProjectSettings::setMostRecentProject(projectDir.c_str());
        populateRecentProjects();

        // TODO do stuff with the project on load
    }
    else
    {
        spdlog::error("Failed to open project at path '{}'",projectDir);
    }

    configureMenuActions();
}

void
DataViewerWindow::closeProject()
{
    if (isProjectOpened())
    {
        spdlog::info("closing project '{}'",proj_.getSavePath().c_str());
        proj_.clear();
        configureMenuActions();
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
