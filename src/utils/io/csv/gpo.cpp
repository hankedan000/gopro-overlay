#include "GoProOverlay/utils/io/csv.h"

#include <fstream>
#include <spdlog/spdlog.h>

namespace utils
{
namespace io
{

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

	static constexpr CSV_ColumnParser CSVPARSER_CALC_ON_TRACK_LAT = MAKE_PARSER(gpo::TelemetrySample, calcSamp.onTrackLL.lat, "onTrackLL_lat");
	static constexpr CSV_ColumnParser CSVPARSER_CALC_ON_TRACK_LON = MAKE_PARSER(gpo::TelemetrySample, calcSamp.onTrackLL.lon, "onTrackLL_lon");
	static constexpr CSV_ColumnParser CSVPARSER_CALC_LAP = MAKE_PARSER(gpo::TelemetrySample, calcSamp.lap, "lap");
	static constexpr CSV_ColumnParser CSVPARSER_CALC_LAP_TIME_OFFSET = MAKE_PARSER(gpo::TelemetrySample, calcSamp.lapTimeOffset, "lapTimeOffset");
	static constexpr CSV_ColumnParser CSVPARSER_CALC_SECTOR = MAKE_PARSER(gpo::TelemetrySample, calcSamp.sector, "sector");
	static constexpr CSV_ColumnParser CSVPARSER_CALC_SECTOR_TIME_OFFSET = MAKE_PARSER(gpo::TelemetrySample, calcSamp.sectorTimeOffset, "sectorTimeOffset");

