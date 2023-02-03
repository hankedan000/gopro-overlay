#pragma once

#include <GoProTelem/SampleTypes.h>
#include <memory>
#include <opencv2/opencv.hpp>
#include <vector>

namespace gpo
{
	// Engine Control Unit telemetry sample
	struct ECU_Sample
	{
		// time offset relative to beginning of data recording in seconds.
		double t_offset;

		float engineSpeed_rpm;

		// throttle position sensor (0 to 100)
		float tps;

		float boost_psi;
	};

	struct ECU_TimedSample
	{
		ECU_Sample sample;

		// time offset relative to data's start time (in seconds)
		double t_offset;
	};

	struct TelemetrySample
	{
		gpt::CombinedSample gpSamp;

		ECU_Sample ecuSamp;

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