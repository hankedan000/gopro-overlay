#include <memory>
#include <QApplication>
#include <string_view>
#include <tracy/Tracy.hpp>
#include <vector>

#include "cmds/AlignmentPlotCmd.hpp"
#include "cmds/Command.hpp"
#include "cmds/ListOpenCL_DevicesCmd.hpp"
#include "cmds/SingleOverlayCmd.hpp"
#include "cmds/TelemetryMergeCmd.hpp"
#include "cmds/TopBottomOverlayCmd.hpp"
#include "cmds/TrackEditorCmd.hpp"
#include "projectwindow.h"

namespace gpo
{
    
    // init static data members
    bool Command::stopRequested_ = false;

    class GoProOverlayCmd : public gpo::Command
    {
        std::vector<std::shared_ptr<gpo::Command>> subcmds_;

        struct Args
        {
            static constexpr std::string_view PROJECT_DIR = "--project_dir";
        };

    public:
        GoProOverlayCmd()
         : gpo::Command("gpo")
        {
            addSubCmd(std::make_shared<gpo::TrackEditorCmd>());
            addSubCmd(std::make_shared<gpo::AlignmentPlotCmd>());
            addSubCmd(std::make_shared<gpo::TelemetryMergeCmd>());
            addSubCmd(std::make_shared<gpo::SingleOverlayCmd>());
            addSubCmd(std::make_shared<gpo::TopBottomOverlayCmd>());
            addSubCmd(std::make_shared<gpo::ListOpenCL_DevicesCmd>());

            parser().add_argument("-p", Args::PROJECT_DIR)
                .help("optional project directory to open")
                .default_value("");
        }

        virtual
        ~GoProOverlayCmd() = default;

        void
        parseArgs(
            int argc,
            char *argv[])
        {
            try
            {
                parser().parse_args(argc, argv);
            }
            catch (const std::exception & err)
            {
                std::cerr << err.what() << std::endl;

                // first, try to display help menu for any subcmd that may have been used
                for (const auto & subcmd : subcmds_)
                {
                    if (parser().is_subcommand_used(subcmd->parser()))
                    {
                        std::cout << subcmd->parser() << std::endl;
                        exit(-1);
                    }
                }

                // subcmd must not have been used, so display main help menu
                std::cout << parser() << std::endl;
                exit(-1);
            }
        }

        void
        addSubCmd(
            std::shared_ptr<gpo::Command> cmd)
        {
            if ( ! cmd)
            {
                return;
            }

            parser().add_subparser(cmd->parser());
            subcmds_.push_back(cmd);
        }
        
        int
        runProjectWindow()
        {
            int argc = 0;
            char ** argv = nullptr;
            QApplication app(argc, argv);

            // open the project window
            ProjectWindow pw;
            pw.setWindowIcon(QIcon(":/icons/gpo_icon.ico"));
            pw.show();

            const auto projectDir = parser().get<std::string>(Args::PROJECT_DIR);
            if ( ! projectDir.empty())
            {
                pw.loadProject(projectDir);
            }

            return app.exec();
        }

        int
        exec() final
        {
            // try to run any subcommands we have registered
            for (const auto & subcmd : subcmds_)
            {
                if (parser().is_subcommand_used(subcmd->parser()))
                {
                    return subcmd->exec();
                }
            }

            // no subcmds were specified, so default to opening project window
            return runProjectWindow();
        }

    };

} // end namespace gpo

int
main(
    int argc,
    char *argv[])
{
    gpo::GoProOverlayCmd gpoCmd;
    gpoCmd.parseArgs(argc, argv);

#ifdef TRACY_ENABLE
    tracy::StartupProfiler();
#endif
    
    // run the command
    return gpoCmd.exec();
}
