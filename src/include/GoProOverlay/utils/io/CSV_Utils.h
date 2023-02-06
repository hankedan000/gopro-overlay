#pragma once

#include <filesystem>
#include <ostream>

#include "GoProOverlay/data/TelemetrySample.h"

namespace utils
{
namespace io
{

	bool
    writeTelemetryToCSV(
        const gpo::TelemetrySamplesPtr &tSamps,
        std::ostream &out);

    bool
    writeTelemetryToCSV(
        const gpo::TelemetrySamplesPtr &tSamps,
        const std::filesystem::path &csvFilepath);

	std::pair<bool, gpo::ECU_DataAvailBitSet>
	readMegaSquirtLog(
		const std::string mslPath,
		std::vector<gpo::ECU_TimedSample> &ecuTelem);
}
}