#include "GoProOverlay/data/TelemetrySource.h"

#include <stdexcept>

namespace gpo
{
	TelemetrySource::TelemetrySource(
		TelemetrySamplesPtr samples,
		TelemetrySeekerPtr seeker)
	 : samples_(samples)
	 , seeker_(seeker)
	{
	}

	const TelemetrySample &
	TelemetrySource::at(
		size_t idx) const
	{
		return samples_->at(idx);
	}

	size_t
	TelemetrySource::seekedIdx() const
	{
		return seeker_->seekedIdx();
	}

	size_t
	TelemetrySource::size() const
	{
		return samples_->size();
	}
}