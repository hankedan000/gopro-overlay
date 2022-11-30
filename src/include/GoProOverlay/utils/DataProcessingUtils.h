#pragma once

#include "GoProOverlay/data/TrackDataObjects.h"

namespace utils
{
	bool
	computeTrackTimes(
		const gpo::Track *track,
		gpo::TelemetrySamplesPtr tSamps);
}