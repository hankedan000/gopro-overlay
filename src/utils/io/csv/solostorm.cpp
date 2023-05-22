#include "GoProOverlay/utils/io/csv.h"

#include <fstream>
#include <spdlog/spdlog.h>

namespace utils
{
namespace io
{

	static constexpr CSV_ColumnParser CSVPARSER_SSTORM_T_OFFSET = MAKE_PARSER(gpo::TelemetrySample, t_offset, "MATH_ELAPSED_TIME");

	static constexpr CSV_ColumnParser CSVPARSER_SSTORM_ACCL_X = MAKE_PARSER(gpo::TelemetrySample, gpSamp.accl.x, "ACCEL_X (m/s^2)");
	static constexpr CSV_ColumnParser CSVPARSER_SSTORM_ACCL_Y = MAKE_PARSER(gpo::TelemetrySample, gpSamp.accl.y, "ACCEL_Y (m/s^2)");
	static constexpr CSV_ColumnParser CSVPARSER_SSTORM_ACCL_Z = MAKE_PARSER(gpo::TelemetrySample, gpSamp.accl.z, "ACCEL_Z (m/s^2)");
	static constexpr CSV_ColumnParser CSVPARSER_SSTORM_GPS_LAT = MAKE_PARSER(gpo::TelemetrySample, gpSamp.gps.coord.lat, "LAT (deg)");
	static constexpr CSV_ColumnParser CSVPARSER_SSTORM_GPS_LON = MAKE_PARSER(gpo::TelemetrySample, gpSamp.gps.coord.lon, "LONG (deg)");
	static constexpr CSV_ColumnParser CSVPARSER_SSTORM_GPS_ALTITUDE = MAKE_PARSER(gpo::TelemetrySample, gpSamp.gps.altitude, "ELEV (m)");
	static constexpr CSV_ColumnParser CSVPARSER_SSTORM_GPS_SPEED2D = MAKE_PARSER(gpo::TelemetrySample, gpSamp.gps.speed2D, "SPEED (m/s)");

	static constexpr CSV_ColumnParser CSVPARSER_SSTORM_ECU_ENGINE_SPEED = MAKE_PARSER(gpo::TelemetrySample, ecuSamp.engineSpeed_rpm, "RPM");
	static constexpr CSV_ColumnParser CSVPARSER_SSTORM_ECU_TPS = MAKE_PARSER(gpo::TelemetrySample, ecuSamp.tps, "THROTTLE (%)");

	static constexpr CSV_ColumnParser CSVPARSER_SSTORM_CALC_ON_TRACK_LAT = MAKE_PARSER(gpo::TelemetrySample, calcSamp.onTrackLL.lat, "MATH_GRID_X");
	static constexpr CSV_ColumnParser CSVPARSER_SSTORM_CALC_ON_TRACK_LON = MAKE_PARSER(gpo::TelemetrySample, calcSamp.onTrackLL.lon, "MATH_GRID_Y");
	static constexpr CSV_ColumnParser CSVPARSER_SSTORM_CALC_LAP = MAKE_PARSER(gpo::TelemetrySample, calcSamp.lap, "MATH_LAP_NUMBER");
	static constexpr CSV_ColumnParser CSVPARSER_SSTORM_CALC_LAP_TIME_OFFSET = MAKE_PARSER(gpo::TelemetrySample, calcSamp.lapTimeOffset, "MATH_LAP_ELAPSED_TIME");
	static constexpr CSV_ColumnParser CSVPARSER_SSTORM_CALC_SECTOR = MAKE_PARSER(gpo::TelemetrySample, calcSamp.sector, "MATH_SECTOR_NUMBER");
	static constexpr CSV_ColumnParser CSVPARSER_SSTORM_CALC_SECTOR_TIME_OFFSET = MAKE_PARSER(gpo::TelemetrySample, calcSamp.sectorTimeOffset, "MATH_SECTOR_ELAPSED_TIME");

