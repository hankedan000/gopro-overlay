#pragma once

#include <memory>
#include <unordered_map>

#include "TelemetrySample.h"

namespace gpo
{
	class TelemetrySeeker
	{
	public:
		TelemetrySeeker(
			TelemetrySamplesPtr samples);

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
		seekToTime(
			double timeOffset);
		
		void
		seekToLapEntry(
			unsigned int lap);
		
		void
		seekToLapExit(
			unsigned int lap);
		
		size_t
		seekedIdx();

		size_t
		size() const;

		unsigned int
		lapCount() const;

		std::pair<size_t, size_t>
		getLapEntryExit(
			unsigned int lap) const;

		// forces seeker to analyze samples and find lap/sector seek points again
		void
		analyze();

	private:
		TelemetrySamplesPtr samples_;
		size_t seekedIdx_;

		struct LapIndices
		{
			LapIndices()
			 : entryIdx(-1)
			 , exitIdx(-1)
			{}

			size_t entryIdx;
			size_t exitIdx;
		};
		std::unordered_map<unsigned int, LapIndices> lapIndicesMap_;

	};

	using TelemetrySeekerPtr = std::shared_ptr<TelemetrySeeker>;
}