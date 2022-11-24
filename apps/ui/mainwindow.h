#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "trackview.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void
	setStartToggled(
		bool checked);

	void
	setFinishToggled(
		bool checked);

	void
	trackViewGatePlaced(
		TrackView::PlacementMode pMode,
		gpo::DetectionGate gate);

private:
	Ui::MainWindow *ui;
	TrackView *trackView_;
	gpo::Track track_;

};
#endif // MAINWINDOW_H
