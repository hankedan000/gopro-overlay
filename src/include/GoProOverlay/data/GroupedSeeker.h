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
		next();

		void
		seekToIdx(
			size_t idx);

		bool
		seekToLap(
			unsigned int lap);

	private:
		std::vector<TelemetrySeekerPtr> seekers_;

	};

	using GroupedSeekerPtr = std::shared_ptr<GroupedSeeker>;
}