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
    sectorTableModel_->setHorizontalHeaderLabels({"Sector Name",""});

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
            releaseTrack();
            track_ = newTrack;
            trackView_->setTrack(newTrack);

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
        trackView_->setPlacementMode(TrackView::PlacementMode::ePM_StartGate);
    }
    else
    {
        trackView_->setPlacementMode(TrackView::PlacementMode::ePM_None);
    }
}

void
MainWindow::setFinishToggled(
        bool checked)
{
    if (checked)
    {
        ui->setStartButton->setChecked(false);
        trackView_->setPlacementMode(TrackView::PlacementMode::ePM_FinishGate);
    }
    else
    {
        trackView_->setPlacementMode(TrackView::PlacementMode::ePM_None);
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
            break;
        case TrackView::PlacementMode::ePM_FinishGate:
            track_->setFinish(pathIdx);
            trackView_->setPlacementMode(TrackView::PlacementMode::ePM_None);
            ui->setFinishButton->setChecked(false);
            break;
        default:
            break;
    }
}

void
MainWindow::addSectorPressed()
{
    unsigned int nSectors = sectorTableModel_->rowCount();
    QList<QStandardItem *> row;
    QStandardItem *item1 = new QStandardItem;
    QString name = "Sector";
    name.append(std::to_string(nSectors).c_str());
    item1->setText(name);
    row.append(item1);
    sectorTableModel_->appendRow(row);
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
}

void
MainWindow::configureFileMenuButtons()
{
    ui->actionSave_Track->setEnabled(track_ != nullptr && ! filepathToSaveTo_.empty());
    ui->actionSave_Track_as->setEnabled(track_ != nullptr);
    ui->actionLoad_Track->setEnabled(true);
}

