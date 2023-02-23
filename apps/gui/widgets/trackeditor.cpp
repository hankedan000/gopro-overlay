#include "trackeditor.h"
#include "ui_trackeditor.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <GoProOverlay/data/DataSource.h>
#include <GoProOverlay/data/TrackDataObjects.h>
#include <QFileDialog>
#include <yaml-cpp/yaml.h>

TrackEditor::TrackEditor(QWidget *parent)
 : QMainWindow(parent)
 , ui(new Ui::TrackEditor)
 , iOwnTrack_(false)
 , track_(nullptr)
 , sectorTableModel_(0,2,this)
 , filepathToSaveTo_()
{
    ui->setupUi(this);
    ui->menubar->setVisible(false);
    setWindowTitle("Track Editor");

    using namespace std::placeholders;
    ui->trackView->setStartGateFilter(std::bind(&TrackEditor::filterStartGate, this, _1));
    ui->trackView->setFinishGateFilter(std::bind(&TrackEditor::filterFinishGate, this, _1));
    ui->trackView->setSectorEntryFilter(std::bind(&TrackEditor::filterSectorEntryGate, this, _1));
    ui->trackView->setSectorExitFilter(std::bind(&TrackEditor::filterSectorExitGate, this, _1));

    // button actions
    connect(ui->setStartButton, &QPushButton::toggled, this, &TrackEditor::setStartToggled);
    connect(ui->setFinishButton, &QPushButton::toggled, this, &TrackEditor::setFinishToggled);
    connect(ui->addSectorButton, &QPushButton::pressed, this, &TrackEditor::addSectorPressed);
    connect(ui->trackView, &TrackView::gatePlaced, this, &TrackEditor::trackViewGatePlaced);

    // menu actions
    connect(ui->actionSave_Track, &QAction::triggered, this, &TrackEditor::onActionSaveTrack);
    connect(ui->actionSave_Track_as, &QAction::triggered, this, &TrackEditor::onActionSaveTrackAs);
    connect(ui->actionLoad_Track, &QAction::triggered, this, &TrackEditor::onActionLoadTrack);

    // Attach the model to the view
    ui->sectorTable->setModel(&sectorTableModel_);
    clearSectorTable();// calling this to set table headers
}

TrackEditor::~TrackEditor()
{
    releaseTrack();
    delete ui;
}

bool
TrackEditor::loadTrackFromVideo(
    const std::string &filepath)
{
    auto data = gpo::DataSource::loadDataFromVideo(filepath);
    if (data)
    {
        setWindowTitle(QStringLiteral("Track Editor - %1").arg(filepath.c_str()));
        ui->statusbar->showMessage(
                    QStringLiteral("Loaded track data from '%1'").arg(filepath.c_str()),
                    3000);// 3s
        setTrack(gpo::makeTrackFromTelemetry(data->telemSrc));
        iOwnTrack_ = true;

        filepathToSaveTo_ = "";
        configureFileMenuButtons();
    }

    return data != nullptr;
}

bool
TrackEditor::loadTrackFromYAML(
    const std::string &filepath)
{
    bool okay = false;
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
            setTrack(newTrack);
            iOwnTrack_ = true;

            filepathToSaveTo_ = filepath;
            configureFileMenuButtons();
            okay = true;
        }
        else
        {
            delete newTrack;
        }
    }
    return okay;
}

