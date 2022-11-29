#include "GoProOverlay/data/GroupedSeeker.h"

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
	GroupedSeeker::next()
	{
		for (auto &seeker : seekers_)
		{
			seeker->next();
		}
	}

	void
	GroupedSeeker::seekToIdx(
		size_t idx)
	{
		for (auto & seeker : seekers_)
		{
			seeker->seekToIdx(idx);
		}
	}
}