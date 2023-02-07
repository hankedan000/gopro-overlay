#pragma once

#include <GoProTelem/SampleTypes.h>
#include <memory>
#include <opencv2/opencv.hpp>
#include <type_traits>
#include <vector>

#include "GoProOverlay/data/pod_bitset.hpp"

namespace gpo
{
	using GoProDataAvailBitSet = pod_bitset<uint64_t,1>;
	static constexpr size_t GOPRO_AVAIL_ACCL = 0;
	static constexpr size_t GOPRO_AVAIL_GYRO = 1;
	static constexpr size_t GOPRO_AVAIL_GRAV = 2;
	static constexpr size_t GOPRO_AVAIL_CORI = 3;
	static constexpr size_t GOPRO_AVAIL_GPS_LL = 4;
	static constexpr size_t GOPRO_AVAIL_GPS_SPEED2D = 5;
	static constexpr size_t GOPRO_AVAIL_GPS_SPEED3D = 6;

	// Engine Control Unit telemetry sample
	struct ECU_Sample
	{
		float engineSpeed_rpm;

		// throttle position sensor (0 to 100)
		float tps;

		float boost_psi;
	};

	struct ECU_TimedSample
	{
		ECU_Sample sample;

		// time offset relative to beginning of data recording in seconds.
		double t_offset;
	};

	using ECU_DataAvailBitSet = pod_bitset<uint64_t,1>;
	static constexpr size_t ECU_AVAIL_TIME = 0;
	static constexpr size_t ECU_AVAIL_ENGINE_SPEED = 1;
	static constexpr size_t ECU_AVAIL_TPS = 2;
	static constexpr size_t ECU_AVAIL_BOOST = 3;

	struct TelemetrySample
	{
		double t_offset;
		
		gpt::CombinedSample gpSamp;

		ECU_Sample ecuSamp;

		// corrected location of vehicle on the track.
		// currently uses a simple nearest distance algorithm based on where the
		// GPS said the vehicle was.
		gpt::CoordLL onTrackLL;

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

	static_assert(std::is_standard_layout<TelemetrySample>::value && std::is_trivial<TelemetrySample>::value, "TelemetrySample must be a POD type");

	using TelemetrySamples = std::vector<TelemetrySample>;
	using TelemetrySamplesPtr = std::shared_ptr<TelemetrySamples>;
}