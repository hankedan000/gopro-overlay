#include "GoProOverlay/data/GroupedSeeker.h"

#include <limits>

namespace gpo
{
	GroupedSeeker::GroupedSeeker()
	 : seekers_()
	{
	}

	void
	GroupedSeeker::addSeeker(
		TelemetrySeekerPtr seeker)
	{
		seekers_.push_back(seeker);
	}

	bool
	GroupedSeeker::addSeekerUnique(
		TelemetrySeekerPtr seeker)
	{
		bool isNewSeeker = true;
		for (auto mySeeker : seekers_)
		{
			if (mySeeker == seeker)
			{
				isNewSeeker = false;
				break;
			}
		}

		if (isNewSeeker)
		{
			seekers_.push_back(seeker);
		}
		return isNewSeeker;
	}

	size_t
	GroupedSeeker::seekerCount() const
	{
		return seekers_.size();
	}

	TelemetrySeekerPtr
	GroupedSeeker::getSeeker(
		size_t idx)
	{
		return seekers_.at(idx);
	}

	void
	GroupedSeeker::removeSeeker(
		size_t idx)
	{
		seekers_.erase(std::next(seekers_.begin(), idx));
	}

	void
	GroupedSeeker::prevAll(
			bool onlyIfAllHavePrev)
	{
		if (onlyIfAllHavePrev)
		{
			for (auto &seeker : seekers_)
			{
				if ( ! seeker->hasPrev())
				{
					return;// don't move anybody if one can't be moved
				}
			}
		}

		for (auto &seeker : seekers_)
		{
			seeker->prev();
		}
	}

	void
	GroupedSeeker::nextAll(
			bool onlyIfAllHaveNext)
	{
		if (onlyIfAllHaveNext)
		{
			for (auto &seeker : seekers_)
			{
				if ( ! seeker->hasNext())
				{
					return;// don't move anybody if one can't be moved
				}
			}
		}

		for (auto &seeker : seekers_)
		{
			seeker->next();
		}
	}

	void
	GroupedSeeker::seekAllToIdx(
		size_t idx)
	{
		for (auto &seeker : seekers_)
		{
			seeker->seekToIdx(idx);
		}
	}

	unsigned int
	GroupedSeeker::minLapCount() const
	{
		if (seekers_.empty())
		{
			return 0;
		}

		unsigned int min = std::numeric_limits<unsigned int>::max();
		for (auto &seeker : seekers_)
		{
			min = std::min(min,seeker->lapCount());
		}
		return min;
	}

	unsigned int
	GroupedSeeker::maxLapCount() const
	{
		if (seekers_.empty())
		{
			return 0;
		}

		unsigned int max = std::numeric_limits<unsigned int>::min();
		for (auto &seeker : seekers_)
		{
			max = std::max(max,seeker->lapCount());
		}
		return max;
	}

	void
	GroupedSeeker::seekAllToLapEntry(
		unsigned int lap)
	{
		for (auto &seeker : seekers_)
		{
			seeker->seekToLapEntry(lap);
		}
	}

	void
	GroupedSeeker::seekAllToLapExit(
		unsigned int lap)
	{
		for (auto &seeker : seekers_)
		{
			seeker->seekToLapExit(lap);
		}
	}

	std::pair<long, long>
	GroupedSeeker::relativeSeekLimits() const
	{
		if (seekers_.empty())
		{
			return {0,0};
		}

		std::pair<long, long> limits;
		limits.first = std::numeric_limits<decltype(limits.first)>::max();
		limits.second = std::numeric_limits<decltype(limits.second)>::max();
		for (auto &seeker : seekers_)
		{
			if (seeker->size() == 0)
			{
				return {0,0};// corner case where a seeker doesn't have any samples
			}
			limits.first = std::min(limits.first, (long)(seeker->seekedIdx()));
			limits.second = std::min(limits.second, (long)(seeker->size() - seeker->seekedIdx() - 1));
		}
		limits.first *= -1;
		return limits;
	}
}