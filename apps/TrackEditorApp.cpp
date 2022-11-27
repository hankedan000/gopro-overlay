#include "trackeditor.h"
#include "scrubbablevideo.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    std::string videoFile = "/home/daniel/Downloads/Autocross/20220918_GCAC/GH010143.MP4";
	QApplication a(argc, argv);

    TrackEditor te;
    te.show();
    te.loadTrackFromVideo(videoFile);

    ScrubbableVideo scrubVideo;
    scrubVideo.show();
//    imgView.show();
//    cv::Mat testImage = cv::imread("/home/daniel/Downloads/images.png");
//    imgView.setImage(testImage);

	return a.exec();
}
