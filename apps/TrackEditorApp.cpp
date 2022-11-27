#include "trackeditor.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    std::string videoFile = "/home/daniel/Downloads/Autocross/20220918_GCAC/GH010143.MP4";
	QApplication a(argc, argv);

    TrackEditor te;
    te.show();
    te.loadTrackFromVideo(videoFile);

	return a.exec();
}
