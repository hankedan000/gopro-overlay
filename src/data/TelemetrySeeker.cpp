#include "GoProOverlay/data/TelemetrySeeker.h"

#include <stdexcept>

#include "GoProOverlay/data/DataSource.h"

namespace gpo
{
	TelemetrySeeker::TelemetrySeeker(
			DataSourcePtr dSrc)
	 : dataSrc_(dSrc)
	 , seekedIdx_(0)
	 , lapIndicesMap_()
	{
	}

	std::string
	TelemetrySeeker::getDataSourceName() const
	{
		return dataSrc_->getSourceName();
	}

	void
	TelemetrySeeker::prev()
	{
		if (hasPrev())
		{
			seekedIdx_--;
		}
	}

	void
	TelemetrySeeker::next()
	{
		if (hasNext())
		{
			seekedIdx_++;
		}
	}

	bool
	TelemetrySeeker::hasPrev() const
	{
		return seekedIdx_ != 0;
	}

	bool
	TelemetrySeeker::hasNext() const
	{
		return (seekedIdx_ + 1) < size();
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
	TelemetrySeeker::seekRelative(
		size_t amount,
		bool forward)
	{
		if (forward)
		{
			if ((amount + seekedIdx_) < size())
			{
				seekedIdx_ += amount;
			}
			else
			{
				seekedIdx_ = size() - 1;
			}
		}
		else
		{
			if (amount > seekedIdx_)
			{
				seekedIdx_ = 0;
			}
			else
			{
				seekedIdx_ -= amount;
			}
		}
	}

	void
	TelemetrySeeker::seekRelativeTime(
		double offset_secs)
	{
		auto currTimeOffset = getTimeAt(seekedIdx_);
		const auto targetTimeOffset = currTimeOffset + offset_secs;
		const auto maxTimeOffset = getTimeAt(size() - 1);
		if (offset_secs > 0.0)
		{
			if (targetTimeOffset < maxTimeOffset)
			{
				while (currTimeOffset < targetTimeOffset)
				{
					currTimeOffset = getTimeAt(++seekedIdx_);
				}
			}
			else
			{
				seekedIdx_ = size() - 1;
			}
		}
		else
		{
			if (targetTimeOffset <= 0.0)
			{
				seekedIdx_ = 0;
			}
			else
			{
				while (currTimeOffset > targetTimeOffset)
				{
					currTimeOffset = getTimeAt(--seekedIdx_);
				}
			}
		}
	}

	void
	TelemetrySeeker::seekToTime(
		double timeOffset)
	{
		throw std::runtime_error("seekToTime() is not implemented");
	}
	
	void
	TelemetrySeeker::seekToLapEntry(
		unsigned int lap)
	{
		seekedIdx_ = lapIndicesMap_.at(lap).entryIdx;
	}
	
	void
	TelemetrySeeker::seekToLapExit(
		unsigned int lap)
	{
		seekedIdx_ = lapIndicesMap_.at(lap).exitIdx;
	}
	
	size_t
	TelemetrySeeker::seekedIdx()
	{
		return seekedIdx_;
	}

	size_t
	TelemetrySeeker::size() const
	{
		return dataSrc_->samples_->size();
	}

	unsigned int
	TelemetrySeeker::lapCount() const
	{
		return lapIndicesMap_.size();
	}

	double
	TelemetrySeeker::getTimeAt(
		size_t idx) const
	{
		return dataSrc_->telemSrc->at(idx).gpSamp.t_offset;
	}

	std::pair<size_t, size_t>
	TelemetrySeeker::getLapEntryExit(
		unsigned int lap) const
	{
		auto &li = lapIndicesMap_.at(lap);
		return {li.entryIdx,li.exitIdx};
	}

	// forces seeker to analyze samples and find lap/sector seek points again
	void
	TelemetrySeeker::analyze()
	{
		lapIndicesMap_.clear();

		LapIndices li;
		int prevLap = -1;
		int prevSampLap = -1;
		int lapWereIn = -1;
		bool inLap = false;
		for (size_t i=0; i<size(); i++)
		{
			const auto &samp = dataSrc_->samples_->at(i);
			if (prevSampLap == -1 && samp.lap > 0)
			{
				// entered a lap
				lapWereIn = samp.lap;
				li.entryIdx = i;
				li.exitIdx = -1;
			}
			else if (lapWereIn != -1 && prevSampLap != samp.lap)
			{
				// exited a lap
				li.exitIdx = i - 1;
				lapIndicesMap_.insert({lapWereIn,li});

				prevLap = lapWereIn;
				lapWereIn = samp.lap;
				li.entryIdx = i;// circuit case where finishGate == startGate
			}
			prevSampLap = samp.lap;
		}

		// corner case where we never left a lap (could have pitted in early or something)
		if (lapWereIn != -1)
		{
			li.exitIdx = size() - 1;
			lapIndicesMap_.insert({lapWereIn,li});
		}
	}
}