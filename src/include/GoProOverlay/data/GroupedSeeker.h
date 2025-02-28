#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

#include <GoProOverlay/data/RenderProject.h>
#include "GoProOverlay/data/TelemetrySeeker.h"

namespace gpo
{
	struct RelativeFrameLimits
	{
		size_t backwards = 0;
		size_t forwards = 0;

		RelativeFrameLimits() = default;

		inline
		size_t
		totalRange() const
		{
			return backwards + forwards + 1;
		}
	};

	struct RelativeTimeLimits
	{
		RelativeTimeLimits() = default;

		RelativeTimeLimits(
			const double backwards,
			const double forwards)
		{
			setBackwards(backwards);
			setForwards(forwards);
		}

		inline auto backwards() const {return backwards_;}
		inline auto forwards() const {return forwards_;}

		inline
		void
		setBackwards(
			const double limit)
		{
			if (limit > 0.0)
			{
				throw std::invalid_argument("backwards limit can't be positive");
			}
			backwards_ = limit;
		}

		inline
		void
		setForwards(
			const double limit)
		{
			if (limit < 0.0)
			{
				throw std::invalid_argument("forwards limit can't be negative");
			}
			forwards_ = limit;
		}

		inline
		double
		totalRange() const
		{
			return backwards_ + forwards_;
		}

	private:
		double backwards_ = 0.0;
		double forwards_ = 0.0;

	};

	class GroupedSeeker : public ModifiableObject
	{
	public:
		GroupedSeeker();

		void
		clear();

		bool
		addSeeker(
			TelemetrySeekerPtr seeker);

		size_t
		seekerCount() const;

		TelemetrySeekerPtr
		getSeeker(
			size_t idx);

		void
		removeSeeker(
			size_t idx);

		/**
		 * Removes seeker that match the following
		 * 
		 * @param[in] seeker
		 * the seeker pointer to remove
		 * 
		 * @return
		 * true if any were removed. false otherwise.
		 */
		bool
		removeSeeker(
			TelemetrySeekerPtr seeker);

		void
		prevAll(
			bool onlyIfAllHavePrev = true);

		void
		nextAll(
			bool onlyIfAllHaveNext = true,
			bool sendModificationEvent = true);

		void
		seekToAlignmentInfo(
			const RenderAlignmentInfo &renderAlignInfo);

		void
		seekAllRelative(
			size_t amount,
			const SeekDirection dir);

		void
		seekAllRelativeTime(
			double offset_secs);

		void
		setAlignmentHere();

		/**
		 * @return
		 * the lowest number of laps a seeker has in the group
		 */
		unsigned int
		minLapCount() const;

		/**
		 * @return
		 * the most number of laps a seeker has in the group
		 */
		unsigned int
		maxLapCount() const;

		bool
		seekAllToLapEntry(
			unsigned int lap);

		bool
		seekAllToLapExit(
			unsigned int lap);

		/**
		 * Imagine we have three TelemetrySeekers that are seeked to some random point in
		 * their data set. If we aligned them all based on their current location, then
		 * this function would return the relative distances you could seek to while still
		 * remaining within all dataset ranges.
		 * 
		 *                :<- seeked point
		 *                :
		 *     ___________:___________________________
		 *    |___________:___________________________|
		 *   _____________:_____________________
		 *  |_____________:_____________________|
		 *       _________:________________________________
		 *      |_________:________________________________|
		 *                :
		 *                :
		 *      <----- relative seek limits ---->
		 *     +10        0                    +22
		 */
		RelativeFrameLimits
		relativeSeekLimits() const;

		/**
		 * Same logical meaning as relativeSeekLimits(), but limits are in terms of time.
		 * Intended to be used with the seekAllRelativeTime().
		 * Note: returned times are signed, so the first will be negative.
		 */
		RelativeTimeLimits
		relativeSeekLimitsTime() const;

	protected:
        bool
        subclassApplyModifications(
       		bool unnecessaryIsOkay) override;

        bool
        subclassSaveModifications(
        	bool unnecessaryIsOkay) override;

		void
		seekAllToIdx(
			size_t idx);

	private:
		std::vector<TelemetrySeekerPtr> seekers_;

	};

	using GroupedSeekerPtr = std::shared_ptr<GroupedSeeker>;
}