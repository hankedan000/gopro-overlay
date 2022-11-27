#include "trackeditor.h"
#include "ui_trackeditor.h"

#include <GoProOverlay/data/DataSource.h>
#include <GoProOverlay/data/TrackDataObjects.h>
#include <fstream>
#include <QFileDialog>
#include <yaml-cpp/yaml.h>

TrackEditor::TrackEditor(QWidget *parent)
 : QMainWindow(parent)
 , ui(new Ui::TrackEditor)
 , trackView_(new TrackView)
 , iOwnTrack_(parent == nullptr)
 , track_(nullptr)
 , filepathToSaveTo_()
{
    ui->setupUi(this);
    ui->trackViewLayout->addWidget(trackView_);
    ui->menubar->setVisible(iOwnTrack_);// only show menubar if running in standalone
    setWindowTitle("Track Editor");

    // button actions
    connect(ui->setStartButton, &QPushButton::toggled, this, &TrackEditor::setStartToggled);
    connect(ui->setFinishButton, &QPushButton::toggled, this, &TrackEditor::setFinishToggled);
    connect(ui->addSectorButton, &QPushButton::pressed, this, &TrackEditor::addSectorPressed);
    connect(trackView_, &TrackView::gatePlaced, this, &TrackEditor::trackViewGatePlaced);

    // menu actions
    connect(ui->actionSave_Track, &QAction::triggered, this, &TrackEditor::onActionSaveTrack);
    connect(ui->actionSave_Track_as, &QAction::triggered, this, &TrackEditor::onActionSaveTrackAs);
    connect(ui->actionLoad_Track, &QAction::triggered, this, &TrackEditor::onActionLoadTrack);

    // Create a new model
    // QStandardItemModel(int rows, int columns, QObject * parent = 0)
    sectorTableModel_ = new QStandardItemModel(0,2,this);
    clearSectorTable();// calling this to set table headers

    // Attach the model to the view
    ui->sectorTable->setModel(sectorTableModel_);
}

TrackEditor::~TrackEditor()
{
    delete ui;
    delete sectorTableModel_;
    if (iOwnTrack_ && track_)
    {
        delete track_;
        track_ = nullptr;
    }
}

bool
TrackEditor::loadTrackFromVideo(
    const std::string &filepath)
{
    gpo::DataSourcePtr data;
    bool loadOkay = gpo::loadDataFromVideo(filepath,data);
    if (loadOkay)
    {
        setWindowTitle(QStringLiteral("Track Editor - %1").arg(filepath.c_str()));
        ui->statusbar->showMessage(
                    QStringLiteral("Loaded track data from '%1'").arg(filepath.c_str()),
                    3000);// 3s
        releaseTrack();
        setTrack(gpo::makeTrackFromTelemetry(data->telemSrc));

        filepathToSaveTo_ = "";
        configureFileMenuButtons();
    }

    return loadOkay;
}

bool
TrackEditor::loadTrackFromYAML(
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
            setTrack(newTrack);

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
TrackEditor::saveTrackToYAML(
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
TrackEditor::setTrack(
        gpo::Track *track)
{
    track_ = track;
    trackView_->setTrack(track_);
    loadSectorsToTable();
}

void
TrackEditor::setStartToggled(
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
TrackEditor::setFinishToggled(
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
TrackEditor::trackViewGatePlaced(
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
TrackEditor::addSectorPressed()
{
    trackView_->setPlacementMode(TrackView::PlacementMode::ePM_SectorEntry);
    ui->statusbar->showMessage("Click to set sector's entry gate.");
}

void
TrackEditor::onActionSaveTrack()
{
    if ( ! filepathToSaveTo_.empty())
    {
        saveTrackToYAML(filepathToSaveTo_);
    }
}

void
TrackEditor::onActionSaveTrackAs()
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
TrackEditor::onActionLoadTrack()
{
    std::string filepath = QFileDialog::getOpenFileName(
                this,
                "Open Track",
                QDir::currentPath(),
                "Track files (*.yaml) ;; All files (*.*)").toStdString();

    loadTrackFromYAML(filepath);// method handles update to 'filepathToSaveTo_'
}

void
TrackEditor::releaseTrack()
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
TrackEditor::configureFileMenuButtons()
{
    ui->actionSave_Track->setEnabled(track_ != nullptr && ! filepathToSaveTo_.empty());
    ui->actionSave_Track_as->setEnabled(track_ != nullptr);
    ui->actionLoad_Track->setEnabled(true);
}

void
TrackEditor::addNewSector(
        size_t entryIdx,
        size_t exitIdx)
{
    std::string name = "Sector" + std::to_string(track_->sectorCount());
    addSectorToTable(name,entryIdx,exitIdx);
    track_->addSector(name,entryIdx,exitIdx);
}

void
TrackEditor::loadSectorsToTable()
{
    clearSectorTable();
    for (size_t ss=0; track_ && ss<track_->sectorCount(); ss++)
    {
        auto tSector = track_->getSector(ss);
        addSectorToTable(
                    tSector->getName(),
                    tSector->getEntryIdx(),
                    tSector->getExitIdx());
    }
}

void
TrackEditor::addSectorToTable(
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
TrackEditor::clearSectorTable()
{
    sectorTableModel_->clear();
    sectorTableModel_->setHorizontalHeaderLabels({"Sector Name","Entry Index","Exit Index"});
}

