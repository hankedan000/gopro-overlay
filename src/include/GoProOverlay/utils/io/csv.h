#pragma once

#include <csv.hpp>
#include <filesystem>
#include <tuple>
#include <ostream>

#include "GoProOverlay/data/TelemetrySample.h"

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
		auto cellValue = row[colIdx].get<double>();
		*reinterpret_cast<MEMBER_T *>(reinterpret_cast<uint8_t*>(structOut) + FIELD_OFFSET) = cellValue;
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

	static constexpr CSV_ColumnParser CSVPARSER_IGNORED = {"", nullptr, nullptr};

	#define CSV_ROW_ITR_GET(OUT_VAR,ROW_ITR,COL_IDX) OUT_VAR = (*ROW_ITR)[COL_IDX].get<std::remove_reference<decltype(OUT_VAR)>::type>()

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
    
	bool
    readTelemetryFromSoloStormCSV(
        const std::filesystem::path &csvFilepath,
        gpo::TelemetrySamplesPtr tSamps,
		gpo::DataAvailableBitSet &avail);
}
}