bool
TrackEditor::loadTrackFromFile(
    const std::string &filepath)
{
    // get the file extension in all lower case
    std::filesystem::path stdFsFilePath(filepath);
    std::string fileExt = stdFsFilePath.extension();
    std::transform(
                fileExt.begin(),
                fileExt.end(),
                fileExt.begin(),
                [](unsigned char c){ return std::tolower(c); });
    if (fileExt == ".mp4")
    {
        return loadTrackFromVideo(filepath);
    }
    else if (fileExt == ".yaml" || fileExt == ".yml")
    {
        return loadTrackFromYAML(filepath);
    }
    else
    {
        ui->statusbar->showMessage(
                    QStringLiteral("Failed to load. Unknown file extension '%1'").arg(fileExt.c_str()),
                    3000);// 3s
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
    releaseTrack();// sets iOwnTrack_ to false
    track_ = track;
    ui->trackView->setTrack(track_);
    loadSectorsToTable();
    configureFileMenuButtons();
}

void
TrackEditor::setMenuBarVisible(
    bool visible)
{
    ui->menubar->setVisible(visible);
}

void
TrackEditor::setStartToggled(
        bool checked)
{
    if (checked)
    {
        ui->setFinishButton->setChecked(false);
        ui->statusbar->showMessage("Click to set starting gate.");
        ui->trackView->setPlacementMode(TrackView::PlacementMode::ePM_StartGate);
    }
    else
    {
        ui->trackView->setPlacementMode(TrackView::PlacementMode::ePM_None);
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
        ui->trackView->setPlacementMode(TrackView::PlacementMode::ePM_FinishGate);
    }
    else
    {
        ui->trackView->setPlacementMode(TrackView::PlacementMode::ePM_None);
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
            emit trackModified();
            ui->trackView->setPlacementMode(TrackView::PlacementMode::ePM_None);
            ui->setStartButton->setChecked(false);
            ui->statusbar->clearMessage();
            break;
        case TrackView::PlacementMode::ePM_FinishGate:
            track_->setFinish(pathIdx);
            emit trackModified();
            ui->trackView->setPlacementMode(TrackView::PlacementMode::ePM_None);
            ui->setFinishButton->setChecked(false);
            ui->statusbar->clearMessage();
            break;
        case TrackView::PlacementMode::ePM_SectorEntry:
            sectorEntryIdx_ = pathIdx;
            ui->trackView->setSectorEntryIdx(pathIdx);
            ui->trackView->setPlacementMode(TrackView::PlacementMode::ePM_SectorExit);
            ui->statusbar->showMessage("Click to set sector's exit gate.");
            break;
        case TrackView::PlacementMode::ePM_SectorExit:
            addNewSector(sectorEntryIdx_,pathIdx);
            emit trackModified();
            ui->trackView->setPlacementMode(TrackView::PlacementMode::ePM_None);
            ui->statusbar->clearMessage();
            break;
        default:
            break;
    }
}

void
TrackEditor::addSectorPressed()
{
    ui->trackView->setPlacementMode(TrackView::PlacementMode::ePM_SectorEntry);
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
                "Video files (*.mp4) ;; Track files (*.yaml) ;; All files (*.*)").toStdString();

    loadTrackFromFile(filepath);// method handles update to 'filepathToSaveTo_'
}

bool
TrackEditor::filterStartGate(
    size_t pathIdx)
{
    return true;
}

bool
TrackEditor::filterFinishGate(
    size_t pathIdx)
{
    return true;
}

bool
TrackEditor::filterSectorEntryGate(
    size_t pathIdx)
{
    for (size_t ss=0; ss<track_->sectorCount(); ss++)
    {
        const auto &sector = track_->getSector(ss);
        if (sector->getEntryIdx() < pathIdx && pathIdx < sector->getExitIdx())
        {
            // gate can't fall within an existing sector
            return false;
        }
    }
    return true;
}

bool
TrackEditor::filterSectorExitGate(
    size_t pathIdx)
{
    for (size_t ss=0; ss<track_->sectorCount(); ss++)
    {
        const auto &sector = track_->getSector(ss);
        if (sector->getEntryIdx() < pathIdx && pathIdx < sector->getExitIdx())
        {
            // gate can't fall within an existing sector
            return false;
        }
        else if (sectorEntryIdx_ <= sector->getEntryIdx() && sector->getExitIdx() <= pathIdx)
        {
            // sector can't span over an existing sector
            return false;
        }
    }
    return true;
}

void
TrackEditor::releaseTrack()
{
    if (iOwnTrack_ && track_)
    {
        delete track_;
    }
    track_ = nullptr;
    iOwnTrack_ = false;
    ui->trackView->setTrack(nullptr);
    clearSectorTable();
    configureFileMenuButtons();
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
    if ( ! track_)
    {
        return;
    }

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

    sectorTableModel_.appendRow(row);
}

void
TrackEditor::clearSectorTable()
{
    sectorTableModel_.clear();
    sectorTableModel_.setHorizontalHeaderLabels({"Sector Name","Entry Index","Exit Index"});
}

