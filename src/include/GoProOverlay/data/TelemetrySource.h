#pragma once

#include <memory>

#include "TelemetrySample.h"
#include "TelemetrySeeker.h"

namespace gpo
{
	class TelemetrySource
	{
	public:
		TelemetrySource(
			TelemetrySamplesPtr samples,
			TelemetrySeekerPtr seeker);

		const TelemetrySample &
		at(
			size_t idx) const;

		size_t
		seekedIdx() const;

		size_t
		size() const;

	private:
		TelemetrySamplesPtr samples_;
		TelemetrySeekerPtr seeker_;

	};

	using TelemetrySourcePtr = std::shared_ptr<TelemetrySource>;
}