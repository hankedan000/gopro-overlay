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
{
	ui->setupUi(this);
	ui->trackViewLayout->addWidget(trackView_);
	connect(ui->setStartButton, &QPushButton::toggled, this, &MainWindow::setStartToggled);
	connect(ui->setFinishButton, &QPushButton::toggled, this, &MainWindow::setFinishToggled);
	connect(trackView_, &TrackView::gatePlaced, this, &MainWindow::trackViewGatePlaced);

	std::string videoFile = "/home/daniel/Downloads/Autocross/20220918_GCAC/GH010143.MP4";
	gpo::Data data;
	gpo::DataFactory::loadData(videoFile,data);

	track_ = gpo::makeTrackFromTelemetry(data.telemSrc);
    trackView_->setTrack(&track_);

	YAML::Node trackNode = YAML::convert<gpo::Track>::encode(track_);
	std::ofstream ofs("track.yaml");
	ofs << trackNode;
	ofs.close();
}

MainWindow::~MainWindow()
{
	delete ui;
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
		gpo::DetectionGate gate)
{
	switch (pMode)
	{
		case TrackView::PlacementMode::ePM_StartGate:
			track_.setStart(gate);
			trackView_->setPlacementMode(TrackView::PlacementMode::ePM_None);
			ui->setStartButton->setChecked(false);
			break;
		case TrackView::PlacementMode::ePM_FinishGate:
			track_.setFinish(gate);
			trackView_->setPlacementMode(TrackView::PlacementMode::ePM_None);
			ui->setFinishButton->setChecked(false);
			break;
		default:
			break;
	}
}

