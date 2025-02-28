#include "GoProOverlay/data/GroupedSeeker.h"

#include <limits>
#include <spdlog/spdlog.h>
#include <stdexcept>

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
		clearNeedsApply();
		clearNeedsSave();
	}

	bool
	GroupedSeeker::addSeeker(
		TelemetrySeekerPtr seeker)
	{
		bool isNewSeeker = true;
		for (const auto &mySeeker : seekers_)
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
	GroupedSeeker::removeSeeker(
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
			for (const auto &seeker : seekers_)
			{
				if ( ! seeker->hasPrev())
				{
					return;// don't move anybody if one can't be moved
				}
			}
		}

		for (const auto &seeker : seekers_)
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
			for (const auto &seeker : seekers_)
			{
				if ( ! seeker->hasNext())
				{
					return;// don't move anybody if one can't be moved
				}
			}
		}

		for (const auto &seeker : seekers_)
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
			case gpo::RenderAlignmentType_E::eRAT_Sector:
				throw std::invalid_argument("eRAT_Sector is not supported");
				break;
		}
	}

	void
	GroupedSeeker::seekAllRelative(
		size_t amount,
		const SeekDirection dir)
	{
		const auto limits = relativeSeekLimits();
		switch (dir)
		{
			case gpo::SeekDirection::Forward:
				if (amount > limits.forwards)
				{
					amount = limits.forwards;
				}
				break;
			case gpo::SeekDirection::Backward:
				if (amount > limits.backwards)
				{
					amount = limits.backwards;
				}
				break;
		}

		for (const auto &seeker : seekers_)
		{
			seeker->seekRelative(amount, dir);
		}
		markObjectModified(false,false);
	}

	void
	GroupedSeeker::seekAllRelativeTime(
		double offset_secs)
	{
		auto limits = relativeSeekLimitsTime();
		if (offset_secs > 0.0 && offset_secs > limits.forwards())
		{
			offset_secs = limits.forwards();
		}
		else if (offset_secs < 0.0 && offset_secs < limits.backwards())
		{
			offset_secs = limits.backwards();
		}

		for (const auto &seeker : seekers_)
		{
			seeker->seekRelativeTime(offset_secs);
		}
		markObjectModified(false,false);
	}

	void
	GroupedSeeker::setAlignmentHere()
	{
		for (const auto &seeker : seekers_)
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

		for (const auto &seeker : seekers_)
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

		for (const auto &seeker : seekers_)
		{
			seeker->seekToLapExit(lap);
		}
		markObjectModified(false,false);
		return true;
	}

	RelativeFrameLimits
	GroupedSeeker::relativeSeekLimits() const
	{
		if (seekers_.empty())
		{
			return {0,0};
		}

		RelativeFrameLimits limits;
		limits.backwards = std::numeric_limits<decltype(limits.backwards)>::max();
		limits.forwards = std::numeric_limits<decltype(limits.forwards)>::max();
		for (auto &seeker : seekers_)
		{
			if (seeker->size() == 0)
			{
				return {0,0};// corner case where a seeker doesn't have any samples
			}
			limits.backwards = std::min(limits.backwards, seeker->seekedIdx());
			limits.forwards = std::min(limits.forwards, (seeker->size() - seeker->seekedIdx() - 1));
		}
		return limits;
	}

	RelativeTimeLimits
	GroupedSeeker::relativeSeekLimitsTime() const
	{
		if (seekers_.empty())
		{
			return RelativeTimeLimits();
		}

		RelativeTimeLimits timeLimits;
		timeLimits.setBackwards(std::numeric_limits<decltype(timeLimits.backwards())>::max());
		timeLimits.setForwards(std::numeric_limits<decltype(timeLimits.forwards())>::min());
		const auto limits = relativeSeekLimits();
		for (auto &seeker : seekers_)
		{
			const auto seekedIdx = seeker->seekedIdx();
			const auto currTime = seeker->getTimeAt(seekedIdx);
			const auto backwardsTimeLimit = seeker->getTimeAt(seekedIdx - limits.backwards) - currTime;
			const auto forwardsTimeLimit = seeker->getTimeAt(seekedIdx + limits.forwards) - currTime;
			timeLimits.setBackwards(std::min(timeLimits.backwards(), backwardsTimeLimit));
			timeLimits.setForwards(std::max(timeLimits.forwards(),forwardsTimeLimit));
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

	void
	GroupedSeeker::seekAllToIdx(
		size_t idx)
	{
		for (const auto &seeker : seekers_)
		{
			seeker->seekToIdx(idx);
		}
		markObjectModified(false,false);
	}
}