#ifndef DATAVIEWERWINDOW_H
#define DATAVIEWERWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>

#include "GoProOverlay/data/RenderProject.h"
#include "utils/QModifiableObjectObserver.h"

namespace Ui {
class DataViewerWindow;
}

class DataViewerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit
    DataViewerWindow(
        QWidget *parent = nullptr);
    
    ~DataViewerWindow();

    void
    closeEvent(
        QCloseEvent *event) override;

    void
    loadProject(
        const std::string &projectDir);

    void
    saveProject();

    void
    saveProjectAs();
    
    void
    closeProject();

    bool
    isProjectOpened() const;

private:
    void
    populateRecentProjects();

    void
    configureMenuActions();

    void
    populateTopCompareTable();

    bool
    maybeSave();

private:
    Ui::DataViewerWindow *ui;
    QStandardItemModel *topCompareTableModel_;
    QStandardItemModel *sectorDetailsTableModel_;

    gpo::RenderProject proj_;
    QModifiableObjectObserver projectObserver_;

};

#endif // DATAVIEWERWINDOW_H
