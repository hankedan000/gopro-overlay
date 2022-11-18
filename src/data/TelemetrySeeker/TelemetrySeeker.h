#pragma once

#include <memory>

#include "TelemetrySample.h"

namespace gpo
{
	class TelemetrySeeker
	{
	public:
		TelemetrySeeker(
			TelemetrySamplesPtr samples);

		void
		next();

		void
		seekToIdx(
			size_t idx);
	
		void
		seekToTime(
			double timeOffset);
		
		void
		seekToLap(
			size_t lap);
		
		size_t
		seekedIdx();

		size_t
		size() const;

	private:
		TelemetrySamplesPtr samples_;
		size_t seekedIdx_;

	};

	using TelemetrySeekerPtr = std::shared_ptr<TelemetrySeeker>;
}