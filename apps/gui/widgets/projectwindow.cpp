#include "projectwindow.h"
#include "ui_projectwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <spdlog/spdlog.h>

const QString RECENT_PROJECT_KEY = "RECENT_PROJECTS";
const int MAX_RECENT_PROJECTS = 10;

// constants for the rendered entity tables
const int ENTITY_NAME_COLUMN       = 0;
const int ENTITY_TYPE_COLUMN       = 1;
const int ENTITY_VISIBILITY_COLUMN = 2;

// constatns for the custom alginment table
const int CUSTOM_ALIGN_NAME_COLUMN = 0;
const int CUSTOM_ALIGN_SPINBOX_COLUMN = 1;

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
    trackEditor_(new TrackEditor(this)),
    previewWindow_(new ScrubbableVideo()),
    reWizSingle_(new RenderEngineWizardSingleVideo(this,&proj_)),
    reWizTopBot_(new RenderEngineWizard_TopBottom(this,&proj_)),
    projectDirty_(false),
    progressDialog_(new ProgressDialog(this)),
    renderEntityPropertiesTab_(new RenderEntityPropertiesTab(this)),
    telemPlotAcclX_(new TelemetryPlotDialog(this))
{
    ui->setupUi(this);
    renderEntityPropertiesTab_->setProject(&proj_);
    this->setWindowTitle(WINDOW_TITLE);
    ui->stopButton->setVisible(false);
    ui->customAlignmentTableWidget->hide();
    ui->customAlignmentCheckBox->setChecked(false);
    // disable these boxes until track is defined
    ui->renderEngineBox->setEnabled(false);
    ui->alignAndExportBox->setEnabled(false);
    ui->propertiesTab->setLayout(new QGridLayout(this));
    ui->propertiesTab->layout()->addWidget(renderEntityPropertiesTab_);
    progressDialog_->setWindowTitle("Render Progress");
    auto flags = progressDialog_->windowFlags();// disable progress window's close button
    flags = flags & (~Qt::WindowCloseButtonHint);
    progressDialog_->setWindowFlags(flags);
    previewWindow_->setWindowTitle("Render Preview");
    previewResolutionActionGroup_->addAction(ui->action960_x_540);
    previewResolutionActionGroup_->addAction(ui->action1280_x_720);
    previewResolutionActionGroup_->addAction(ui->action1920_x_1080);

    // setup default export file (until project is saved that is...)
    std::filesystem::path exportFilePath = RenderThread::DEFAULT_EXPORT_DIR;
    exportFilePath /= RenderThread::DEFAULT_EXPORT_FILENAME;
    ui->exportFileLineEdit->setText(exportFilePath.c_str());

    telemPlotAcclX_->setWindowTitle("Acceleration X");
    telemPlotAcclX_->plot()->setX_Component(TelemetryPlot::X_Component::eXC_Samples);
    telemPlotAcclX_->plot()->setY_Component(TelemetryPlot::Y_Component::eYC_ACCL_X);
    telemPlotAcclX_->show();

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
        // disable these boxes until track is defined
        ui->renderEngineBox->setEnabled(false);
        ui->alignAndExportBox->setEnabled(false);
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
    connect(ui->singleVideoWizard, &QPushButton::pressed, this, [this]{reWizSingle_->show();});
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
        ui->renderEngineBox->setEnabled(true);
        ui->alignAndExportBox->setEnabled(true);
        setProjectDirty(true);

        trackEditor_->setTrack(newTrack);
        updateTrackPane();
    });
    connect(ui->entryRadio, &QRadioButton::toggled, this, [this]{
        ui->resetAlignment_PushButton->setEnabled(true);
        ui->applyAlignment_PushButton->setEnabled(true);
    });
    connect(ui->exitRadio, &QRadioButton::toggled, this, [this]{
        ui->resetAlignment_PushButton->setEnabled(true);
        ui->applyAlignment_PushButton->setEnabled(true);
    });
    connect(ui->previewAlignment_PushButton, &QPushButton::clicked, this, [this]{
        seekEngineToAlignment(getAlignmentInfoFromUI(),false);
        render();
        if ( ! ui->customAlignmentCheckBox->isChecked())
        {
            updateCustomAlignmentTableValues();
        }
    });
    connect(ui->resetAlignment_PushButton, &QPushButton::clicked, this, [this]{
        resetAlignmentFromProject();
        ui->resetAlignment_PushButton->setEnabled(false);
        ui->applyAlignment_PushButton->setEnabled(false);
    });
    connect(ui->applyAlignment_PushButton, &QPushButton::clicked, this, [this]{
        applyAlignmentToProject();
        seekEngineToAlignment(getAlignmentInfoFromUI(),true);
        render();
        ui->resetAlignment_PushButton->setEnabled(false);
        ui->applyAlignment_PushButton->setEnabled(false);
    });
    connect(ui->exportFileLineEdit, &QLineEdit::editingFinished, this, [this]{
        if (currProjectDir_.empty())
        {
            // no project loaded yet
            return;
        }

        const std::filesystem::path newExportFilePath = ui->exportFileLineEdit->text().toStdString();
        if (newExportFilePath != proj_.getExportFilePath())
        {
            proj_.setExportFilePath(newExportFilePath);
            setProjectDirty(true);
        }
    });
    connect(ui->exportButton, &QPushButton::clicked, this, [this]{
        QString exportDir = RenderThread::DEFAULT_EXPORT_DIR.c_str();
        QString exportFilename = RenderThread::DEFAULT_EXPORT_FILENAME.c_str();
        std::filesystem::path userExportPath = ui->exportFileLineEdit->text().toStdString();
        if ( ! userExportPath.empty())
        {
            // decompose into export dir + filename
            exportDir = userExportPath.parent_path().c_str();
            exportFilename = userExportPath.filename().c_str();
        }

        // check to see if export would overwrite an existing file
        std::filesystem::path finalExportFile = exportDir.toStdString();
        finalExportFile /= exportFilename.toStdString();
        if (std::filesystem::exists(finalExportFile))
        {
            if ( ! askToOverwriteExport())
            {
                // user requested us to not overwrite. break out!
                return;
            }
        }

        auto engine = proj_.getEngine();
        rThread_ = new RenderThread(
                    &proj_,
                    exportDir,
                    exportFilename,
                    engine->getHighestFPS());
        connect(rThread_, &RenderThread::progressChanged, progressDialog_, &ProgressDialog::progressChanged);
        connect(rThread_, &RenderThread::finished, this, [this]{
            spdlog::info("render finished!");
            ui->exportButton->setVisible(true);
            ui->stopButton->setVisible(false);
            progressDialog_->hide();
        });
        rThread_->start();
        progressDialog_->reset();
        progressDialog_->show();
        ui->exportButton->setVisible(false);
        ui->stopButton->setVisible(true);
    });
    connect(ui->stopButton, &QPushButton::clicked, this, [this]{
        rThread_->stopRender();
    });
    connect(progressDialog_, &ProgressDialog::abortPressed, this, [this]{
        if (rThread_) rThread_->stopRender();
    });
    connect(ui->customAlignmentCheckBox, &QCheckBox::stateChanged, this, [this]{
        auto checked = ui->customAlignmentCheckBox->isChecked();
        ui->customAlignmentTableWidget->setVisible(checked);
        ui->lapAlignment_VBox->setVisible( ! checked);
        ui->resetAlignment_PushButton->setEnabled(true);
        ui->applyAlignment_PushButton->setEnabled(true);
    });
    connect(ui->leadIn_SpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this](double value){
        proj_.setLeadInSeconds(value);
        setProjectDirty(true);
    });
    connect(ui->jumpToLeadIn_ToolButton, &QToolButton::clicked, this, [this]{
        auto engine = proj_.getEngine();
        auto gSeeker = engine->getSeeker();
        gSeeker->seekToAlignmentInfo(proj_.getAlignmentInfo());
        auto leadIn = ui->leadIn_SpinBox->value();
        gSeeker->seekAllRelativeTime(leadIn * -1.0);// -1 because lead-in is defined as seconds before alignment point
        render();
    });
    connect(ui->leadOut_SpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this](double value){
        proj_.setLeadOutSeconds(value);
        setProjectDirty(true);
    });
    connect(ui->jumpToLeadOut_ToolButton, &QToolButton::clicked, this, [this]{
        spdlog::warn("jump to lead-out not implemented");
    });

    // connect engine wizards
    connect(reWizSingle_, &RenderEngineWizardSingleVideo::created, this, &ProjectWindow::onEngineCreated);
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

    connect(ui->removeEntity_ToolButton, &QToolButton::clicked, this, [this]{
        auto selectionModel = ui->renderEntitiesTable->selectionModel();
        auto selectedIndexs = selectionModel->selectedIndexes();
        if (selectedIndexs.empty())
        {
            return;// nothing to remove
        }

        // only support removal of 1 row right now
        proj_.getEngine()->removeEntity(selectedIndexs.at(0).row());
        setProjectDirty(true);
        reloadRenderEntitiesTable();
        render();
    });

    connect(entitiesTableModel_, &QStandardItemModel::dataChanged, this, [this](
            const QModelIndex &topLeft,
            const QModelIndex &bottomRight,
            const QVector<int> &roles){
        int editWidth = bottomRight.column() - topLeft.column();
        int editHeight = topLeft.row() - bottomRight.row();
        if (editWidth != 0 || editHeight != 0)
        {
            spdlog::warn("multi-cell edit in EntityTable is not supported");
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
    connect(ui->renderEntitiesTable, &QTableView::clicked, this, [this](const QModelIndex &index){
        const auto row = index.row();
        QStandardItem *entityNameItem = entitiesTableModel_->item(row,ENTITY_NAME_COLUMN);
        // recover the encoded pointer to a RenderedEntity within the name's data()
        QVariant v = entityNameItem->data();
        auto re = reinterpret_cast<gpo::RenderEngine::RenderedEntity *>(v.toULongLong());
        renderEntityPropertiesTab_->setEntity(re);
    });
    connect(renderEntityPropertiesTab_, &RenderEntityPropertiesTab::propertyChanged, this, [this]{
        setProjectDirty(true);
        render();
    });

    // scrubbable video signal handlers
    connect(previewWindow_, &ScrubbableVideo::onEntitySelected, this, [this](gpo::RenderEngine::RenderedEntity *entity){
        // when an entity is selected in the video preview, highlight it in the entity table
        // and also display it in the properties pane.
        for (int r=0; r<entitiesTableModel_->rowCount(); r++)
        {
            QStandardItem *entityNameItem = entitiesTableModel_->item(r,ENTITY_NAME_COLUMN);
            // recover the encoded pointer to a RenderedEntity within the name's data()
            QVariant v = entityNameItem->data();
            auto re = reinterpret_cast<gpo::RenderEngine::RenderedEntity *>(v.toULongLong());
            if (re == entity)
            {
                ui->renderEntitiesTable->selectRow(r);
                renderEntityPropertiesTab_->setEntity(re);
                break;
            }
        }
    });
    connect(previewWindow_, &ScrubbableVideo::onEntityMoved, this, [this](gpo::RenderEngine::RenderedEntity *entity, QPoint moveVector){
        setProjectDirty(true);
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
    if ( ! currProjectDir_.empty())
    {
        spdlog::info("closing project '{}'",currProjectDir_);
        currProjectDir_.clear();
        configureMenuActions();
        setProjectDirty(false);
    }
}

bool
ProjectWindow::saveProject(
        const std::string &projectDir)
{
    spdlog::info("saving project to '{}'",projectDir);
    bool success = proj_.save(projectDir);
    setProjectDirty( ! success);
    return success;
}

bool
ProjectWindow::loadProject(
        const std::string &projectDir)
{
    closeProject();
    spdlog::info("loading project '{}'",projectDir);

    bool loadOkay = proj_.load(projectDir);
    if (loadOkay)
    {
        currProjectDir_ = projectDir;
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
        // FIXME see issue #7
        // https://github.com/hankedan000/gopro-overlay/issues/7
        // try-catch is just a hack to prevent app from crashing in the interim
        try
        {
            seekEngineToAlignment(getAlignmentInfoFromUI(),true);
        } catch (...)
        {
            spdlog::error("caught exception!");
        }
        ui->leadIn_SpinBox->setValue(proj_.getLeadInSeconds());
        ui->leadOut_SpinBox->setValue(proj_.getLeadOutSeconds());
        std::filesystem::path exportFilePath = proj_.getExportFilePath();
        if (exportFilePath.empty())
        {
            // default to export within project directory
            exportFilePath = projectDir;
            exportFilePath /= RenderThread::DEFAULT_EXPORT_FILENAME;
        }
        ui->exportFileLineEdit->setText(exportFilePath.c_str());
        if (proj_.getEngine()->entityCount() > 0)
        {
            previewWindow_->show();
            render();
        }

        if (proj_.hasTrack())
        {
            trackEditor_->setTrack(proj_.getTrack());
            ui->renderEngineBox->setEnabled(true);
            ui->alignAndExportBox->setEnabled(true);
        }

        setProjectDirty(false);
    }
    else
    {
        spdlog::error("Failed to open project at path '{}'",projectDir);
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
        updateTrackPane();
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

    // TODO should probably do this else where
    telemPlotAcclX_->plot()->clear();
    for (size_t i=0; i<dsm.sourceCount(); i++)
    {
        auto dataSrc = dsm.getSource(i);
        if (dataSrc->hasTelemetry())
        {
            telemPlotAcclX_->plot()->addSource(dataSrc->telemSrc,false);// hold off replotting until the end
        }
    }
    telemPlotAcclX_->plot()->replot();
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

    // add a button that shows source's track view
    auto trackButtonIndex = sourcesTableModel_->index(sourcesTableModel_->rowCount()-1,2);
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
    ui->tableDataSources->setIndexWidget(trackButtonIndex,showTrackButton);
}

void
ProjectWindow::populateRecentProjects()
{
    QStringList recentProjects = settings.value(
                RECENT_PROJECT_KEY,
                QStringList()).toStringList();
    ui->menuLoad_Recent_Project->clear();
    ui->menuLoad_Recent_Project->setEnabled(recentProjects.size() > 0);

    // populate recent projects dropdown menu, keeping track of ones that no longer exist
    std::vector<QString> projectsToRemove;
    for (const auto &projectPath : recentProjects)
    {
        if ( ! gpo::RenderProject::isValidProject(projectPath.toStdString()))
        {
            spdlog::warn("recent project '{}' no longer exists. removing it from history.",projectPath.toStdString());
            projectsToRemove.push_back(projectPath);
            continue;
        }
        auto action = new QAction(this);
        action->setText(projectPath);
        connect(action,&QAction::triggered,this,[this,projectPath]{ loadProject(projectPath.toStdString()); });
        ui->menuLoad_Recent_Project->addAction(action);
    }

    // remove projects from history that no longer exist
    if (projectsToRemove.size() > 0)
    {
        for (const auto &project : projectsToRemove)
        {
            recentProjects.removeOne(project);
        }
        settings.setValue(RECENT_PROJECT_KEY,recentProjects);
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
    renderEntityPropertiesTab_->setEntity(nullptr);
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
        nameItem->setText(entity.name.c_str());
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
    auto table = ui->customAlignmentTableWidget;
    QStringList columnTitles = {"Name","Offset"};
    table->setColumnCount(columnTitles.size());
    table->setRowCount(gSeeker->seekerCount());
    table->setHorizontalHeaderLabels(columnTitles);
    for (size_t i=0; i<gSeeker->seekerCount(); i++)
    {
        auto seeker = gSeeker->getSeeker(i);

        auto lineEdit = new QLineEdit(table);
        lineEdit->setText(seeker->getDataSourceName().c_str());
        lineEdit->setReadOnly(true);
        table->setCellWidget(i,CUSTOM_ALIGN_NAME_COLUMN,lineEdit);

        auto spinbox = new QSpinBox(table);
        spinbox->setMinimum(0);
        spinbox->setMaximum(seeker->size());
        spinbox->setValue(seeker->seekedIdx());
        connect(spinbox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this,seeker](int value){
            if (ui->customAlignmentCheckBox->isChecked())
            {
                ui->resetAlignment_PushButton->setEnabled(true);
                ui->applyAlignment_PushButton->setEnabled(true);
            }
        });
        table->setCellWidget(i,CUSTOM_ALIGN_SPINBOX_COLUMN,spinbox);
    }

    resetAlignmentFromProject();
    if ( ! ui->customAlignmentCheckBox->isChecked())
    {
        updateCustomAlignmentTableValues();
    }
}

void
ProjectWindow::updateCustomAlignmentTableValues()
{
    // update custom alignment table values
    auto table = ui->customAlignmentTableWidget;
    auto gSeeker = proj_.getEngine()->getSeeker();

    for (size_t i=0; i<gSeeker->seekerCount() && i<(size_t)table->rowCount(); i++)
    {
        auto seeker = gSeeker->getSeeker(i);
        QSpinBox *spinBox = (QSpinBox*)(table->cellWidget(i,CUSTOM_ALIGN_SPINBOX_COLUMN));

        spinBox->setValue(seeker->seekedIdx());
    }
}

gpo::RenderAlignmentInfo
ProjectWindow::getAlignmentInfoFromUI()
{
    gpo::RenderAlignmentInfo rai;

    if (ui->customAlignmentCheckBox->isChecked())
    {
        // handle custom alignment
        auto table = ui->customAlignmentTableWidget;
        auto gSeeker = proj_.getEngine()->getSeeker();
        gpo::CustomAlignment customAlignment;
        for (size_t i=0; i<gSeeker->seekerCount() && i<(size_t)table->rowCount(); i++)
        {
            auto seeker = gSeeker->getSeeker(i);
            QSpinBox *spinBox = (QSpinBox*)(table->cellWidget(i,CUSTOM_ALIGN_SPINBOX_COLUMN));
            customAlignment.idxBySourceName[seeker->getDataSourceName()] = spinBox->value();
        }

        rai.initFrom(customAlignment);
    }
    else
    {
        // handle case as "lap" alignment. UI doesn't support "sector" aligntment right now
        gpo::LapAlignment lapAlignment;
        lapAlignment.lap = ui->lapSpinBox->value();
        lapAlignment.side = gpo::ElementSide_E::eES_Entry;
        if (ui->exitRadio->isChecked())
        {
            lapAlignment.side = gpo::ElementSide_E::eES_Exit;
        }

        rai.initFrom(lapAlignment);
    }

    return rai;
}

void
ProjectWindow::applyAlignmentToProject()
{
    auto rai = getAlignmentInfoFromUI();
    proj_.setAlignmentInfo(rai);
    setProjectDirty(true);
}

void
ProjectWindow::resetAlignmentFromProject()
{
    auto rai = proj_.getAlignmentInfo();

    ui->customAlignmentCheckBox->setChecked(rai.type == gpo::RenderAlignmentType_E::eRAT_Custom);

    if (rai.type == gpo::RenderAlignmentType_E::eRAT_Lap)
    {
        ui->lapSpinBox->setValue(rai.alignInfo.lap->lap);
        switch (rai.alignInfo.lap->side)
        {
            case gpo::ElementSide_E::eES_Entry:
                ui->entryRadio->setChecked(true);
                break;
            case gpo::ElementSide_E::eES_Exit:
                ui->exitRadio->setChecked(true);
                break;
        }
    }
    else if (rai.type == gpo::RenderAlignmentType_E::eRAT_Custom)
    {
        auto table = ui->customAlignmentTableWidget;
        for (const auto &customEntry : rai.alignInfo.custom->idxBySourceName)
        {
            for (int i=0; i<table->rowCount(); i++)
            {
                auto nameEdit = (QLineEdit*)(table->cellWidget(i,CUSTOM_ALIGN_NAME_COLUMN));
                if (nameEdit->text().toStdString() == customEntry.first)
                {
                    QSpinBox *spinBox = (QSpinBox*)(table->cellWidget(i,CUSTOM_ALIGN_SPINBOX_COLUMN));
                    spinBox->setValue(customEntry.second);
                    break;
                }
            }
        }
    }
}

void
ProjectWindow::seekEngineToAlignment(
        const gpo::RenderAlignmentInfo &renderAlignInfo,
        bool setAsNewAlignmentPoint)
{
    auto engine = proj_.getEngine();
    auto gSeeker = engine->getSeeker();
    gSeeker->seekToAlignmentInfo(renderAlignInfo);
    if (setAsNewAlignmentPoint)
    {
        gSeeker->setAlignmentHere();
    }
    telemPlotAcclX_->plot()->realignData();
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

bool
ProjectWindow::askToOverwriteExport()
{
    auto msgBox = new QMessageBox(
                QMessageBox::Icon::Warning,
                "Overwrite Export?",
                "File exists at the specific export path.\nDo you want to overwrite it?",
                QMessageBox::Yes | QMessageBox::No);
    return msgBox->exec() == QMessageBox::Yes;
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
ProjectWindow::plotTelemetry(
        gpo::DataSourcePtr dataSrc)
{
    auto telem = dataSrc->telemSrc;

    auto acclDialog = new TelemetryPlotDialog(this);
    auto acclPlot = acclDialog->plot();
    const size_t N_SAMPS = telem->size();
    QVector<double> accl_keys(N_SAMPS);
    QVector<double> acclX_values(N_SAMPS),acclY_values(N_SAMPS),acclZ_values(N_SAMPS);
    for (size_t i=0; i<N_SAMPS; i++)
    {
        auto &acclSamp = telem->at(i).gpSamp.accl;
        accl_keys[i] = i;
        acclX_values[i] = acclSamp.x;
        acclY_values[i] = acclSamp.y;
        acclZ_values[i] = acclSamp.z;
    }
    acclPlot->plotLayout()->insertRow(0);
    acclPlot->plotLayout()->addElement(0,0,new QCPTextElement(acclPlot,"Acceleration"));
    acclPlot->addGraph();
    acclPlot->graph(0)->setPen(QPen(Qt::red));
    acclPlot->graph(0)->setData(accl_keys,acclX_values,true);
    acclPlot->graph(0)->setName("accl_x");
    acclPlot->addGraph();
    acclPlot->graph(1)->setPen(QPen(Qt::green));
    acclPlot->graph(1)->setData(accl_keys,acclY_values,true);
    acclPlot->graph(1)->setName("accl_y");
    acclPlot->addGraph();
    acclPlot->graph(2)->setPen(QPen(Qt::blue));
    acclPlot->graph(2)->setData(accl_keys,acclZ_values,true);
    acclPlot->graph(2)->setName("accl_z");
    acclPlot->xAxis->setRange(0,N_SAMPS);
    acclPlot->xAxis->setLabel("samples");
    acclPlot->yAxis->setRange(-15,15);
    acclPlot->yAxis->setLabel("acceleration (m/s^2)");
    acclPlot->legend->setVisible(true);
    acclPlot->setInteraction(QCP::iRangeDrag,true);
    acclPlot->setInteraction(QCP::iRangeZoom,true);
    acclDialog->show();
}

void
ProjectWindow::onActionSaveProject()
{
    saveProject(currProjectDir_);
}

void
ProjectWindow::onActionSaveProjectAs()
{
    // default dialog's directory to user's home dir
    QString suggestedDir = QDir::homePath();
    if ( ! currProjectDir_.empty())
    {
        // open dialog to dir where existing project is located
        const std::filesystem::path tmpFsPath = currProjectDir_;
        suggestedDir = tmpFsPath.parent_path().c_str();
    }

    // open "Save As" dialog
    std::string filepath = QFileDialog::getExistingDirectory(
                this,
                "Save Project As",
                suggestedDir).toStdString();
    if (filepath.empty())
    {
        // dialog was close without selecting a path
        return;
    }

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
                "Open Project",
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
