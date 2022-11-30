#include "GoProOverlay/data/TelemetrySeeker.h"

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
	TelemetrySeeker::next()
	{
		auto nextIdx = seekedIdx_ + 1;
		if (nextIdx < size())
		{
			seekedIdx_ = nextIdx;
		}
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
	
	std::pair<bool,size_t>
	TelemetrySeeker::seekToLap(
		unsigned int lap)
	{
		for (size_t i=0; i<size(); i++)
		{
			const auto &samp = samples_->at(i);
			if (samp.lap == lap)
			{
				seekedIdx_ = i;
				return {true,seekedIdx_};
			}
		}
		return {false,seekedIdx_};
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