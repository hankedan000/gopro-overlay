#pragma once

#include <QSettings>

namespace gpo
{

class ProjectSettings
{
public:
    ProjectSettings();

    static
    QStringList
    getRecentProjects();

    /**
     * Call this any time you open a project. It will float that project
     * to the top of the recent project history. If the history grows past
     * its maximum size, then the oldest project will be removed.
     */
    static
    void
    setMostRecentProject(
        const QString &projectPath);

    /**
     * Searches through the recent project history and removes entries that
     * no longer reference valid projects, or projects that no longer exists.
     * 
     * @return
     * the number of projects removed from this scan
     */
    static
    unsigned int
    removeInvalidRecentProjects();

private:
    static QSettings settings_;

};

}