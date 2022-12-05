#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include <QActionGroup>
#include <QMainWindow>
#include <QSettings>
#include <QStandardItemModel>

#include <GoProOverlay/data/RenderProject.h>

#include "progressdialog.h"
#include "renderenginewizard_topbottom.h"
#include "renderthread.h"
#include "scrubbablevideo.h"
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

    void
    clearRenderEntitiesTable();

    void
    reloadRenderEntitiesTable();

    /**
     * @brief updatePreviewWindowWithNewEngine
     * All logic to handle setting a new RenderEngine to the preview window.
     * @param newEngine
     * the new engine to set. can be nullptr.
     *
     * @param updateVisibility
     * set to true and the preview pane will be shown or hidden based on if
     * the engine is null or not.
     *
     * @param updateRender
     * will trigger a render update after setting the engine.
     */
    void
    updatePreviewWindowWithNewEngine(
            gpo::RenderEnginePtr newEngine,
            bool updateVisibility,
            bool updateRender);

    void
    updateAlignmentPane();

    void
    updateCustomAlignmentTableValues();

    void
    seekEngineToAlignment();

    void
    render();

    void
    setProjectDirty(
            bool dirty);

    virtual
    void
    closeEvent(
            QCloseEvent *event) override;

    bool
    maybeSave();

    void
    addProjectToRecentHistory(
            const std::string &projectDir);

private slots:
    void
    onActionSaveProject();

    void
    onActionSaveProjectAs();

    void
    onActionLoadProject();

    void
    onActionImportSources();

    // connected to various RenderEnginer Wizards
    void
    onEngineCreated(
            gpo::RenderEnginePtr newEngine);

private:
    Ui::ProjectWindow *ui;
    QActionGroup *previewResolutionActionGroup_;
    QSettings settings;

    std::string currProjectDir_;
    gpo::RenderProject proj_;

    QStandardItemModel *sourcesTableModel_;
    QStandardItemModel *entitiesTableModel_;

    TrackEditor *trackEditor_;
    ScrubbableVideo *previewWindow_;
    RenderEngineWizard_TopBottom *reWizTopBot_;

    // set true if anything in the project has been modified and could use
    // a save to disk. set false once project is saved.
    bool projectDirty_;

    RenderThread *rThread_;
    ProgressDialog *progressDialog_;
};

#endif // PROJECTWINDOW_H