	bool
    readTelemetryFromSoloStormCSV(
        const std::filesystem::path &csvFilepath,
        gpo::TelemetrySamplesPtr tSamps,
		gpo::DataAvailableBitSet &avail)
	{
		std::ifstream ifs(csvFilepath);
		csv::CSVReader reader(ifs, csv::CSVFormat());

		bitset_clear(avail);

		auto colNames = reader.get_col_names();
		std::vector<CSV_ColumnParser> columns;
		columns.reserve(100);
		for (const auto &colName : colNames)
		{
			if (colName == CSVPARSER_SSTORM_T_OFFSET.columnTitle)
			{
				columns.push_back(CSVPARSER_SSTORM_T_OFFSET);
			}
			// ----------------------
			// GoPro telemetry
			// ----------------------
			else if (colName == CSVPARSER_SSTORM_ACCL_X.columnTitle)
			{
				columns.push_back(CSVPARSER_SSTORM_ACCL_X);
				bitset_set_bit(avail, gpo::eDA_GOPRO_ACCL);
			}
			else if (colName == CSVPARSER_SSTORM_ACCL_Y.columnTitle)
			{
				columns.push_back(CSVPARSER_SSTORM_ACCL_Y);
				bitset_set_bit(avail, gpo::eDA_GOPRO_ACCL);
			}
			else if (colName == CSVPARSER_SSTORM_ACCL_Z.columnTitle)
			{
				columns.push_back(CSVPARSER_SSTORM_ACCL_Z);
				bitset_set_bit(avail, gpo::eDA_GOPRO_ACCL);
			}
			else if (colName == CSVPARSER_SSTORM_GPS_LAT.columnTitle)
			{
				columns.push_back(CSVPARSER_SSTORM_GPS_LAT);
				bitset_set_bit(avail, gpo::eDA_GOPRO_GPS_LATLON);
			}
			else if (colName == CSVPARSER_SSTORM_GPS_LON.columnTitle)
			{
				columns.push_back(CSVPARSER_SSTORM_GPS_LON);
				bitset_set_bit(avail, gpo::eDA_GOPRO_GPS_LATLON);
			}
			else if (colName == CSVPARSER_SSTORM_GPS_ALTITUDE.columnTitle)
			{
				columns.push_back(CSVPARSER_SSTORM_GPS_ALTITUDE);
				bitset_set_bit(avail, gpo::eDA_GOPRO_GPS_ALTITUDE);
			}
			else if (colName == CSVPARSER_SSTORM_GPS_SPEED2D.columnTitle)
			{
				columns.push_back(CSVPARSER_SSTORM_GPS_SPEED2D);
				bitset_set_bit(avail, gpo::eDA_GOPRO_GPS_SPEED2D);
			}
			// ----------------------
			// ECU telemetry
			// ----------------------
			else if (colName == CSVPARSER_SSTORM_ECU_ENGINE_SPEED.columnTitle)
			{
				columns.push_back(CSVPARSER_SSTORM_ECU_ENGINE_SPEED);
				bitset_set_bit(avail, gpo::eDA_ECU_ENGINE_SPEED);
			}
			else if (colName == CSVPARSER_SSTORM_ECU_TPS.columnTitle)
			{
				columns.push_back(CSVPARSER_SSTORM_ECU_TPS);
				bitset_set_bit(avail, gpo::eDA_ECU_TPS);
			}
			// ----------------------
			// TrackData telemetry
			// ----------------------
			else if (colName == CSVPARSER_SSTORM_CALC_ON_TRACK_LAT.columnTitle)
			{
				columns.push_back(CSVPARSER_SSTORM_CALC_ON_TRACK_LAT);
				bitset_set_bit(avail, gpo::eDA_CALC_ON_TRACK_LATLON);
			}
			else if (colName == CSVPARSER_SSTORM_CALC_ON_TRACK_LON.columnTitle)
			{
				columns.push_back(CSVPARSER_SSTORM_CALC_ON_TRACK_LON);
				bitset_set_bit(avail, gpo::eDA_CALC_ON_TRACK_LATLON);
			}
			else if (colName == CSVPARSER_SSTORM_CALC_LAP.columnTitle)
			{
				columns.push_back(CSVPARSER_SSTORM_CALC_LAP);
				bitset_set_bit(avail, gpo::eDA_CALC_LAP);
			}
			else if (colName == CSVPARSER_SSTORM_CALC_LAP_TIME_OFFSET.columnTitle)
			{
				columns.push_back(CSVPARSER_SSTORM_CALC_LAP_TIME_OFFSET);
				bitset_set_bit(avail, gpo::eDA_CALC_LAP_TIME_OFFSET);
			}
			else if (colName == CSVPARSER_SSTORM_CALC_SECTOR.columnTitle)
			{
				columns.push_back(CSVPARSER_SSTORM_CALC_SECTOR);
				bitset_set_bit(avail, gpo::eDA_CALC_SECTOR);
			}
			else if (colName == CSVPARSER_SSTORM_CALC_SECTOR_TIME_OFFSET.columnTitle)
			{
				columns.push_back(CSVPARSER_SSTORM_CALC_SECTOR_TIME_OFFSET);
				bitset_set_bit(avail, gpo::eDA_CALC_SECTOR_TIME_OFFSET);
			}
			else
			{
				columns.push_back(CSVPARSER_IGNORED);
				// throw std::runtime_error("unexpected CSV column name '" + colName + "'");
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
				if (column.toStruct)
				{
					column.toStruct(*rowItr, colIdx, &samp);
				}
				colIdx++;
			}

			// convert units
			samp.t_offset /= 1000.0;// ms -> sec

			tSamps->push_back(samp);
		}

		return true;
	}

}
}