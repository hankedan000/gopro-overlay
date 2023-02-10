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
		const gpo::GoProDataAvailBitSet &gpAvail,
		const gpo::ECU_DataAvailBitSet &ecuAvail,
		const gpo::TrackDataAvailBitSet &trackAvail);

    bool
    writeTelemetryToCSV(
        const gpo::TelemetrySamplesPtr &tSamps,
        const std::filesystem::path &csvFilepath,
		const gpo::GoProDataAvailBitSet &gpAvail,
		const gpo::ECU_DataAvailBitSet &ecuAvail,
		const gpo::TrackDataAvailBitSet &trackAvail);
    
	bool
    readTelemetryFromCSV(
        const std::filesystem::path &csvFilepath,
        gpo::TelemetrySamplesPtr tSamps,
		gpo::GoProDataAvailBitSet &gpAvail,
		gpo::ECU_DataAvailBitSet &ecuAvail,
		gpo::TrackDataAvailBitSet &trackAvail);

	std::pair<bool, gpo::ECU_DataAvailBitSet>
	readMegaSquirtLog(
		const std::string mslPath,
		std::vector<gpo::ECU_TimedSample> &ecuTelem);
}
}