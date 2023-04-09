#include "GoProOverlay/data/GroupedSeeker.h"

#include <limits>
#include <spdlog/spdlog.h>

namespace gpo
{
	GroupedSeeker::GroupedSeeker()
	 : ModifiableObject("GroupedSeeker",false,true)
	 , seekers_()
	{
	}

	void
	GroupedSeeker::clear()
	{
		seekers_.clear();
		markObjectModified(false,true);
	}

	void
	GroupedSeeker::addSeeker(
		TelemetrySeekerPtr seeker)
	{
		seekers_.push_back(seeker);
		markObjectModified(false,true);
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
			markObjectModified(false,true);
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
		markObjectModified(false,true);
	}

	bool
	GroupedSeeker::removeAllSeekers(
		TelemetrySeekerPtr seeker)
	{
		bool removed = false;
		auto it = seekers_.begin();
		while(it != seekers_.end())
		{
			if((*it) == seeker)
			{
				it = seekers_.erase(it);
				removed = true;
			}
			else
			{
				++it;
			}
		}

		markObjectModified(false,removed);

		return removed;
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
		markObjectModified(false,false);
	}

	void
	GroupedSeeker::nextAll(
			bool onlyIfAllHaveNext,
			bool sendModificationEvent)
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
		if (sendModificationEvent)
		{
			markObjectModified(false,false);
		}
	}

	void
	GroupedSeeker::seekToAlignmentInfo(
		const RenderAlignmentInfo &renderAlignInfo)
	{
		switch (renderAlignInfo.type)
		{
			case gpo::RenderAlignmentType_E::eRAT_Custom:
			{
				const auto &customAlign = renderAlignInfo.alignInfo.custom;
				for (auto seeker : seekers_)
				{
					auto alignItr = customAlign->idxBySourceName.find(seeker->getDataSourceName());
					if (alignItr != customAlign->idxBySourceName.end())
					{
						seeker->seekToIdx(alignItr->second);
					}
				}
				markObjectModified(false,false);
				break;
			}
			case gpo::RenderAlignmentType_E::eRAT_Lap:
			{
				const auto &lapAlign = renderAlignInfo.alignInfo.lap;
				if (lapAlign->side == gpo::ElementSide_E::eES_Entry)
				{
					seekAllToLapEntry(lapAlign->lap);
				}
				else
				{
					seekAllToLapExit(lapAlign->lap);
				}
				break;
			}
			case gpo::RenderAlignmentType_E::eRAT_None:
				seekAllToIdx(0);
				break;
			default:
				spdlog::error("RenderAlignmentType_E ({}) is not supported!",(int)renderAlignInfo.type);
				break;
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
		markObjectModified(false,false);
	}

	void
	GroupedSeeker::seekAllRelative(
		size_t amount,
		bool forward)
	{
		auto limits = relativeSeekLimits();
		if (forward && amount > limits.second)
		{
			amount = limits.second;
		}
		else if ( ! forward && amount > limits.first)
		{
			amount = limits.first;
		}

		for (auto &seeker : seekers_)
		{
			seeker->seekRelative(amount,forward);
		}
		markObjectModified(false,false);
	}

	void
	GroupedSeeker::seekAllRelativeTime(
		double offset_secs)
	{
		auto limits = relativeSeekLimitsTime();
		if (offset_secs > 0.0 && offset_secs > limits.second)
		{
			offset_secs = limits.second;
		}
		else if (offset_secs < 0.0 && offset_secs < limits.first)
		{
			offset_secs = limits.first;
		}

		for (auto &seeker : seekers_)
		{
			seeker->seekRelativeTime(offset_secs);
		}
		markObjectModified(false,false);
	}

	void
	GroupedSeeker::setAlignmentHere()
	{
		for (auto &seeker : seekers_)
		{
			seeker->setAlignmentIdx(seeker->seekedIdx());
		}
		markObjectModified(false,true);
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

	bool
	GroupedSeeker::seekAllToLapEntry(
		unsigned int lap)
	{
		if (lap == 0)
		{
			spdlog::warn("can't seek to lap index 0");
			return false;
		}
		for (const auto &seeker : seekers_)
		{
			if (lap >= (seeker->lapCount() + 1))
			{
				spdlog::warn(
					"'{}' doesn't have lap {}. aborting {}()",
					seeker->getAlignmentIdx(),
					lap,
					__func__);
				return false;
			}
		}

		for (auto &seeker : seekers_)
		{
			seeker->seekToLapEntry(lap);
		}
		markObjectModified(false,false);
		return true;
	}

	bool
	GroupedSeeker::seekAllToLapExit(
		unsigned int lap)
	{
		if (lap == 0)
		{
			spdlog::warn("can't seek to lap index 0");
			return false;
		}
		for (const auto &seeker : seekers_)
		{
			if (lap >= (seeker->lapCount() + 1))
			{
				spdlog::warn(
					"'{}' doesn't have lap {}. aborting {}()",
					seeker->getAlignmentIdx(),
					lap,
					__func__);
				return false;
			}
		}

		for (auto &seeker : seekers_)
		{
			seeker->seekToLapExit(lap);
		}
		markObjectModified(false,false);
		return true;
	}

	std::pair<size_t, size_t>
	GroupedSeeker::relativeSeekLimits() const
	{
		if (seekers_.empty())
		{
			return {0,0};
		}

		std::pair<size_t, size_t> limits;
		limits.first = std::numeric_limits<decltype(limits.first)>::max();
		limits.second = std::numeric_limits<decltype(limits.second)>::max();
		for (auto &seeker : seekers_)
		{
			if (seeker->size() == 0)
			{
				return {0,0};// corner case where a seeker doesn't have any samples
			}
			limits.first = std::min(limits.first, seeker->seekedIdx());
			limits.second = std::min(limits.second, (seeker->size() - seeker->seekedIdx() - 1));
		}
		return limits;
	}

	std::pair<double, double>
	GroupedSeeker::relativeSeekLimitsTime() const
	{
		if (seekers_.empty())
		{
			return {0.0,0.0};
		}

		std::pair<double,double> timeLimits;
		timeLimits.first = std::numeric_limits<decltype(timeLimits.first)>::max();
		timeLimits.second = std::numeric_limits<decltype(timeLimits.second)>::min();
		auto limits = relativeSeekLimits();
		for (auto &seeker : seekers_)
		{
			const auto seekedIdx = seeker->seekedIdx();
			const auto currTime = seeker->getTimeAt(seekedIdx);
			const auto backwardsTimeLimit = seeker->getTimeAt(seekedIdx - limits.first) - currTime;
			const auto forwardsTimeLimit = seeker->getTimeAt(seekedIdx + limits.second) - currTime;
			timeLimits.first = std::min(timeLimits.first,backwardsTimeLimit);
			timeLimits.second = std::max(timeLimits.second,forwardsTimeLimit);
		}
		return timeLimits;
	}

	bool
	GroupedSeeker::subclassApplyModifications(
        bool unnecessaryIsOkay)
	{
		return true;
	}

	bool
	GroupedSeeker::subclassSaveModifications(
        bool unnecessaryIsOkay)
	{
		return true;
	}
}