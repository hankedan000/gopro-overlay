#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <GoProOverlay/data/DataFactory.h>
#include <GoProOverlay/data/TrackDataObjects.h>
#include <fstream>
#include <QFileDialog>
#include <yaml-cpp/yaml.h>

MainWindow::MainWindow(QWidget *parent)
 : QMainWindow(parent)
 , ui(new Ui::MainWindow)
 , trackView_(new TrackView)
 , track_(nullptr)
 , filepathToSaveTo_()
{
    ui->setupUi(this);
    ui->trackViewLayout->addWidget(trackView_);
    setWindowTitle("Track Editor");

    // button actions
    connect(ui->setStartButton, &QPushButton::toggled, this, &MainWindow::setStartToggled);
    connect(ui->setFinishButton, &QPushButton::toggled, this, &MainWindow::setFinishToggled);
    connect(ui->addSectorButton, &QPushButton::pressed, this, &MainWindow::addSectorPressed);
    connect(trackView_, &TrackView::gatePlaced, this, &MainWindow::trackViewGatePlaced);

    // menu actions
    connect(ui->actionSave_Track, &QAction::triggered, this, &MainWindow::onActionSaveTrack);
    connect(ui->actionSave_Track_as, &QAction::triggered, this, &MainWindow::onActionSaveTrackAs);
    connect(ui->actionLoad_Track, &QAction::triggered, this, &MainWindow::onActionLoadTrack);

    // Create a new model
    // QStandardItemModel(int rows, int columns, QObject * parent = 0)
    sectorTableModel_ = new QStandardItemModel(0,2,this);
    clearSectorTable();// calling this to set table headers

    // Attach the model to the view
    ui->sectorTable->setModel(sectorTableModel_);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete sectorTableModel_;
    if (track_)
    {
        delete track_;
        track_ = nullptr;
    }
}

bool
MainWindow::loadTrackFromVideo(
    const std::string &filepath)
{
    gpo::Data data;
    bool loadOkay = gpo::DataFactory::loadData(filepath,data);
    if (loadOkay)
    {
        setWindowTitle(QStringLiteral("Track Editor - %1").arg(filepath.c_str()));
        ui->statusbar->showMessage(
                    QStringLiteral("Loaded track data from '%1'").arg(filepath.c_str()),
                    3000);// 3s
        releaseTrack();
        track_ = gpo::makeTrackFromTelemetry(data.telemSrc);
        trackView_->setTrack(track_);

        filepathToSaveTo_ = "";
        configureFileMenuButtons();
    }

    return loadOkay;
}

bool
MainWindow::loadTrackFromYAML(
    const std::string &filepath)
{
    YAML::Node trackNode = YAML::LoadFile(filepath);
    if ( ! trackNode.IsNull())
    {
        gpo::Track *newTrack = new gpo::Track();
        if (newTrack->decode(trackNode))
        {
            setWindowTitle(QStringLiteral("Track Editor - %1").arg(filepath.c_str()));
            ui->statusbar->showMessage(
                        QStringLiteral("Loaded track data from '%1'").arg(filepath.c_str()),
                        3000);// 3s
            releaseTrack();
            track_ = newTrack;
            trackView_->setTrack(newTrack);
            loadSectorsToTable();

            filepathToSaveTo_ = filepath;
            configureFileMenuButtons();
        }
        else
        {
            delete newTrack;
        }
    }
    return false;
}

bool
MainWindow::saveTrackToYAML(
    const std::string &filepath)
{
    if (track_)
    {
        YAML::Node trackNode = track_->encode();
        std::ofstream ofs(filepath);
        ofs << trackNode;
        ofs.close();
        return true;
    }
    return false;
}

void
MainWindow::setStartToggled(
        bool checked)
{
    if (checked)
    {
        ui->setFinishButton->setChecked(false);
        ui->statusbar->showMessage("Click to set starting gate.");
        trackView_->setPlacementMode(TrackView::PlacementMode::ePM_StartGate);
    }
    else
    {
        trackView_->setPlacementMode(TrackView::PlacementMode::ePM_None);
        ui->statusbar->clearMessage();
    }
}

void
MainWindow::setFinishToggled(
        bool checked)
{
    if (checked)
    {
        ui->setStartButton->setChecked(false);
        ui->statusbar->showMessage("Click to set finishing gate.");
        trackView_->setPlacementMode(TrackView::PlacementMode::ePM_FinishGate);
    }
    else
    {
        trackView_->setPlacementMode(TrackView::PlacementMode::ePM_None);
        ui->statusbar->clearMessage();
    }
}

