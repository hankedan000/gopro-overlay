#include "projectwindow.h"

#include <argparse/argparse.hpp>
#include <string_view>
#include <tracy/Tracy.hpp>
#include <QApplication>

static const char * PROG_NAME = "gopro_overlay";

struct Args
{
    static constexpr std::string_view PROJECT_DIR = "project_dir";
};

int
runProjectWindow(
    const argparse::ArgumentParser & parser)
{
    int argc = 0;
    char ** argv = nullptr;
    QApplication app(argc, argv);

    // open the project window
    ProjectWindow pw;
    pw.setWindowIcon(QIcon(":/icons/gpo_icon.ico"));
    pw.show();

    // load the project directory if one was specified
    if (parser.is_used(Args::PROJECT_DIR))
    {
        pw.loadProject(parser.get<std::string>(Args::PROJECT_DIR));
    }

    return app.exec();
}

int
main(
    int argc,
    char *argv[])
{
    argparse::ArgumentParser parser(PROG_NAME);
    parser.add_argument(Args::PROJECT_DIR)
        .help("path to a project directory to open")
        .nargs(argparse::nargs_pattern::optional);
    
    try
    {
        parser.parse_args(argc, argv);
    }
    catch (const std::exception & err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

#ifdef TRACY_ENABLE
    tracy::StartupProfiler();
#endif

    return runProjectWindow(parser);
}
