#pragma once

#include <GoProTelem/SampleTypes.h>
#include <memory>
#include <opencv2/opencv.hpp>
#include <type_traits>
#include <vector>

#include "GoProOverlay/data/pod_bitset.hpp"

namespace gpo
{
	enum DataAvailable
	{
		// GoPro
		eDA_GOPRO_ACCL = 0,
		eDA_GOPRO_GYRO = 1,
		eDA_GOPRO_GRAV = 2,
		eDA_GOPRO_CORI = 3,
		eDA_GOPRO_GPS_LATLON = 4,
		eDA_GOPRO_GPS_ALTITUDE = 5,
		eDA_GOPRO_GPS_SPEED2D = 6,
		eDA_GOPRO_GPS_SPEED3D = 7,

		// ECU
		eDA_ECU_TIME = 64,
		eDA_ECU_ENGINE_SPEED = 65,
		eDA_ECU_TPS = 66,
		eDA_ECU_BOOST = 67,

		// Track
		eDA_TRACK_ON_TRACK_LATLON = 192,
		eDA_TRACK_LAP = 193,
		eDA_TRACK_LAP_TIME_OFFSET = 194,
		eDA_TRACK_SECTOR = 195,
		eDA_TRACK_SECTOR_TIME_OFFSET = 196
	};

	using DataAvailableBitSet = pod_bitset<uint64_t,4>;

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

	struct VehicleAccl
	{
		// lateral g-forces (turning) experienced by vehicle. (+) is to vehicle's
		// right side when facing natural direction of motion. Does not include
		// acceleration due to gravity.
		float lat_g;

		// longitudinal g-forces (braking/accelerating) experienced by vehicle.
		// (+) is facing natural direction of motion.
		float lon_g;
	};

	struct CalcSample
	{
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

		// lateral & longitudinal g-forces experienced by the vehicle
		VehicleAccl vehiAccl;
	};

	struct TelemetrySample
	{
		double t_offset;
		
		gpt::CombinedSample gpSamp;

		ECU_Sample ecuSamp;

		CalcSample calcSamp;
	};

	static_assert(std::is_standard_layout<TelemetrySample>::value && std::is_trivial<TelemetrySample>::value, "TelemetrySample must be a POD type");

	using TelemetrySamples = std::vector<TelemetrySample>;
	using TelemetrySamplesPtr = std::shared_ptr<TelemetrySamples>;
}