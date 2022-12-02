#pragma once

#include <memory>
#include <vector>

#include "GoProOverlay/data/TelemetrySeeker.h"

namespace gpo
{
	class GroupedSeeker
	{
	public:
		GroupedSeeker();

		void
		addSeeker(
			TelemetrySeekerPtr seeker);

		bool
		addSeekerUnique(
			TelemetrySeekerPtr seeker);

		size_t
		seekerCount() const;

		TelemetrySeekerPtr
		getSeeker(
			size_t idx);

		void
		removeSeeker(
			size_t idx);

		void
		prevAll(
			bool onlyIfAllHavePrev = true);

		void
		nextAll(
			bool onlyIfAllHaveNext = true);

		void
		seekAllToIdx(
			size_t idx);

		void
		seekAllRelative(
			size_t amount,
			bool forward);

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

		void
		seekAllToLapEntry(
			unsigned int lap);

		void
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
		std::pair<size_t, size_t>
		relativeSeekLimits() const;

	private:
		std::vector<TelemetrySeekerPtr> seekers_;

	};

	using GroupedSeekerPtr = std::shared_ptr<GroupedSeeker>;
}