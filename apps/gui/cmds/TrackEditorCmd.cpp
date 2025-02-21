#include "cmds/TrackEditorCmd.h"

#include <QApplication>
#include "trackeditor.h"

namespace gpo
{

    static const char * CMD_NAME = "track-editor";

    struct Args
    {
        static constexpr std::string_view TRACK_FILE = "track_file";
    };

    TrackEditorCmd::TrackEditorCmd()
     : Command(CMD_NAME)
    {
        parser().add_description(
            "tool for editing track files, or viewing a video's track path");

        parser().add_argument(Args::TRACK_FILE)
            .help("path to a gopro video or track file");
    }
    
    int
    TrackEditorCmd::exec()
    {
        int argc = 0;
        char ** argv = nullptr;
        QApplication app(argc, argv);

        TrackEditor te;
        const auto trackFile = parser().get<std::string>(Args::TRACK_FILE);
        if ( ! te.loadTrackFromFile(trackFile))
        {
            printf("failed to load track from '%s'\n", trackFile.c_str());
            return -1;
        }
        te.setMenuBarVisible(true);
        te.show();

        return app.exec();
    }
}