void
MainWindow::trackViewGatePlaced(
        TrackView::PlacementMode pMode,
        size_t pathIdx)
{
    switch (pMode)
    {
        case TrackView::PlacementMode::ePM_StartGate:
            track_->setStart(pathIdx);
            trackView_->setPlacementMode(TrackView::PlacementMode::ePM_None);
            ui->setStartButton->setChecked(false);
            ui->statusbar->clearMessage();
            break;
        case TrackView::PlacementMode::ePM_FinishGate:
            track_->setFinish(pathIdx);
            trackView_->setPlacementMode(TrackView::PlacementMode::ePM_None);
            ui->setFinishButton->setChecked(false);
            ui->statusbar->clearMessage();
            break;
        case TrackView::PlacementMode::ePM_SectorEntry:
            sectorEntryIdx_ = pathIdx;
            trackView_->setPlacementMode(TrackView::PlacementMode::ePM_SectorExit);
            ui->statusbar->showMessage("Click to set sector's exit gate.");
            break;
        case TrackView::PlacementMode::ePM_SectorExit:
            addNewSector(sectorEntryIdx_,pathIdx);
            trackView_->setPlacementMode(TrackView::PlacementMode::ePM_None);
            ui->statusbar->clearMessage();
            break;
        default:
            break;
    }
}

void
MainWindow::addSectorPressed()
{
    trackView_->setPlacementMode(TrackView::PlacementMode::ePM_SectorEntry);
    ui->statusbar->showMessage("Click to set sector's entry gate.");
}

void
MainWindow::onActionSaveTrack()
{
    if ( ! filepathToSaveTo_.empty())
    {
        saveTrackToYAML(filepathToSaveTo_);
    }
}

void
MainWindow::onActionSaveTrackAs()
{
    std::string filepath = QFileDialog::getSaveFileName(
                this,
                "Save Track",
                QDir::currentPath(),
                "Track files (*.yaml) ;; All files (*.*)").toStdString();

    if (saveTrackToYAML(filepath))
    {
        filepathToSaveTo_ = filepath;
        configureFileMenuButtons();
    }
}

void
MainWindow::onActionLoadTrack()
{
    std::string filepath = QFileDialog::getOpenFileName(
                this,
                "Open Track",
                QDir::currentPath(),
                "Track files (*.yaml) ;; All files (*.*)").toStdString();

    loadTrackFromYAML(filepath);// method handles update to 'filepathToSaveTo_'
}

void
MainWindow::releaseTrack()
{
    if (track_)
    {
        delete track_;
        track_ = nullptr;
    }
    trackView_->setTrack(track_);
    clearSectorTable();
}

void
MainWindow::configureFileMenuButtons()
{
    ui->actionSave_Track->setEnabled(track_ != nullptr && ! filepathToSaveTo_.empty());
    ui->actionSave_Track_as->setEnabled(track_ != nullptr);
    ui->actionLoad_Track->setEnabled(true);
}

void
MainWindow::addNewSector(
        size_t entryIdx,
        size_t exitIdx)
{
    std::string name = "Sector" + std::to_string(track_->sectorCount());
    addSectorToTable(name,entryIdx,exitIdx);
    track_->addSector(name,entryIdx,exitIdx);
}

void
MainWindow::loadSectorsToTable()
{
    clearSectorTable();
    for (size_t ss=0; ss<track_->sectorCount(); ss++)
    {
        auto tSector = track_->getSector(ss);
        addSectorToTable(
                    tSector->getName(),
                    tSector->getEntryIdx(),
                    tSector->getExitIdx());
    }
}

void
MainWindow::addSectorToTable(
        const std::string &name,
        size_t entryIdx,
        size_t exitIdx)
{
    QList<QStandardItem *> row;

    QStandardItem *nameItem = new QStandardItem;
    nameItem->setText(name.c_str());
    row.append(nameItem);

    QStandardItem *entryItem = new QStandardItem;
    entryItem->setText(QStringLiteral("%1").arg(entryIdx));
    row.append(entryItem);

    QStandardItem *exitItem = new QStandardItem;
    exitItem->setText(QStringLiteral("%1").arg(exitIdx));
    row.append(exitItem);

    sectorTableModel_->appendRow(row);
}

void
MainWindow::clearSectorTable()
{
    sectorTableModel_->clear();
    sectorTableModel_->setHorizontalHeaderLabels({"Sector Name","Entry Index","Exit Index"});
}

