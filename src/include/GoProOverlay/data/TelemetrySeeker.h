#pragma once

#include <memory>
#include <unordered_map>

#include "TelemetrySample.h"

namespace gpo
{
	// forward declaration
	class DataSource;
	using DataSourcePtr = std::shared_ptr<DataSource>;

	class TelemetrySeeker
	{
	public:
		explicit
		TelemetrySeeker(
			DataSourcePtr dSrc);

		std::string
		getDataSourceName() const;

		void
		prev();

		void
		next();

		bool
		hasPrev() const;

		bool
		hasNext() const;

		void
		seekToIdx(
			size_t idx);

		void
		setAlignmentIdx(
			size_t idx);

		size_t
		getAlignmentIdx() const;

		void
		seekRelative(
			size_t amount,
			bool forward);

		void
		seekRelativeTime(
			double offset_secs,
			double quanta_secs = 0.0001);
	
		void
		seekToLapEntry(
			unsigned int lap);
		
		void
		seekToLapExit(
			unsigned int lap);
		
		size_t
		seekedIdx() const;

		size_t
		size() const;

		unsigned int
		lapCount() const;

		double
		rateHz() const;

		double
		getTimeAt(
			size_t idx) const;

		std::pair<size_t, size_t>
		getLapEntryExit(
			unsigned int lap) const;

		// forces seeker to analyze samples and find lap/sector seek points again
		void
		analyze();

	private:
		std::weak_ptr<DataSource> dataSrc_;
		size_t seekedIdx_;
		size_t alignmentIdx_;
		double rate_hz_;

		struct LapIndices
		{
			size_t entryIdx = -1;
			size_t exitIdx = -1;
		};
		std::unordered_map<unsigned int, LapIndices> lapIndicesMap_;

	};

	using TelemetrySeekerPtr = std::shared_ptr<TelemetrySeeker>;
}