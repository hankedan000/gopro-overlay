#include "GoProOverlay/utils/io/CSV_Utils.h"

#include <csv.hpp>
#include <fstream>
#include <spdlog/spdlog.h>
#include <tuple>

namespace utils
{
namespace io
{

	template <typename STRUCT_T, typename MEMBER_T, size_t FIELD_OFFSET>
	void
	csvRowToStruct(
		const csv::CSVRow &row,
		size_t colIdx,
		void *structOut)
	{
		*reinterpret_cast<MEMBER_T *>(reinterpret_cast<uint8_t*>(structOut) + FIELD_OFFSET) =
			row[colIdx].get<MEMBER_T>();
	}

	template <typename STRUCT_T, typename MEMBER_T, size_t FIELD_OFFSET>
	void
	structToCsvRow(
		const void *structIn,
		std::vector<std::string> &row)
	{
		row.push_back(std::to_string(*reinterpret_cast<const MEMBER_T *>(reinterpret_cast<const uint8_t*>(structIn) + FIELD_OFFSET)));
	}

	struct CSV_ColumnParser
	{
		const char *columnTitle;
		void (*toStruct)(const csv::CSVRow &/*row*/, size_t/*colIdx*/, void */*structOut*/);
		void (*fromStruct)(const void */*structOut*/, std::vector<std::string> &/*row*/);
	};

	#define typeof(STRUCT_T,MEMBER) decltype(reinterpret_cast<STRUCT_T*>(0)->MEMBER)

	#define MAKE_PARSER(STRUCT_T, MEMBER, COLUMN_TITLE) {\
		COLUMN_TITLE,\
		csvRowToStruct<STRUCT_T, typeof(STRUCT_T,MEMBER), offsetof(STRUCT_T,MEMBER)>,\
		structToCsvRow<STRUCT_T, typeof(STRUCT_T,MEMBER), offsetof(STRUCT_T,MEMBER)>\
	}

	static constexpr CSV_ColumnParser CSVPARSER_T_OFFSET = MAKE_PARSER(gpo::TelemetrySample, t_offset, "t_offset");

	static constexpr CSV_ColumnParser CSVPARSER_GP_ACCL_X = MAKE_PARSER(gpo::TelemetrySample, gpSamp.accl.x, "accl_x");
	static constexpr CSV_ColumnParser CSVPARSER_GP_ACCL_Y = MAKE_PARSER(gpo::TelemetrySample, gpSamp.accl.y, "accl_y");
	static constexpr CSV_ColumnParser CSVPARSER_GP_ACCL_Z = MAKE_PARSER(gpo::TelemetrySample, gpSamp.accl.z, "accl_z");
	static constexpr CSV_ColumnParser CSVPARSER_GP_GYRO_X = MAKE_PARSER(gpo::TelemetrySample, gpSamp.gyro.x, "gyro_x");
	static constexpr CSV_ColumnParser CSVPARSER_GP_GYRO_Y = MAKE_PARSER(gpo::TelemetrySample, gpSamp.gyro.y, "gyro_y");
	static constexpr CSV_ColumnParser CSVPARSER_GP_GYRO_Z = MAKE_PARSER(gpo::TelemetrySample, gpSamp.gyro.z, "gyro_z");
	static constexpr CSV_ColumnParser CSVPARSER_GP_GRAV_X = MAKE_PARSER(gpo::TelemetrySample, gpSamp.grav.x, "grav_x");
	static constexpr CSV_ColumnParser CSVPARSER_GP_GRAV_Y = MAKE_PARSER(gpo::TelemetrySample, gpSamp.grav.y, "grav_y");
	static constexpr CSV_ColumnParser CSVPARSER_GP_GRAV_Z = MAKE_PARSER(gpo::TelemetrySample, gpSamp.grav.z, "grav_z");
	static constexpr CSV_ColumnParser CSVPARSER_GP_CORI_W = MAKE_PARSER(gpo::TelemetrySample, gpSamp.cori.w, "cori_w");
	static constexpr CSV_ColumnParser CSVPARSER_GP_CORI_X = MAKE_PARSER(gpo::TelemetrySample, gpSamp.cori.x, "cori_x");
	static constexpr CSV_ColumnParser CSVPARSER_GP_CORI_Y = MAKE_PARSER(gpo::TelemetrySample, gpSamp.cori.y, "cori_y");
	static constexpr CSV_ColumnParser CSVPARSER_GP_CORI_Z = MAKE_PARSER(gpo::TelemetrySample, gpSamp.cori.z, "cori_z");
	static constexpr CSV_ColumnParser CSVPARSER_GP_GPS_LAT = MAKE_PARSER(gpo::TelemetrySample, gpSamp.gps.coord.lat, "gps_lat");
	static constexpr CSV_ColumnParser CSVPARSER_GP_GPS_LON = MAKE_PARSER(gpo::TelemetrySample, gpSamp.gps.coord.lon, "gps_lon");
	static constexpr CSV_ColumnParser CSVPARSER_GP_GPS_ALTITUDE = MAKE_PARSER(gpo::TelemetrySample, gpSamp.gps.altitude, "gps_altitude");
	static constexpr CSV_ColumnParser CSVPARSER_GP_GPS_SPEED2D = MAKE_PARSER(gpo::TelemetrySample, gpSamp.gps.speed2D, "gps_speed2D");
	static constexpr CSV_ColumnParser CSVPARSER_GP_GPS_SPEED3D = MAKE_PARSER(gpo::TelemetrySample, gpSamp.gps.speed3D, "gps_speed3D");

