#include <argparse/argparse.hpp>
#include "cmds/AlignmentPlotCmd.h"
#include "cmds/Command.h"
#include "cmds/ListOpenCL_DevicesCmd.h"
#include "cmds/SingleOverlayCmd.h"
#include "cmds/TelemetryMergeCmd.h"
#include "cmds/TopBottomOverlayCmd.h"
#include "cmds/TrackEditorCmd.h"
#include <memory>
#include "projectwindow.h"
#include <QApplication>
#include <string_view>
#include <tracy/Tracy.hpp>
#include <vector>

static const char * CMD_NAME = "gpo";

struct Args
{
    static constexpr std::string_view PROJECT_DIR = "--project_dir";
};

class GoProOverlayCmd : public gpo::Command
{
    std::vector<std::shared_ptr<gpo::Command>> subcmds_;

public:
    GoProOverlayCmd()
     : gpo::Command(CMD_NAME)
    {
        addSubCmd(std::make_shared<gpo::TrackEditorCmd>());
        addSubCmd(std::make_shared<gpo::AlignmentPlotCmd>());
        addSubCmd(std::make_shared<gpo::TelemetryMergeCmd>());
        addSubCmd(std::make_shared<gpo::SingleOverlayCmd>());
        addSubCmd(std::make_shared<gpo::TopBottomOverlayCmd>());
        addSubCmd(std::make_shared<gpo::ListOpenCL_DevicesCmd>());

        parser().add_argument(Args::PROJECT_DIR)
            .help("path to a project directory to open")
            .nargs(argparse::nargs_pattern::optional);
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

        // load the project directory if one was specified
        if (parser().is_used(Args::PROJECT_DIR))
        {
            pw.loadProject(parser().get<std::string>(Args::PROJECT_DIR));
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

int
main(
    int argc,
    char *argv[])
{
    GoProOverlayCmd gpoCmd;
    gpoCmd.parseArgs(argc, argv);

#ifdef TRACY_ENABLE
    tracy::StartupProfiler();
#endif
    
    // run the command
    return gpoCmd.exec();
}
