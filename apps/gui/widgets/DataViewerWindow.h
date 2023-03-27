#ifndef DATAVIEWERWINDOW_H
#define DATAVIEWERWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>

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

    void
    populateTopCompareTable();

private:
    Ui::DataViewerWindow *ui;
    QStandardItemModel *topCompareTableModel_;
    QStandardItemModel *sectorDetailsTableModel_;

    gpo::RenderProject proj_;

};

#endif // DATAVIEWERWINDOW_H