	static constexpr CSV_ColumnParser CSVPARSER_ECU_ENGINE_SPEED = MAKE_PARSER(gpo::TelemetrySample, ecuSamp.engineSpeed_rpm, "engineSpeed");
	static constexpr CSV_ColumnParser CSVPARSER_ECU_TPS = MAKE_PARSER(gpo::TelemetrySample, ecuSamp.tps, "tps");
	static constexpr CSV_ColumnParser CSVPARSER_ECU_BOOST = MAKE_PARSER(gpo::TelemetrySample, ecuSamp.boost_psi, "boost");

	static constexpr CSV_ColumnParser CSVPARSER_TRACK_ON_TRACK_LAT = MAKE_PARSER(gpo::TelemetrySample, trackData.onTrackLL.lat, "onTrackLL_lat");
	static constexpr CSV_ColumnParser CSVPARSER_TRACK_ON_TRACK_LON = MAKE_PARSER(gpo::TelemetrySample, trackData.onTrackLL.lon, "onTrackLL_lon");
	static constexpr CSV_ColumnParser CSVPARSER_TRACK_LAP = MAKE_PARSER(gpo::TelemetrySample, trackData.lap, "lap");
	static constexpr CSV_ColumnParser CSVPARSER_TRACK_LAP_TIME_OFFSET = MAKE_PARSER(gpo::TelemetrySample, trackData.lapTimeOffset, "lapTimeOffset");
	static constexpr CSV_ColumnParser CSVPARSER_TRACK_SECTOR = MAKE_PARSER(gpo::TelemetrySample, trackData.sector, "sector");
	static constexpr CSV_ColumnParser CSVPARSER_TRACK_SECTOR_TIME_OFFSET = MAKE_PARSER(gpo::TelemetrySample, trackData.sectorTimeOffset, "sectorTimeOffset");

