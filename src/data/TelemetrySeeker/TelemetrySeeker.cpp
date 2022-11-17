#include "TelemetrySeeker.h"

#include <stdexcept>

namespace gpo
{
	TelemetrySeeker::TelemetrySeeker(
		TelemetrySamplesPtr samples)
	 : samples_(samples)
	 , seekedIdx_(0)
	{
	}

	void
	TelemetrySeeker::seekToIdx(
		size_t idx)
	{
		if (idx >= size())
		{
			throw std::out_of_range("idx " + std::to_string(idx) + " is > size of " + std::to_string(size()));
		}
		seekedIdx_ = idx;
	}

	void
	TelemetrySeeker::seekToTime(
		double timeOffset)
	{
		throw std::runtime_error("seekToTime() is not implemented");
	}
	
	void
	TelemetrySeeker::seekToLap(
		size_t lap)
	{
		throw std::runtime_error("seekToLap() is not implemented");
	}
	
	size_t
	TelemetrySeeker::seekedIdx()
	{
		return seekedIdx_;
	}

	size_t
	TelemetrySeeker::size() const
	{
		return samples_->size();
	}
}