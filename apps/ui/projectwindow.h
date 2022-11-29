#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QStandardItemModel>

#include <GoProOverlay/data/RenderProject.h>

#include "renderenginewizard_topbottom.h"
#include "trackeditor.h"

namespace Ui {
class ProjectWindow;
}

class ProjectWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ProjectWindow(QWidget *parent = nullptr);
    ~ProjectWindow();

    bool
    isProjectOpened() const;

    void
    closeProject();

    bool
    saveProject(
            const std::string &projectDir);

    bool
    loadProject(
            const std::string &projectDir);

    bool
    importVideoSource(
            const std::string &file);

private:
    void
    configureMenuActions();

    void
    clearDataSourcesTable();

    void
    reloadDataSourceTable();

    void
    addSourceToTable(
            const std::string &name,
            const std::string &originPath,
            const gpo::DataSourcePtr dSrc);

    void
    populateRecentProjects();

    void
    updateTrackPane();

private slots:
    void
    onActionSaveProject();

    void
    onActionSaveProjectAs();

    void
    onActionLoadProject();

    void
    onActionImportSources();

    void
    onActionShowTrackEditor();

private:
    Ui::ProjectWindow *ui;
    QSettings settings;

    std::string currProjectDir_;
    gpo::RenderProject proj_;

    QStandardItemModel *sourcesTableModel_;

    TrackEditor *trackEditor_;
    RenderEngineWizard_TopBottom *reWizTopBot_;
};

#endif // PROJECTWINDOW_H