	bool
    writeTelemetryToCSV(
        const gpo::TelemetrySamplesPtr &tSamps,
        std::ostream &out,
		const gpo::GoProDataAvailBitSet &gpAvail,
		const gpo::ECU_DataAvailBitSet &ecuAvail,
		const gpo::TrackDataAvailBitSet &trackAvail)
	{
		auto writer = csv::make_csv_writer(out);

		std::vector<CSV_ColumnParser> columns;
		columns.reserve(100);

		columns.push_back(CSVPARSER_T_OFFSET);

		// -----------------------------
		// GoPro telemetery
		// -----------------------------
		if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_ACCL))
		{
			columns.push_back(CSVPARSER_GP_ACCL_X);
			columns.push_back(CSVPARSER_GP_ACCL_Y);
			columns.push_back(CSVPARSER_GP_ACCL_Z);
		}
		if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_GYRO))
		{
			columns.push_back(CSVPARSER_GP_GYRO_X);
			columns.push_back(CSVPARSER_GP_GYRO_Y);
			columns.push_back(CSVPARSER_GP_GYRO_Z);
		}
		if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_GRAV))
		{
			columns.push_back(CSVPARSER_GP_GRAV_X);
			columns.push_back(CSVPARSER_GP_GRAV_Y);
			columns.push_back(CSVPARSER_GP_GRAV_Z);
		}
		if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_CORI))
		{
			columns.push_back(CSVPARSER_GP_CORI_W);
			columns.push_back(CSVPARSER_GP_CORI_X);
			columns.push_back(CSVPARSER_GP_CORI_Y);
			columns.push_back(CSVPARSER_GP_CORI_Z);
		}
		if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_GPS_LATLON))
		{
			columns.push_back(CSVPARSER_GP_GPS_LAT);
			columns.push_back(CSVPARSER_GP_GPS_LON);
		}
		if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_GPS_ALTITUDE))
		{
			columns.push_back(CSVPARSER_GP_GPS_ALTITUDE);
		}
		if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_GPS_SPEED2D))
		{
			columns.push_back(CSVPARSER_GP_GPS_SPEED2D);
		}
		if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_GPS_SPEED3D))
		{
			columns.push_back(CSVPARSER_GP_GPS_SPEED3D);
		}

		// -----------------------------
		// ECU telemetery
		// -----------------------------
		if (bitset_is_set(ecuAvail, gpo::ECU_AVAIL_ENGINE_SPEED))
		{
			columns.push_back(CSVPARSER_ECU_ENGINE_SPEED);
		}
		if (bitset_is_set(ecuAvail, gpo::ECU_AVAIL_TPS))
		{
			columns.push_back(CSVPARSER_ECU_TPS);
		}
		if (bitset_is_set(ecuAvail, gpo::ECU_AVAIL_BOOST))
		{
			columns.push_back(CSVPARSER_ECU_BOOST);
		}

		// -----------------------------
		// Processed telemetery
		// -----------------------------
		if (bitset_is_set(trackAvail, gpo::TRACK_AVAIL_ON_TRACK_LATLON))
		{
			columns.push_back(CSVPARSER_TRACK_ON_TRACK_LAT);
			columns.push_back(CSVPARSER_TRACK_ON_TRACK_LON);
		}
		if (bitset_is_set(trackAvail, gpo::TRACK_AVAIL_LAP))
		{
			columns.push_back(CSVPARSER_TRACK_LAP);
		}
		if (bitset_is_set(trackAvail, gpo::TRACK_AVAIL_LAP_TIME_OFFSET))
		{
			columns.push_back(CSVPARSER_TRACK_LAP_TIME_OFFSET);
		}
		if (bitset_is_set(trackAvail, gpo::TRACK_AVAIL_SECTOR))
		{
			columns.push_back(CSVPARSER_TRACK_SECTOR);
		}
		if (bitset_is_set(trackAvail, gpo::TRACK_AVAIL_SECTOR_TIME_OFFSET))
		{
			columns.push_back(CSVPARSER_TRACK_SECTOR_TIME_OFFSET);
		}

		std::vector<std::string> headings;
		headings.reserve(columns.size());
		for (const auto &column : columns)
		{
			headings.push_back(column.columnTitle);
		}
		writer << headings;
		std::vector<std::string> row;
		row.reserve(columns.size());
		for (const auto &samp : *tSamps)
		{
			row.clear();
			for (const auto &column : columns)
			{
				column.fromStruct(&samp, row);
			}

			writer << row;
		}

		return true;
	}

    bool
    writeTelemetryToCSV(
        const gpo::TelemetrySamplesPtr &tSamps,
        const std::filesystem::path &csvFilepath,
		const gpo::GoProDataAvailBitSet &gpAvail,
		const gpo::ECU_DataAvailBitSet &ecuAvail,
		const gpo::TrackDataAvailBitSet &trackAvail)
	{
		// open file for writing and overwrite any existing contents
		std::ofstream fs(csvFilepath,std::ios_base::trunc);
		return writeTelemetryToCSV(tSamps,fs,gpAvail,ecuAvail,trackAvail);
	}

	#define CSV_ROW_ITR_GET(OUT_VAR,ROW_ITR,COL_IDX) OUT_VAR = (*ROW_ITR)[COL_IDX].get<std::remove_reference<decltype(OUT_VAR)>::type>()

	bool
    readTelemetryFromCSV(
        const std::filesystem::path &csvFilepath,
        gpo::TelemetrySamplesPtr tSamps,
		gpo::GoProDataAvailBitSet &gpAvail,
		gpo::ECU_DataAvailBitSet &ecuAvail,
		gpo::TrackDataAvailBitSet &trackAvail)
	{
		std::ifstream ifs(csvFilepath);
		csv::CSVReader reader(ifs, csv::CSVFormat());

		bitset_clear(gpAvail);
		bitset_clear(ecuAvail);
		bitset_clear(trackAvail);

		auto colNames = reader.get_col_names();
		std::vector<CSV_ColumnParser> columns;
		columns.reserve(100);
		for (const auto &colName : colNames)
		{
			if (colName == CSVPARSER_T_OFFSET.columnTitle)
			{
				columns.push_back(CSVPARSER_T_OFFSET);
			}
			// ----------------------
			// GoPro telemetry
			// ----------------------
			else if (colName == CSVPARSER_GP_ACCL_X.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_ACCL_X);
				bitset_set_bit(gpAvail, gpo::GOPRO_AVAIL_ACCL);
			}
			else if (colName == CSVPARSER_GP_ACCL_Y.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_ACCL_Y);
				bitset_set_bit(gpAvail, gpo::GOPRO_AVAIL_ACCL);
			}
			else if (colName == CSVPARSER_GP_ACCL_Z.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_ACCL_Z);
				bitset_set_bit(gpAvail, gpo::GOPRO_AVAIL_ACCL);
			}
			else if (colName == CSVPARSER_GP_GYRO_X.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GYRO_X);
				bitset_set_bit(gpAvail, gpo::GOPRO_AVAIL_GYRO);
			}
			else if (colName == CSVPARSER_GP_GYRO_Y.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GYRO_Y);
				bitset_set_bit(gpAvail, gpo::GOPRO_AVAIL_GYRO);
			}
			else if (colName == CSVPARSER_GP_GYRO_Z.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GYRO_Z);
				bitset_set_bit(gpAvail, gpo::GOPRO_AVAIL_GYRO);
			}
			else if (colName == CSVPARSER_GP_GRAV_X.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GRAV_X);
				bitset_set_bit(gpAvail, gpo::GOPRO_AVAIL_GRAV);
			}
			else if (colName == CSVPARSER_GP_GRAV_Y.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GRAV_Y);
				bitset_set_bit(gpAvail, gpo::GOPRO_AVAIL_GRAV);
			}
			else if (colName == CSVPARSER_GP_GRAV_Z.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GRAV_Z);
				bitset_set_bit(gpAvail, gpo::GOPRO_AVAIL_GRAV);
			}
			else if (colName == CSVPARSER_GP_CORI_W.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_CORI_W);
				bitset_set_bit(gpAvail, gpo::GOPRO_AVAIL_CORI);
			}
			else if (colName == CSVPARSER_GP_CORI_X.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_CORI_X);
				bitset_set_bit(gpAvail, gpo::GOPRO_AVAIL_CORI);
			}
			else if (colName == CSVPARSER_GP_CORI_Y.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_CORI_Y);
				bitset_set_bit(gpAvail, gpo::GOPRO_AVAIL_CORI);
			}
			else if (colName == CSVPARSER_GP_CORI_Z.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_CORI_Z);
				bitset_set_bit(gpAvail, gpo::GOPRO_AVAIL_CORI);
			}
			else if (colName == CSVPARSER_GP_GPS_LAT.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GPS_LAT);
				bitset_set_bit(gpAvail, gpo::GOPRO_AVAIL_GPS_LATLON);
			}
			else if (colName == CSVPARSER_GP_GPS_LON.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GPS_LON);
				bitset_set_bit(gpAvail, gpo::GOPRO_AVAIL_GPS_LATLON);
			}
			else if (colName == CSVPARSER_GP_GPS_ALTITUDE.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GPS_ALTITUDE);
				bitset_set_bit(gpAvail, gpo::GOPRO_AVAIL_GPS_ALTITUDE);
			}
			else if (colName == CSVPARSER_GP_GPS_SPEED2D.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GPS_SPEED2D);
				bitset_set_bit(gpAvail, gpo::GOPRO_AVAIL_GPS_SPEED2D);
			}
			else if (colName == CSVPARSER_GP_GPS_SPEED3D.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GPS_SPEED3D);
				bitset_set_bit(gpAvail, gpo::GOPRO_AVAIL_GPS_SPEED3D);
			}
			// ----------------------
			// ECU telemetry
			// ----------------------
			else if (colName == CSVPARSER_ECU_ENGINE_SPEED.columnTitle)
			{
				columns.push_back(CSVPARSER_ECU_ENGINE_SPEED);
				bitset_set_bit(ecuAvail, gpo::ECU_AVAIL_ENGINE_SPEED);
			}
			else if (colName == CSVPARSER_ECU_TPS.columnTitle)
			{
				columns.push_back(CSVPARSER_ECU_TPS);
				bitset_set_bit(ecuAvail, gpo::ECU_AVAIL_TPS);
			}
			else if (colName == CSVPARSER_ECU_BOOST.columnTitle)
			{
				columns.push_back(CSVPARSER_ECU_BOOST);
				bitset_set_bit(ecuAvail, gpo::ECU_AVAIL_BOOST);
			}
			// ----------------------
			// TrackData telemetry
			// ----------------------
			else if (colName == CSVPARSER_TRACK_ON_TRACK_LAT.columnTitle)
			{
				columns.push_back(CSVPARSER_TRACK_ON_TRACK_LAT);
				bitset_set_bit(trackAvail, gpo::TRACK_AVAIL_ON_TRACK_LATLON);
			}
			else if (colName == CSVPARSER_TRACK_ON_TRACK_LON.columnTitle)
			{
				columns.push_back(CSVPARSER_TRACK_ON_TRACK_LON);
				bitset_set_bit(trackAvail, gpo::TRACK_AVAIL_ON_TRACK_LATLON);
			}
			else if (colName == CSVPARSER_TRACK_LAP.columnTitle)
			{
				columns.push_back(CSVPARSER_TRACK_LAP);
				bitset_set_bit(trackAvail, gpo::TRACK_AVAIL_LAP);
			}
			else if (colName == CSVPARSER_TRACK_LAP_TIME_OFFSET.columnTitle)
			{
				columns.push_back(CSVPARSER_TRACK_LAP_TIME_OFFSET);
				bitset_set_bit(trackAvail, gpo::TRACK_AVAIL_LAP_TIME_OFFSET);
			}
			else if (colName == CSVPARSER_TRACK_SECTOR.columnTitle)
			{
				columns.push_back(CSVPARSER_TRACK_SECTOR);
				bitset_set_bit(trackAvail, gpo::TRACK_AVAIL_SECTOR);
			}
			else if (colName == CSVPARSER_TRACK_SECTOR_TIME_OFFSET.columnTitle)
			{
				columns.push_back(CSVPARSER_TRACK_SECTOR_TIME_OFFSET);
				bitset_set_bit(trackAvail, gpo::TRACK_AVAIL_SECTOR_TIME_OFFSET);
			}
			else
			{
				throw std::runtime_error("unexpected CSV column name '" + colName + "'");
			}
		}

		tSamps->clear();
		tSamps->reserve(4096);// prealloc memory
		gpo::TelemetrySample samp;
		for (auto rowItr=reader.begin(); rowItr!=reader.end(); rowItr++)
		{
			size_t colIdx = 0;
			for (const auto &column : columns)
			{
				column.toStruct(*rowItr, colIdx++, &samp);
			}

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