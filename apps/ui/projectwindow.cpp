#include "projectwindow.h"
#include "ui_projectwindow.h"

#include <QFileDialog>
#include <QMessageBox>

const QString RECENT_PROJECT_KEY = "RECENT_PROJECTS";
const int MAX_RECENT_PROJECTS = 10;

ProjectWindow::ProjectWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProjectWindow),
    settings(QSettings::Format::NativeFormat, QSettings::UserScope, "ProjectWindow", "GoProOverlay Render Project Application"),
    currProjectDir_(),
    proj_(),
    sourcesTableModel_(new QStandardItemModel(0,3,this)),
    entitiesTableModel_(new QStandardItemModel(0,3,this)),
    trackEditor_(new TrackEditor(this)),
    previewWindow_(new ScrubbableVideo()),
    reWizTopBot_(new RenderEngineWizard_TopBottom(this,&proj_))
{
    ui->setupUi(this);
    previewWindow_->setWindowTitle("Render Preview");

    // menu actions
    connect(ui->actionSave_Project, &QAction::triggered, this, &ProjectWindow::onActionSaveProject);
    connect(ui->actionSave_Project_as, &QAction::triggered, this, &ProjectWindow::onActionSaveProjectAs);
    connect(ui->actionLoad_Project, &QAction::triggered, this, &ProjectWindow::onActionLoadProject);
    connect(ui->actionImport_Sources, &QAction::triggered, this, &ProjectWindow::onActionImportSources);
    connect(ui->actionShow_Render_Preview, &QAction::triggered, this, [this]{previewWindow_->show();});

    // connect buttons
    connect(ui->topBottomWizard, &QPushButton::pressed, this, [this]{reWizTopBot_->show();});
    connect(ui->editTrackButton, &QPushButton::pressed, this, [this]{trackEditor_->show();});
    connect(ui->newTrackButton, &QPushButton::pressed, this, [this]{
        if (proj_.hasTrack())
        {
            auto msgBox = new QMessageBox(
                        QMessageBox::Icon::Warning,
                        "Track Exists",
                        "Your project already has a Track defined.\nIf you continue, it will be overwritten.",
                        QMessageBox::Ok | QMessageBox::Cancel);
            msgBox->setModal(true);
            int res = msgBox->exec();
            if (res != QMessageBox::Ok)
            {
                return;
            }
        }

        QString sourceName = ui->newTrackSourceCombo->currentText();
        auto &dsm = proj_.dataSourceManager();
        auto dScr = dsm.getSourceByName(sourceName.toStdString());
        proj_.setTrack(dScr->makeTrack());

        updateTrackPane();
    });

    // connect engines wizards.
    connect(reWizTopBot_, &RenderEngineWizard_TopBottom::created, this, &ProjectWindow::onEngineCreated);

    // attach table models
    ui->tableDataSources->setModel(sourcesTableModel_);
    clearDataSourcesTable();// calling this to set table headers
    ui->renderEntitiesTable->setModel(entitiesTableModel_);
    clearRenderEntitiesTable();// calling this to set table headers

    configureMenuActions();
    populateRecentProjects();
    updateTrackPane();
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
        reloadRenderEntitiesTable();
        populateRecentProjects();
        updateTrackPane();
        updatePreviewWindowWithNewEngine(proj_.getEngine());

        auto gSeeker = proj_.getEngine()->getSeeker();
        ui->lapSpinBox->setMinimum(gSeeker->minLapCount());
        ui->lapSpinBox->setMaximum(gSeeker->maxLapCount());
        ui->lapSpinBox->setValue(ui->lapSpinBox->minimum());
        seekPreviewToAlignment();

        if (proj_.hasTrack())
        {
            trackEditor_->setTrack(proj_.getTrack());
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
    sourcesTableModel_->setHorizontalHeaderLabels({"Source Name","Path","Track View"});
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
    originItem->setEditable(false);
    row.append(originItem);

    QStandardItem *trackViewItem = new QStandardItem;
    row.append(trackViewItem);

    sourcesTableModel_->appendRow(row);

    // add a button the table to show source's track view
    auto buttonIndex = sourcesTableModel_->index(sourcesTableModel_->rowCount()-1,2);
    auto showTrackButton = new QPushButton(this);
    showTrackButton->setText("Show Track");
    showTrackButton->setEnabled(false);
    if (dSrc->hasTelemetry())
    {
        showTrackButton->setEnabled(true);
        connect(showTrackButton, &QPushButton::pressed, this, [this,dSrc]{
            auto trackview = new TrackView();
            trackview->setTrack(dSrc->makeTrack());
            trackview->setWindowTitle(QStringLiteral("Track View - %1").arg(dSrc->getSourceName().c_str()));
            trackview->show();
        });
    }
    ui->tableDataSources->setIndexWidget(buttonIndex,showTrackButton);
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
ProjectWindow::updateTrackPane()
{
    ui->newTrackSourceCombo->clear();
    auto &dsm = proj_.dataSourceManager();
    for (size_t ss=0; ss<dsm.sourceCount(); ss++)
    {
        auto src = dsm.getSource(ss);
        if (src->hasTelemetry())
        {
            ui->newTrackSourceCombo->addItem(src->getSourceName().c_str());
        }
    }
    ui->newTrackButton->setEnabled(ui->newTrackSourceCombo->count() > 0);
    ui->newTrackSourceCombo->setEnabled(ui->newTrackSourceCombo->count() > 0);
    ui->editTrackButton->setEnabled(proj_.hasTrack());
}

void
ProjectWindow::clearRenderEntitiesTable()
{
    entitiesTableModel_->clear();
    entitiesTableModel_->setHorizontalHeaderLabels({"Name", "Type", "Visible"});
}

void
ProjectWindow::reloadRenderEntitiesTable()
{
    clearRenderEntitiesTable();

    auto engine = proj_.getEngine();
    for (size_t ee=0; ee<engine->entityCount(); ee++)
    {
        auto &entity = engine->getEntity(ee);

        QList<QStandardItem *> row;

        QStandardItem *nameItem = new QStandardItem;
        nameItem->setText(entity.name().c_str());
        row.append(nameItem);

        QStandardItem *typeItem = new QStandardItem;
        typeItem->setText(entity.rObj->typeName().c_str());
        typeItem->setEditable(false);
        row.append(typeItem);

        QStandardItem *visibleItem = new QStandardItem;
        visibleItem->setCheckable(true);
        visibleItem->setCheckState(
                    entity.rObj->isVisible() ?
                    Qt::CheckState::Checked :
                    Qt::CheckState::Unchecked);
        row.append(visibleItem);

        entitiesTableModel_->appendRow(row);
    }
}

void
ProjectWindow::updatePreviewWindowWithNewEngine(
        gpo::RenderEnginePtr newEngine)
{
    previewWindow_->setEngine(newEngine);
    if (newEngine)
    {
        previewWindow_->show();
        newEngine->render();// render initial frame
        previewWindow_->showImage(newEngine->getFrame());
    }
    else
    {
        previewWindow_->hide();
    }
}

void
ProjectWindow::seekPreviewToAlignment()
{
    int lap = ui->lapSpinBox->value();
    auto engine = proj_.getEngine();
    auto gSeeker = engine->getSeeker();
    if (lap > 0)
    {
        gSeeker->seekAllToLapEntry(lap);
    }
    else
    {
        gSeeker->seekAllToIdx(0);
    }
    engine->render();
    previewWindow_->showImage(engine->getFrame());
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
ProjectWindow::onEngineCreated(
        gpo::RenderEnginePtr newEngine)
{
    proj_.setEngine(newEngine);
    reloadRenderEntitiesTable();
    updatePreviewWindowWithNewEngine(newEngine);
}
