#ifndef DATAVIEWERWINDOW_H
#define DATAVIEWERWINDOW_H

#include <QMainWindow>

#include "GoProOverlay/data/RenderProject.h"

namespace Ui {
class DataViewerWindow;
}

class DataViewerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DataViewerWindow(QWidget *parent = nullptr);
    ~DataViewerWindow();

    void
    loadProject(
        const std::string &projectDir);
    
    void
    closeProject();

    bool
    isProjectOpened() const;

private:
    void
    populateRecentProjects();

    void
    configureMenuActions();

private:
    Ui::DataViewerWindow *ui;

    gpo::RenderProject proj_;

};

#endif // DATAVIEWERWINDOW_H
