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
        std::ostream &out,
		const gpo::DataAvailableBitSet &avail);

    bool
    writeTelemetryToCSV(
        const gpo::TelemetrySamplesPtr &tSamps,
        const std::filesystem::path &csvFilepath,
		const gpo::DataAvailableBitSet &avail);
    
	bool
    readTelemetryFromCSV(
        const std::filesystem::path &csvFilepath,
        gpo::TelemetrySamplesPtr tSamps,
		gpo::DataAvailableBitSet &avail);

	std::pair<bool, gpo::DataAvailableBitSet>
	readMegaSquirtLog(
		const std::string mslPath,
		std::vector<gpo::ECU_TimedSample> &ecuTelem);
}
}