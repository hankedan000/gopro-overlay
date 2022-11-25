#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <GoProOverlay/data/DataFactory.h>
#include <GoProOverlay/data/TrackDataObjects.h>
#include <fstream>
#include <yaml-cpp/yaml.h>

MainWindow::MainWindow(QWidget *parent)
 : QMainWindow(parent)
 , ui(new Ui::MainWindow)
 , trackView_(new TrackView)
 , track_(nullptr)
{
    ui->setupUi(this);
    ui->trackViewLayout->addWidget(trackView_);
    connect(ui->setStartButton, &QPushButton::toggled, this, &MainWindow::setStartToggled);
    connect(ui->setFinishButton, &QPushButton::toggled, this, &MainWindow::setFinishToggled);
    connect(ui->addSectorButton, &QPushButton::pressed, this, &MainWindow::addSectorPressed);
    connect(trackView_, &TrackView::gatePlaced, this, &MainWindow::trackViewGatePlaced);

    // Create a new model
    // QStandardItemModel(int rows, int columns, QObject * parent = 0)
    sectorTableModel_ = new QStandardItemModel(0,2,this);
    sectorTableModel_->setHorizontalHeaderLabels({"Sector Name",""});

    // Attach the model to the view
    ui->sectorTable->setModel(sectorTableModel_);

    std::string videoFile = "/home/daniel/Downloads/Autocross/20220918_GCAC/GH010143.MP4";
    gpo::Data data;
    gpo::DataFactory::loadData(videoFile,data);

    track_ = gpo::makeTrackFromTelemetry(data.telemSrc);
    trackView_->setTrack(track_);

    YAML::Node trackNode = track_->encode();
    std::ofstream ofs("track.yaml");
    ofs << trackNode;
    ofs.close();
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

