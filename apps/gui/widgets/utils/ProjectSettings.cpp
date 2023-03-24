#include "utils/ProjectSettings.h"

#include <spdlog/spdlog.h>

#include "GoProOverlay/data/RenderProject.h"

namespace gpo
{

const QString RECENT_PROJECT_KEY = "RECENT_PROJECTS";
const int MAX_RECENT_PROJECTS = 10;

// init static members of ProjectSettings
QSettings ProjectSettings::settings_ = QSettings(
    QSettings::Format::NativeFormat,
    QSettings::UserScope,
    "GoProOverlay",
    "ProjectSettings");

ProjectSettings::ProjectSettings()
{
    removeInvalidRecentProjects();
}

QStringList
ProjectSettings::getRecentProjects()
{
    return settings_.value(RECENT_PROJECT_KEY, QStringList()).toStringList();
}

void
ProjectSettings::setMostRecentProject(
    const QString &projectPath)
{
    QStringList recentProjects = getRecentProjects();
    recentProjects.removeAll(projectPath);
    recentProjects.push_front(projectPath);
    if (recentProjects.size() >= MAX_RECENT_PROJECTS)
    {
        recentProjects.pop_back();// remove oldests
    }
    settings_.setValue(RECENT_PROJECT_KEY, recentProjects);
}

unsigned int
ProjectSettings::removeInvalidRecentProjects()
{
    QStringList recentProjects = getRecentProjects();

    // find projects in history that no longer exists or are deemed invalid
    std::vector<QString> projectsToRemove;
    for (const auto &projectPath : recentProjects)
    {
        if ( ! gpo::RenderProject::isValidProject(projectPath.toStdString()))
        {
            spdlog::warn("recent project '{}' no longer exists. removing it from history.",projectPath.toStdString());
            projectsToRemove.push_back(projectPath);
        }
    }

    // remove the projects from history
    if (projectsToRemove.size() > 0)
    {
        for (const auto &project : projectsToRemove)
        {
            recentProjects.removeOne(project);
        }
        settings_.setValue(RECENT_PROJECT_KEY,recentProjects);
    }

    return projectsToRemove.size();
}

}