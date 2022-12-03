#include "projectwindow.h"
#include "ui_projectwindow.h"

#include <QFileDialog>
#include <QMessageBox>

const QString RECENT_PROJECT_KEY = "RECENT_PROJECTS";
const int MAX_RECENT_PROJECTS = 10;

// constants for the rendered entity tables
const int ENTITY_NAME_COLUMN       = 0;
const int ENTITY_TYPE_COLUMN       = 1;
const int ENTITY_VISIBILITY_COLUMN = 2;

const QString WINDOW_TITLE = "Render Editor";

ProjectWindow::ProjectWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProjectWindow),
    previewResolutionActionGroup_(new QActionGroup(this)),
    settings(QSettings::Format::NativeFormat, QSettings::UserScope, "ProjectWindow", "GoProOverlay Render Project Application"),
    currProjectDir_(),
    proj_(),
    sourcesTableModel_(new QStandardItemModel(0,3,this)),
    entitiesTableModel_(new QStandardItemModel(0,3,this)),
    alignmentTableModel_(new QStandardItemModel(0,2,this)),
    trackEditor_(new TrackEditor(this)),
    previewWindow_(new ScrubbableVideo()),
    reWizTopBot_(new RenderEngineWizard_TopBottom(this,&proj_)),
    projectDirty_(false)
{
    ui->setupUi(this);
    this->setWindowTitle(WINDOW_TITLE);
    ui->stopButton->setVisible(false);
    ui->customAlignmentTable->setVisible(false);
    ui->customAlignmentCheckBox->setChecked(false);
    previewWindow_->setWindowTitle("Render Preview");
    previewResolutionActionGroup_->addAction(ui->action960_x_540);
    previewResolutionActionGroup_->addAction(ui->action1280_x_720);
    previewResolutionActionGroup_->addAction(ui->action1920_x_1080);

    // menu actions
    connect(ui->actionNew_Project, &QAction::triggered, this, [this]{
        if (maybeSave())
        {
            if (currProjectDir_.empty())
            {
                onActionSaveProjectAs();
            }
            else
            {
                onActionSaveProject();
            }
        }

        // clear old project
        proj_.clear();
        currProjectDir_.clear();
        projectDirty_ = false;

        // reset UI elements related to project
        reloadDataSourceTable();
        reloadRenderEntitiesTable();
        populateRecentProjects();
        updateTrackPane();
        updatePreviewWindowWithNewEngine(
                    proj_.getEngine(),
                    false,// we'll close it out selves below
                    false);// hold off render until below
        updateAlignmentPane();
        previewWindow_->hide();
        render();
        trackEditor_->hide();
        trackEditor_->setTrack(nullptr);
    });
    connect(ui->actionSave_Project, &QAction::triggered, this, &ProjectWindow::onActionSaveProject);
    connect(ui->actionSave_Project_as, &QAction::triggered, this, &ProjectWindow::onActionSaveProjectAs);
    connect(ui->actionLoad_Project, &QAction::triggered, this, &ProjectWindow::onActionLoadProject);
    connect(ui->actionImport_Sources, &QAction::triggered, this, &ProjectWindow::onActionImportSources);
    connect(ui->actionShow_Render_Preview, &QAction::triggered, this, [this]{previewWindow_->show();});
    connect(ui->action960_x_540, &QAction::triggered, this, [this]{
        previewWindow_->setSize(cv::Size(960,540));
    });
    connect(ui->action1280_x_720, &QAction::triggered, this, [this]{
        previewWindow_->setSize(cv::Size(1280,720));
    });
    connect(ui->action1920_x_1080, &QAction::triggered, this, [this]{
        previewWindow_->setSize(cv::Size(1920,1080));
    });

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
            int res = msgBox->exec();
            if (res != QMessageBox::Ok)
            {
                return;
            }
        }

        QString sourceName = ui->newTrackSourceCombo->currentText();
        auto &dsm = proj_.dataSourceManager();
        auto dScr = dsm.getSourceByName(sourceName.toStdString());
        auto newTrack = dScr->makeTrack();
        proj_.setTrack(newTrack);
        setProjectDirty(true);

        trackEditor_->setTrack(newTrack);
        updateTrackPane();
    });
    connect(ui->entryRadio, &QRadioButton::toggled, this, [this]{seekEngineToAlignment();render();});
    connect(ui->exitRadio, &QRadioButton::toggled, this, [this]{seekEngineToAlignment();render();});
    connect(ui->exportButton, &QPushButton::clicked, this, [this]{
        rThread_ = new RenderThread(
                    proj_.getEngine(),
                    "render.mp4",
                    60);// FIXME get FPS from source video
        connect(rThread_, &RenderThread::progressChanged, this, [this](qulonglong progress, qulonglong total){
            printf("progress: %lld/%lld\n",progress,total);
        });
        connect(rThread_, &RenderThread::finished, this, [this]{
            printf("render finished!\n");
            ui->exportButton->setVisible(true);
            ui->stopButton->setVisible(false);
        });
        rThread_->start();
        ui->exportButton->setVisible(false);
        ui->stopButton->setVisible(true);
    });
    connect(ui->stopButton, &QPushButton::clicked, this, [this]{
        rThread_->stopRender();
    });
    connect(ui->customAlignmentCheckBox, &QCheckBox::stateChanged, this, [this]{
        ui->customAlignmentTable->setVisible(ui->customAlignmentCheckBox->isChecked());
    });

    // connect engines wizards.
    connect(reWizTopBot_, &RenderEngineWizard_TopBottom::created, this, &ProjectWindow::onEngineCreated);

    connect(trackEditor_, &TrackEditor::trackModified, this, [this]{
        setProjectDirty(true);
        proj_.reprocessDatumTrack();
        updateAlignmentPane();
    });

    // attach table models
    ui->tableDataSources->setModel(sourcesTableModel_);
    clearDataSourcesTable();// calling this to set table headers
    ui->renderEntitiesTable->setModel(entitiesTableModel_);
    clearRenderEntitiesTable();// calling this to set table headers
    ui->customAlignmentTable->setModel(alignmentTableModel_);

    connect(entitiesTableModel_, &QStandardItemModel::dataChanged, this, [this](
            const QModelIndex &topLeft,
            const QModelIndex &bottomRight,
            const QVector<int> &roles){
        int editWidth = bottomRight.column() - topLeft.column();
        int editHeight = topLeft.row() - bottomRight.row();
        if (editWidth != 0 || editHeight != 0)
        {
            printf("multi-cell edit in EntityTable is not supported\n");
            return;
        }

        const auto row = topLeft.row();
        const auto col = topLeft.column();
        QStandardItem *entityNameItem = entitiesTableModel_->item(row,ENTITY_NAME_COLUMN);
        // recover the encoded pointer to a RenderedEntity within the name's data()
        QVariant v = entityNameItem->data();
        auto re = reinterpret_cast<gpo::RenderEngine::RenderedEntity *>(v.toULongLong());
        if (col == ENTITY_VISIBILITY_COLUMN)
        {
            QStandardItem *visItem = entitiesTableModel_->item(row,col);
            re->rObj->setVisible(visItem->checkState() == Qt::CheckState::Checked);
            setProjectDirty(true);
            render();
        }
    });

    setProjectDirty(false);
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
    setProjectDirty(false);
}

