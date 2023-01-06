#pragma once

#include <GoProTelem/SampleTypes.h>
#include <memory>
#include <opencv2/opencv.hpp>
#include <vector>

namespace gpo
{
	struct TelemetrySample
	{
		gpt::CombinedSample gpSamp;

		// corrected location of vehicle on the track.
		// currently uses a simple nearest distance algorithm based on where the
		// GPS said the vehicle was.
		cv::Vec2d onTrackLL;

		// -1 if not within a track
		// starts at 1 and counts up from there
		int lap;

		// if within a lap (lap != -1), this value represents the time offset from
		// when we last crossed the track's starting gate
		double lapTimeOffset;

		// -1 if not within a track sector
		// starts at 1 and counts up from there
		int sector;

		// if within a sector (sector != -1), this value represents the time offset
		// from when we croseed the sector's entry gate
		double sectorTimeOffset;

	};

	using TelemetrySamples = std::vector<TelemetrySample>;
	using TelemetrySamplesPtr = std::shared_ptr<TelemetrySamples>;
}