	bool
    writeTelemetryToCSV(
        const gpo::TelemetrySamplesPtr &tSamps,
        std::ostream &out,
		const gpo::DataAvailableBitSet &avail)
	{
		auto writer = csv::make_csv_writer(out);

		std::vector<CSV_ColumnParser> columns;
		columns.reserve(100);

		columns.push_back(CSVPARSER_T_OFFSET);

		// -----------------------------
		// GoPro telemetery
		// -----------------------------
		if (bitset_is_set(avail, gpo::eDA_GOPRO_ACCL))
		{
			columns.push_back(CSVPARSER_GP_ACCL_X);
			columns.push_back(CSVPARSER_GP_ACCL_Y);
			columns.push_back(CSVPARSER_GP_ACCL_Z);
		}
		if (bitset_is_set(avail, gpo::eDA_GOPRO_GYRO))
		{
			columns.push_back(CSVPARSER_GP_GYRO_X);
			columns.push_back(CSVPARSER_GP_GYRO_Y);
			columns.push_back(CSVPARSER_GP_GYRO_Z);
		}
		if (bitset_is_set(avail, gpo::eDA_GOPRO_GRAV))
		{
			columns.push_back(CSVPARSER_GP_GRAV_X);
			columns.push_back(CSVPARSER_GP_GRAV_Y);
			columns.push_back(CSVPARSER_GP_GRAV_Z);
		}
		if (bitset_is_set(avail, gpo::eDA_GOPRO_CORI))
		{
			columns.push_back(CSVPARSER_GP_CORI_W);
			columns.push_back(CSVPARSER_GP_CORI_X);
			columns.push_back(CSVPARSER_GP_CORI_Y);
			columns.push_back(CSVPARSER_GP_CORI_Z);
		}
		if (bitset_is_set(avail, gpo::eDA_GOPRO_GPS_LATLON))
		{
			columns.push_back(CSVPARSER_GP_GPS_LAT);
			columns.push_back(CSVPARSER_GP_GPS_LON);
		}
		if (bitset_is_set(avail, gpo::eDA_GOPRO_GPS_ALTITUDE))
		{
			columns.push_back(CSVPARSER_GP_GPS_ALTITUDE);
		}
		if (bitset_is_set(avail, gpo::eDA_GOPRO_GPS_SPEED2D))
		{
			columns.push_back(CSVPARSER_GP_GPS_SPEED2D);
		}
		if (bitset_is_set(avail, gpo::eDA_GOPRO_GPS_SPEED3D))
		{
			columns.push_back(CSVPARSER_GP_GPS_SPEED3D);
		}

		// -----------------------------
		// ECU telemetery
		// -----------------------------
		if (bitset_is_set(avail, gpo::eDA_ECU_ENGINE_SPEED))
		{
			columns.push_back(CSVPARSER_ECU_ENGINE_SPEED);
		}
		if (bitset_is_set(avail, gpo::eDA_ECU_TPS))
		{
			columns.push_back(CSVPARSER_ECU_TPS);
		}
		if (bitset_is_set(avail, gpo::eDA_ECU_BOOST))
		{
			columns.push_back(CSVPARSER_ECU_BOOST);
		}

		// -----------------------------
		// Processed telemetery
		// -----------------------------
		if (bitset_is_set(avail, gpo::eDA_CALC_ON_TRACK_LATLON))
		{
			columns.push_back(CSVPARSER_CALC_ON_TRACK_LAT);
			columns.push_back(CSVPARSER_CALC_ON_TRACK_LON);
		}
		if (bitset_is_set(avail, gpo::eDA_CALC_LAP))
		{
			columns.push_back(CSVPARSER_CALC_LAP);
		}
		if (bitset_is_set(avail, gpo::eDA_CALC_LAP_TIME_OFFSET))
		{
			columns.push_back(CSVPARSER_CALC_LAP_TIME_OFFSET);
		}
		if (bitset_is_set(avail, gpo::eDA_CALC_SECTOR))
		{
			columns.push_back(CSVPARSER_CALC_SECTOR);
		}
		if (bitset_is_set(avail, gpo::eDA_CALC_SECTOR_TIME_OFFSET))
		{
			columns.push_back(CSVPARSER_CALC_SECTOR_TIME_OFFSET);
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
		const gpo::DataAvailableBitSet &avail)
	{
		// open file for writing and overwrite any existing contents
		std::ofstream fs(csvFilepath,std::ios_base::trunc);
		return writeTelemetryToCSV(tSamps,fs,avail);
	}

	bool
    readTelemetryFromCSV(
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
				bitset_set_bit(avail, gpo::eDA_GOPRO_ACCL);
			}
			else if (colName == CSVPARSER_GP_ACCL_Y.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_ACCL_Y);
				bitset_set_bit(avail, gpo::eDA_GOPRO_ACCL);
			}
			else if (colName == CSVPARSER_GP_ACCL_Z.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_ACCL_Z);
				bitset_set_bit(avail, gpo::eDA_GOPRO_ACCL);
			}
			else if (colName == CSVPARSER_GP_GYRO_X.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GYRO_X);
				bitset_set_bit(avail, gpo::eDA_GOPRO_GYRO);
			}
			else if (colName == CSVPARSER_GP_GYRO_Y.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GYRO_Y);
				bitset_set_bit(avail, gpo::eDA_GOPRO_GYRO);
			}
			else if (colName == CSVPARSER_GP_GYRO_Z.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GYRO_Z);
				bitset_set_bit(avail, gpo::eDA_GOPRO_GYRO);
			}
			else if (colName == CSVPARSER_GP_GRAV_X.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GRAV_X);
				bitset_set_bit(avail, gpo::eDA_GOPRO_GRAV);
			}
			else if (colName == CSVPARSER_GP_GRAV_Y.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GRAV_Y);
				bitset_set_bit(avail, gpo::eDA_GOPRO_GRAV);
			}
			else if (colName == CSVPARSER_GP_GRAV_Z.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GRAV_Z);
				bitset_set_bit(avail, gpo::eDA_GOPRO_GRAV);
			}
			else if (colName == CSVPARSER_GP_CORI_W.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_CORI_W);
				bitset_set_bit(avail, gpo::eDA_GOPRO_CORI);
			}
			else if (colName == CSVPARSER_GP_CORI_X.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_CORI_X);
				bitset_set_bit(avail, gpo::eDA_GOPRO_CORI);
			}
			else if (colName == CSVPARSER_GP_CORI_Y.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_CORI_Y);
				bitset_set_bit(avail, gpo::eDA_GOPRO_CORI);
			}
			else if (colName == CSVPARSER_GP_CORI_Z.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_CORI_Z);
				bitset_set_bit(avail, gpo::eDA_GOPRO_CORI);
			}
			else if (colName == CSVPARSER_GP_GPS_LAT.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GPS_LAT);
				bitset_set_bit(avail, gpo::eDA_GOPRO_GPS_LATLON);
			}
			else if (colName == CSVPARSER_GP_GPS_LON.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GPS_LON);
				bitset_set_bit(avail, gpo::eDA_GOPRO_GPS_LATLON);
			}
			else if (colName == CSVPARSER_GP_GPS_ALTITUDE.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GPS_ALTITUDE);
				bitset_set_bit(avail, gpo::eDA_GOPRO_GPS_ALTITUDE);
			}
			else if (colName == CSVPARSER_GP_GPS_SPEED2D.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GPS_SPEED2D);
				bitset_set_bit(avail, gpo::eDA_GOPRO_GPS_SPEED2D);
			}
			else if (colName == CSVPARSER_GP_GPS_SPEED3D.columnTitle)
			{
				columns.push_back(CSVPARSER_GP_GPS_SPEED3D);
				bitset_set_bit(avail, gpo::eDA_GOPRO_GPS_SPEED3D);
			}
			// ----------------------
			// ECU telemetry
			// ----------------------
			else if (colName == CSVPARSER_ECU_ENGINE_SPEED.columnTitle)
			{
				columns.push_back(CSVPARSER_ECU_ENGINE_SPEED);
				bitset_set_bit(avail, gpo::eDA_ECU_ENGINE_SPEED);
			}
			else if (colName == CSVPARSER_ECU_TPS.columnTitle)
			{
				columns.push_back(CSVPARSER_ECU_TPS);
				bitset_set_bit(avail, gpo::eDA_ECU_TPS);
			}
			else if (colName == CSVPARSER_ECU_BOOST.columnTitle)
			{
				columns.push_back(CSVPARSER_ECU_BOOST);
				bitset_set_bit(avail, gpo::eDA_ECU_BOOST);
			}
			// ----------------------
			// TrackData telemetry
			// ----------------------
			else if (colName == CSVPARSER_CALC_ON_TRACK_LAT.columnTitle)
			{
				columns.push_back(CSVPARSER_CALC_ON_TRACK_LAT);
				bitset_set_bit(avail, gpo::eDA_CALC_ON_TRACK_LATLON);
			}
			else if (colName == CSVPARSER_CALC_ON_TRACK_LON.columnTitle)
			{
				columns.push_back(CSVPARSER_CALC_ON_TRACK_LON);
				bitset_set_bit(avail, gpo::eDA_CALC_ON_TRACK_LATLON);
			}
			else if (colName == CSVPARSER_CALC_LAP.columnTitle)
			{
				columns.push_back(CSVPARSER_CALC_LAP);
				bitset_set_bit(avail, gpo::eDA_CALC_LAP);
			}
			else if (colName == CSVPARSER_CALC_LAP_TIME_OFFSET.columnTitle)
			{
				columns.push_back(CSVPARSER_CALC_LAP_TIME_OFFSET);
				bitset_set_bit(avail, gpo::eDA_CALC_LAP_TIME_OFFSET);
			}
			else if (colName == CSVPARSER_CALC_SECTOR.columnTitle)
			{
				columns.push_back(CSVPARSER_CALC_SECTOR);
				bitset_set_bit(avail, gpo::eDA_CALC_SECTOR);
			}
			else if (colName == CSVPARSER_CALC_SECTOR_TIME_OFFSET.columnTitle)
			{
				columns.push_back(CSVPARSER_CALC_SECTOR_TIME_OFFSET);
				bitset_set_bit(avail, gpo::eDA_CALC_SECTOR_TIME_OFFSET);
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

}
}