bool
ProjectWindow::saveProject(
        const std::string &projectDir)
{
    bool success = proj_.save(projectDir);
    setProjectDirty( ! success);
    return success;
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
        setProjectDirty(false);
        addProjectToRecentHistory(projectDir);

        // update menus that change based on project
        reloadDataSourceTable();
        reloadRenderEntitiesTable();
        populateRecentProjects();
        updateTrackPane();
        updatePreviewWindowWithNewEngine(
                    proj_.getEngine(),
                    false,// don't show until we seek below
                    false);// holdoff render untill we seek below

        updateAlignmentPane();
        previewWindow_->show();
        render();

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
        setProjectDirty(true);
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
        nameItem->setData(QVariant(reinterpret_cast<qulonglong>((void*)(&entity))));
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
        gpo::RenderEnginePtr newEngine,
        bool updateVisibility,
        bool updateRender)
{
    previewWindow_->setEngine(newEngine);

    if (updateVisibility)
    {
        previewWindow_->setVisible(newEngine != nullptr);
    }

    // render initial frame
    if (newEngine && updateRender)
    {
        render();
    }
}

void
ProjectWindow::updateAlignmentPane()
{
    auto gSeeker = proj_.getEngine()->getSeeker();
    ui->lapSpinBox->setMinimum(gSeeker->minLapCount());
    ui->lapSpinBox->setMaximum(gSeeker->maxLapCount());
    ui->lapSpinBox->setValue(ui->lapSpinBox->minimum());

    // update custom alignment table
    alignmentTableModel_->clear();
    alignmentTableModel_->setHorizontalHeaderLabels({"Name", "Index"});
    for (size_t i=0; i<gSeeker->seekerCount(); i++)
    {
        auto seeker = gSeeker->getSeeker(i);

        QList<QStandardItem *> row;

        QStandardItem *nameItem = new QStandardItem;
        nameItem->setText("Source");
        nameItem->setData(QVariant(reinterpret_cast<qulonglong>((void*)(seeker.get()))));
        row.append(nameItem);

        QStandardItem *indexItem = new QStandardItem;
        indexItem->setText(std::to_string(seeker->seekedIdx()).c_str());
        row.append(indexItem);

        alignmentTableModel_->appendRow(row);
    }

    seekEngineToAlignment();
}

