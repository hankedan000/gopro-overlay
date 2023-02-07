#include "GoProOverlay/utils/io/CSV_Utils.h"

#include <csv.hpp>
#include <fstream>
#include <spdlog/spdlog.h>
#include <tuple>

namespace utils
{
namespace io
{

	bool
    writeTelemetryToCSV(
        const gpo::TelemetrySamplesPtr &tSamps,
        std::ostream &out)
	{
		auto writer = csv::make_csv_writer(out);

		writer << std::list<std::string>({
			"time (s)",

			// gpt::CombinedSample
    		"accl_x (m/s^2)", "accl_y (m/s^2)", "accl_z (m/s^2)",
    		"gyro_x (rad/s)", "gyro_y (rad/s)", "gyro_z (rad/s)",
    		"grav_x (G)", "grav_y (G)", "grav_z (G)",
    		"cori_w", "cori_x", "cori_y", "cori_z",
    		"gps_lat (deg)", "gps_lon (deg)", "gps_altitude (m)", "gps_speed2D (m/s)", "gps_speed3D (m/s)",

			// ECU_Sample
			"engineSpeed (rpm)",
			"tps",
			"boost (psi)",

			// top-level
			"onTrackLL_lat (deg)", "onTrackLL_lon (deg)",
			"lap",
			"lapTimeOffset (s)",
			"sector",
			"sectorTimeOffset (s)"
		});

		for (const auto &samp : *tSamps)
		{
			const auto &gpSamp = samp.gpSamp;
			const auto &ecuSamp = samp.ecuSamp;

			writer << std::tuple(
				samp.t_offset,

				// gpt::CombinedSample
				gpSamp.accl.x, gpSamp.accl.y, gpSamp.accl.z,
				gpSamp.gyro.x, gpSamp.gyro.y, gpSamp.gyro.z,
				gpSamp.grav.x, gpSamp.grav.y, gpSamp.grav.z,
				gpSamp.cori.w, gpSamp.cori.x, gpSamp.cori.y, gpSamp.cori.z,
				gpSamp.gps.coord.lat, gpSamp.gps.coord.lon, gpSamp.gps.altitude, gpSamp.gps.speed2D, gpSamp.gps.speed3D,

				// ECU_Sample
				ecuSamp.engineSpeed_rpm,
				ecuSamp.tps,
				ecuSamp.boost_psi,

				// top-level
				samp.onTrackLL[0], samp.onTrackLL[1],
				samp.lap,
				samp.lapTimeOffset,
				samp.sector,
				samp.sectorTimeOffset
			);
		}

		return true;
	}

    bool
    writeTelemetryToCSV(
        const gpo::TelemetrySamplesPtr &tSamps,
        const std::filesystem::path &csvFilepath)
	{
		// open file for writing and overwrite any existing contents
		std::ofstream fs(csvFilepath,std::ios_base::trunc);
		return writeTelemetryToCSV(tSamps,fs);
	}

	#define CSV_ROW_ITR_GET(OUT_VAR,ROW_ITR,COL_IDX) OUT_VAR = (*ROW_ITR)[COL_IDX].get<std::remove_reference<decltype(OUT_VAR)>::type>()

	bool
    readTelemetryFromCSV(
        const std::filesystem::path &csvFilepath,
        gpo::TelemetrySamplesPtr tSamps)
	{
		std::ifstream ifs(csvFilepath);
		csv::CSVReader reader(ifs, csv::CSVFormat());

		auto colNames = reader.get_col_names();

		size_t rowIdx = 0;
		tSamps->clear();
		tSamps->reserve(4096);// prealloc memory
		gpo::TelemetrySample samp;
		for (auto rowItr=reader.begin(); rowItr!=reader.end(); rowItr++,rowIdx++)
		{
			size_t colIdx = 0;
			CSV_ROW_ITR_GET(samp.t_offset, rowItr, colIdx++);

			auto &gpSamp = samp.gpSamp;
			CSV_ROW_ITR_GET(gpSamp.accl.x, rowItr, colIdx++);
			CSV_ROW_ITR_GET(gpSamp.accl.y, rowItr, colIdx++);
			CSV_ROW_ITR_GET(gpSamp.accl.z, rowItr, colIdx++);
			CSV_ROW_ITR_GET(gpSamp.gyro.x, rowItr, colIdx++);
			CSV_ROW_ITR_GET(gpSamp.gyro.y, rowItr, colIdx++);
			CSV_ROW_ITR_GET(gpSamp.gyro.z, rowItr, colIdx++);
			CSV_ROW_ITR_GET(gpSamp.grav.x, rowItr, colIdx++);
			CSV_ROW_ITR_GET(gpSamp.grav.y, rowItr, colIdx++);
			CSV_ROW_ITR_GET(gpSamp.grav.z, rowItr, colIdx++);
			CSV_ROW_ITR_GET(gpSamp.cori.w, rowItr, colIdx++);
			CSV_ROW_ITR_GET(gpSamp.cori.x, rowItr, colIdx++);
			CSV_ROW_ITR_GET(gpSamp.cori.y, rowItr, colIdx++);
			CSV_ROW_ITR_GET(gpSamp.cori.z, rowItr, colIdx++);

			auto &ecuSamp = samp.ecuSamp;
			CSV_ROW_ITR_GET(ecuSamp.engineSpeed_rpm, rowItr, colIdx++);
			CSV_ROW_ITR_GET(ecuSamp.tps, rowItr, colIdx++);
			CSV_ROW_ITR_GET(ecuSamp.boost_psi, rowItr, colIdx++);

			CSV_ROW_ITR_GET(samp.onTrackLL[0], rowItr, colIdx++);
			CSV_ROW_ITR_GET(samp.onTrackLL[1], rowItr, colIdx++);
			CSV_ROW_ITR_GET(samp.lap, rowItr, colIdx++);
			CSV_ROW_ITR_GET(samp.lapTimeOffset, rowItr, colIdx++);
			CSV_ROW_ITR_GET(samp.sector, rowItr, colIdx++);
			CSV_ROW_ITR_GET(samp.sectorTimeOffset, rowItr, colIdx++);

			tSamps->push_back(samp);
		}

		return true;
	}

