#include "cmds/AlignmentPlotCmd.h"

#include "AlignmentPlot.h"
#include "GoProOverlay/data/DataSource.h"
#include <QApplication>

namespace gpo
{

    static const char * CMD_NAME = "alignment-plot";

    struct Args
    {
        static constexpr std::string_view SRC_A_FILE = "src-a-file";
        static constexpr std::string_view SRC_B_FILE = "src-b-file";
    };

    AlignmentPlotCmd::AlignmentPlotCmd()
     : Command(CMD_NAME)
    {
        parser().add_description(
            "tool for viewing/aligning telemetry data from two sources");

        parser().add_argument(Args::SRC_A_FILE)
            .help("path to first telemetry source file (video, or ECU datalog)");

        parser().add_argument(Args::SRC_B_FILE)
            .help("path to second telemetry source file (video, or ECU datalog)");
    }

    int
    AlignmentPlotCmd::exec()
    {
        int argc = 0;
        char ** argv = nullptr;
        QApplication app(argc, argv);

        const auto srcA_File = parser().get<std::string>(Args::SRC_A_FILE);
        auto srcA = gpo::DataSource::loadDataFromFile(srcA_File);

        const auto srcB_File = parser().get<std::string>(Args::SRC_B_FILE);
        auto srcB = gpo::DataSource::loadDataFromFile(srcB_File);
        srcB->resampleTelemetry(srcA->getTelemetryRate_hz());

        AlignmentPlot plot;
        plot.setSourceA(srcA->telemSrc);
        plot.setSourceB(srcB->telemSrc);
        plot.show();

        printf("telemA.size_bytes() = %ld\n", srcA->telemSrc->size_bytes());
        printf("telemB.size_bytes() = %ld\n", srcB->telemSrc->size_bytes());

        return app.exec();
    }
}
