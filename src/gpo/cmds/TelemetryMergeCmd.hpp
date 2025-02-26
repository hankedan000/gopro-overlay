#pragma once

#include <QApplication>

#include "cmds/Command.hpp"
#include "TelemetryMerger.h"

namespace gpo
{
    class TelemetryMergeCmd : public Command
    {
        struct Args
        {
            static constexpr std::string_view TELEM_FILES = "telem-file";
        };

    public:
        TelemetryMergeCmd()
         : Command("telemetry-merge")
        {
            parser().add_description(
                "tool for viewing/aligning telemetry data from two sources");

            parser().add_argument(Args::TELEM_FILES)
                .help("path to a telemetry source file (video, ECU datalog, CSV, etc.)")
                .nargs(argparse::nargs_pattern::at_least_one);
        }
        
        int
        exec() final
        {
            int argc = 0;
            char ** argv = nullptr;
            QApplication app(argc, argv);

            TelemetryMerger merger;
            merger.show();

            for (const auto & telemFile : parser().get<std::vector<std::string>>(Args::TELEM_FILES))
            {
                merger.addSourceFromFile(telemFile);
            }

            return app.exec();
        }
    };
}