	std::pair<bool, gpo::ECU_DataAvailBitSet>
	readMegaSquirtLog(
		const std::string mslPath,
		std::vector<gpo::ECU_TimedSample> &ecuTelem)
	{
		std::pair<bool, gpo::ECU_DataAvailBitSet> ret;
		auto &okay = ret.first;
		auto &ecuDataAvail = ret.second;
		okay = true;
		bitset_clear(ecuDataAvail);

		csv::CSVReader reader(mslPath);

		auto colNames = reader.get_col_names();
		int timeColIdx = -1;
		int engineRPM_ColIdx = -1;
		int tpsColIdx = -1;
		int boostColIdx = -1;
		for (size_t cc=0; cc<colNames.size(); cc++)
		{
			const auto &colName = colNames.at(cc);
			if (timeColIdx < 0 && colName.compare("Time") == 0)
			{
				bitset_set_bit(ecuDataAvail,gpo::ECU_AVAIL_TIME);
				timeColIdx = cc;
			}
			else if (engineRPM_ColIdx < 0 && colName.compare("RPM") == 0)
			{
				bitset_set_bit(ecuDataAvail,gpo::ECU_AVAIL_ENGINE_SPEED);
				engineRPM_ColIdx = cc;
			}
			else if (tpsColIdx < 0 && colName.compare("TPS") == 0)
			{
				bitset_set_bit(ecuDataAvail,gpo::ECU_AVAIL_TPS);
				tpsColIdx = cc;
			}
			else if (boostColIdx < 0 && colName.compare("Boost psi") == 0)
			{
				bitset_set_bit(ecuDataAvail,gpo::ECU_AVAIL_BOOST);
				boostColIdx = cc;
			}
		}
		spdlog::debug("timeColIdx = {}", timeColIdx);
		spdlog::debug("engineRPM_ColIdx = {}", engineRPM_ColIdx);
		spdlog::debug("tpsColIdx = {}", tpsColIdx);
		spdlog::debug("boostColIdx = {}", boostColIdx);
		if (timeColIdx < 0)
		{
			spdlog::warn("could not determine 't_offset' (time) column index");
		}
		if (engineRPM_ColIdx < 0)
		{
			spdlog::warn("could not determine 'engineRPM' column index");
		}
		if (tpsColIdx < 0)
		{
			spdlog::warn("could not determine 'tps' column index");
		}
		if (boostColIdx < 0)
		{
			spdlog::warn("could not determine 'boost_psi' column index");
		}

		size_t rowIdx = 0;
		ecuTelem.clear();
		ecuTelem.reserve(1024);// prealloc memory for ~100s log (assume 10Hz data rate)
		for (auto rowItr=reader.begin(); rowItr!=reader.end(); rowItr++,rowIdx++)
		{
			if (rowIdx == 0)
			{
				// 2nd row contains units
				continue;
			}

			gpo::ECU_TimedSample ecuSamp;
			if (timeColIdx >= 0)
			{
				CSV_ROW_ITR_GET(ecuSamp.t_offset, rowItr, timeColIdx);
			}
			if (engineRPM_ColIdx >= 0)
			{
				CSV_ROW_ITR_GET(ecuSamp.sample.engineSpeed_rpm, rowItr, engineRPM_ColIdx);
			}
			if (tpsColIdx >= 0)
			{
				CSV_ROW_ITR_GET(ecuSamp.sample.tps, rowItr, tpsColIdx);
			}
			if (boostColIdx >= 0)
			{
				CSV_ROW_ITR_GET(ecuSamp.sample.boost_psi, rowItr, boostColIdx);
			}
			ecuTelem.push_back(ecuSamp);
		}

		return ret;
	}

}
}