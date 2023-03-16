#include "GoProOverlay/utils/io/csv.h"

#include <csv.hpp>
#include <fstream>
#include <spdlog/spdlog.h>
#include <tuple>

namespace utils
{
namespace io
{

	std::pair<bool, gpo::DataAvailableBitSet>
	readMegaSquirtLog(
		const std::string mslPath,
		std::vector<gpo::ECU_TimedSample> &ecuTelem)
	{
		std::pair<bool, gpo::DataAvailableBitSet> ret;
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
				bitset_set_bit(ecuDataAvail,gpo::eDA_ECU_TIME);
				timeColIdx = cc;
			}
			else if (engineRPM_ColIdx < 0 && colName.compare("RPM") == 0)
			{
				bitset_set_bit(ecuDataAvail,gpo::eDA_ECU_ENGINE_SPEED);
				engineRPM_ColIdx = cc;
			}
			else if (tpsColIdx < 0 && colName.compare("TPS") == 0)
			{
				bitset_set_bit(ecuDataAvail,gpo::eDA_ECU_TPS);
				tpsColIdx = cc;
			}
			else if (boostColIdx < 0 && colName.compare("Boost psi") == 0)
			{
				bitset_set_bit(ecuDataAvail,gpo::eDA_ECU_BOOST);
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