void
ProjectWindow::seekEngineToAlignment()
{
    int lap = ui->lapSpinBox->value();
    auto engine = proj_.getEngine();
    auto gSeeker = engine->getSeeker();
    if (lap > 0)
    {
        if (ui->entryRadio->isChecked())
        {
            gSeeker->seekAllToLapEntry(lap);
        }
        else
        {
            gSeeker->seekAllToLapExit(lap);
        }
    }
    else
    {
        gSeeker->seekAllToIdx(0);
    }
}

void
ProjectWindow::render()
{
    auto engine = proj_.getEngine();
    if (engine)
    {
        engine->render();
        if (previewWindow_->isVisible())
        {
            previewWindow_->showImage(engine->getFrame());
        }
    }
}

void
ProjectWindow::setProjectDirty(
        bool dirty)
{
    projectDirty_ = dirty;
    QString title = WINDOW_TITLE;
    if ( ! currProjectDir_.empty())
    {
        title += " - ";
        title += currProjectDir_.c_str();
    }
    if (projectDirty_)
    {
        title += " *";
    }
    setWindowTitle(title);
}

void
ProjectWindow::closeEvent(
        QCloseEvent *event)
{
    if (maybeSave())
    {
        if (currProjectDir_.empty())
        {
            onActionSaveProjectAs();
        }
        else
        {
            onActionSaveProject();
        }
    }
    QApplication::closeAllWindows();
}

bool
ProjectWindow::maybeSave()
{
    if (projectDirty_)
    {
        auto msgBox = new QMessageBox(
                    QMessageBox::Icon::Warning,
                    "Unsaved Changes",
                    "Your project has unsaved changes!\nDo you want to save before closing?",
                    QMessageBox::Yes | QMessageBox::No);
        return msgBox->exec() == QMessageBox::Yes;
    }
    return false;
}

void
ProjectWindow::addProjectToRecentHistory(
        const std::string &projectDir)
{
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
        addProjectToRecentHistory(filepath);
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
    setProjectDirty(true);
    reloadRenderEntitiesTable();
    updatePreviewWindowWithNewEngine(
                newEngine,
                true,// updateVisibilty
                true);// updateRender
}
