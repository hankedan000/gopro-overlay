#include "trackeditor.h"

#include <filesystem>
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

    if (argc < 2)
    {
        printf("requires video or track file input\n");
        return -1;
    }

    TrackEditor te;
    std::filesystem::path trackFile = argv[1];
    if ( ! te.loadTrackFromFile(trackFile))
    {
        printf("failed to load track from '%s'\n",trackFile.c_str());
        return -1;
    }
    te.show();
    
	return a.exec();
}
