#include "GoProOverlay/data/TelemetrySeeker.h"

#include <stdexcept>

#include "GoProOverlay/data/DataSource.h"

namespace gpo
{
	TelemetrySeeker::TelemetrySeeker(
			DataSourcePtr dSrc)
	 : dataSrc_(dSrc)
	 , seekedIdx_(0)
	 , alignmentIdx_(0)
	 , rate_hz_(0.0)
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
	TelemetrySeeker::setAlignmentIdx(
		size_t idx)
	{
		alignmentIdx_ = idx;
	}

	size_t
	TelemetrySeeker::getAlignmentIdx() const
	{
		return alignmentIdx_;
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
		double offset_secs,
		double quanta_secs)
	{
		auto quantize = [](double value, double quanta){
			return std::round(value / quanta) * quanta;
		};
		auto currTimeOffset = quantize(getTimeAt(seekedIdx_),quanta_secs);
		const auto targetTimeOffset = quantize(currTimeOffset + offset_secs,quanta_secs);
		const auto maxTimeOffset = getTimeAt(size() - 1);
		if (offset_secs > 0.0)
		{
			if (targetTimeOffset < maxTimeOffset)
			{
				while (currTimeOffset < targetTimeOffset)
				{
					currTimeOffset = quantize(getTimeAt(++seekedIdx_),quanta_secs);
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
					currTimeOffset = quantize(getTimeAt(--seekedIdx_),quanta_secs);
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
	TelemetrySeeker::rateHz() const
	{
		return rate_hz_;
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

		rate_hz_ = 0.0;
		if (size() >= 2)
		{
			const auto cycles = size() - 1;
			rate_hz_ = cycles / getTimeAt(cycles);
		}

		LapIndices li;
		int prevSampLap = -1;
		int lapWereIn = -1;
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