#include "GoProOverlay/utils/DataProcessingUtils.h"

#include "GoProOverlay/utils/LineSegmentUtils.h"
#include "GoProTelem/SampleMath.h"// for lerp()
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace utils
{
	
	float
	magnitude(
		const cv::Vec3f &v)
	{
		return std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	}
	
	cv::Vec3f
	normalize(
		const cv::Vec3f &v)
	{
		float m = magnitude(v);
		cv::Vec3f norm;
		norm[0] = v[0] / m;
		norm[1] = v[1] / m;
		norm[2] = v[2] / m;
		return norm;
	}

	float
	dot(
		const cv::Vec3f &a,
		const cv::Vec3f &b)
	{
		return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
	}
	
	cv::Vec3f
	projection(
		const cv::Vec3f &vec,
		const cv::Vec3f &vecOnto)
	{
		// projection is equal to
		// u = dot(a, norm(b)) * norm(b)
		float d = dot(vec,vecOnto);
		cv::Vec3f prj;
		prj[0] = d * vecOnto[0];
		prj[1] = d * vecOnto[1];
		prj[2] = d * vecOnto[2];
		return prj;
	}

	bool
	computeVehicleDirectionVectors(
		gpo::TelemetrySamplesPtr tSamps,
		const gpo::DataAvailableBitSet &avail,
		cv::Vec3f &latDir,
		cv::Vec3f &lonDir)
	{
		if (tSamps->empty())
		{
			return false;
		}

		// default init
		latDir[Vec3::x] = +0.0;
		latDir[Vec3::y] = +0.0;
		latDir[Vec3::z] = +0.0;
		lonDir[Vec3::x] = +0.0;
		lonDir[Vec3::y] = +0.0;
		lonDir[Vec3::z] = +0.0;

		// Assume camera is always recording toward the vehicle's direction of motion
		// TODO if we wanted to improve this, we could detect when the vehicle's GPS
		// speed is increasing while there is no gyroscopic motion (acceleration in
		// a straight line). The direction of the average acceleration vector would
		// tell you the vehicle's longitudinal direction.
		lonDir[Vec3::y] = -1.0;

		if (bitset_is_set(avail, gpo::DataAvailable::eDA_GOPRO_GRAV))
		{
			// base lateral direction vectors on direction of gravity
			// and our assumption on the vehicle's longitudinal direction.
			const auto &grav0 = tSamps->at(0).gpSamp.grav;
			const float G_THRESHOLD = 0.5;// half G
			if (grav0.x > G_THRESHOLD)
			{
				latDir[Vec3::z] = -1.0;
			}
			else if (grav0.x < -G_THRESHOLD)
			{
				latDir[Vec3::z] = +1.0;
			}
			else if (grav0.z > G_THRESHOLD)
			{
				latDir[Vec3::x] = +1.0;
			}
			else if (grav0.z < -G_THRESHOLD)
			{
				latDir[Vec3::x] = -1.0;
			}
			else
			{
				spdlog::warn("lateral direction vector is indeterminate based on gravity");
				latDir[Vec3::x] = -1.0;
			}
		}
		else
		{
			spdlog::warn("telemetry doesn't have gravity vector. using default lateral direction vectors");
			latDir[Vec3::x] = -1.0;
		}

		return true;
	}

	bool
	computeVehicleAcceleration(
		gpo::TelemetrySamplesPtr tSamps,
		gpo::DataAvailableBitSet &avail,
		const cv::Vec3f &latDir,
		const cv::Vec3f &lonDir)
	{
		const cv::Vec3f latDirNorm = normalize(latDir);
		const cv::Vec3f lonDirNorm = normalize(lonDir);

		// find non-zero component of lateral & longidtudinal direction vectors.
		// we'll use these later when projecting the net accerlation vector onto
		// the lateral & longitudinal direction vectors.
		size_t iiLat = 0;
		size_t iiLon = 0;
		for (size_t ii=0; ii<3; ii++)
		{
			if (latDirNorm[ii] != 0.0)
			{
				iiLat = ii;
			}
			if (lonDirNorm[ii] != 0.0)
			{
				iiLon = ii;
			}
		}

		// -----------------------------------
		// determine where we should source our acceleration from.
		// prefer smoothed acceleration if available.

		enum AcclSource
		{
			eNormalAccl,
			eSmoothedAccl
		};

		AcclSource acclSource = AcclSource::eNormalAccl;
		if (bitset_is_set(avail, gpo::DataAvailable::eDA_CALC_SMOOTH_ACCL))
		{
			acclSource = AcclSource::eSmoothedAccl;
		}
		else if ( ! bitset_is_set(avail, gpo::DataAvailable::eDA_GOPRO_ACCL))
		{
			spdlog::error("{} - telemetry has no acceleration source", __func__);
			return false;
		}

		// -----------------------------------
		// compute vehicle acceleration at each sample

		for (size_t ii=0; ii<tSamps->size(); ii++)
		{
			auto &samp = tSamps->at(ii);

			// get net acceleration vector
			cv::Vec3d accl;
			switch (acclSource)
			{
				case AcclSource::eNormalAccl:
					accl[Vec3::x] = samp.gpSamp.accl.x;
					accl[Vec3::y] = samp.gpSamp.accl.y;
					accl[Vec3::z] = samp.gpSamp.accl.z;
					break;
				case AcclSource::eSmoothedAccl:
					accl[Vec3::x] = samp.calcSamp.smoothAccl.x;
					accl[Vec3::y] = samp.calcSamp.smoothAccl.y;
					accl[Vec3::z] = samp.calcSamp.smoothAccl.z;
					break;
			}

			// remove gravity vector if provided
			if (bitset_is_set(avail, gpo::DataAvailable::eDA_GOPRO_GRAV))
			{
				accl[Vec3::x] -= samp.gpSamp.grav.x * constants::GRAVITY;
				accl[Vec3::y] -= samp.gpSamp.grav.y * constants::GRAVITY;
				accl[Vec3::z] -= samp.gpSamp.grav.z * constants::GRAVITY;
			}

			// compute lateral & longitudinal g-force vectors based on directionality
			cv::Vec3f latProj = projection(accl,latDirNorm);
			cv::Vec3f lonProj = projection(accl,lonDirNorm);
			samp.calcSamp.vehiAccl.lat_g = latProj[iiLat] / latDirNorm[iiLat] / constants::GRAVITY;
			samp.calcSamp.vehiAccl.lon_g = lonProj[iiLon] / lonDirNorm[iiLon] / constants::GRAVITY;
		}

		bitset_set_bit(avail, gpo::DataAvailable::eDA_CALC_VEHI_ACCL);

		return true;
	}
	
	bool
	computeTrackTimes(
		const std::shared_ptr<const gpo::Track> &track,
		gpo::TelemetrySamplesPtr tSamps,
		gpo::DataAvailableBitSet &avail)
	{
		if ( ! track)
		{
			spdlog::error("{} - track can't be null!", __func__);
			return false;
		}
		
		std::vector<const gpo::TrackPathObject *> trackObjs;
		if ( ! track->getSortedPathObjects(trackObjs))
		{
			return false;
		}
		else if (trackObjs.empty())
		{
			// no track objects to process
			return true;
		}
		size_t totalGatesInTrack = 0;
		for (const auto &trackObj : trackObjs)
		{
			if (trackObj->isGate())
			{
				totalGatesInTrack += 1;
			}
			else if (trackObj->isSector())
			{
				totalGatesInTrack += 2;
			}
			else
			{
				spdlog::warn("unknown track object. it's not a gate and it's not a sector.");
			}
		}

		bitset_clr_bit(avail, gpo::eDA_CALC_LAP);
		bitset_clr_bit(avail, gpo::eDA_CALC_LAP_TIME_OFFSET);
		bitset_clr_bit(avail, gpo::eDA_CALC_SECTOR);
		bitset_clr_bit(avail, gpo::eDA_CALC_SECTOR_TIME_OFFSET);
		int currLap = -1;
		int sectorSeq = 1;// increments everytime we exit a sector
		int currSector = -1;
		cv::Vec2d prevCoord;
		auto tpoItr = trackObjs.begin();
		bool isSector = (*tpoItr)->isSector();
		bool isEntry = true;
		double lapStartTimeOffset = 0;
		double sectorStartTimeOffset = 0;
		gpo::GateType_E gateType = (*tpoItr)->getGateType();
		gpo::DetectionGate gate = (*tpoItr)->getEntryGate();
		size_t onTrackFindInitialIdx = 0;
		for (size_t ii=0; ii<tSamps->size(); ii++)
		{
			auto &samp = tSamps->at(ii);
			auto &calcSamp = samp.calcSamp;
			cv::Vec2d currCoord(samp.gpSamp.gps.coord.lat,samp.gpSamp.gps.coord.lon);
			std::pair<size_t,size_t> onTrackFindWindow = {5,100};
			if (samp.gpSamp.gps.speed2D < 0.447)// 0.447m/s ~= 1mph
			{
				// search everywhere when speed is low or stationary. we do this because we
				// can get can have "clumps" of points to search through. this can result
				// in us getting stuck inside the clump and never being able to search past
				// if the search window isn't wide enough
				onTrackFindWindow = {tSamps->size(),tSamps->size()};
			}
			else if (samp.gpSamp.gps.speed2D < 2.25)// 2.25m/s ~= 5mph
			{
				onTrackFindWindow = {100,500};
			}
			// TODO might want to look into quadtrees to improve this search algorithm
			auto findRes = track->findClosestPointWithIdx(
						currCoord,
						onTrackFindInitialIdx,
						onTrackFindWindow);
			const auto &foundCoord = std::get<1>(findRes);
			calcSamp.onTrackLL.lat = foundCoord[0];
			calcSamp.onTrackLL.lon = foundCoord[1];
			bitset_set_bit(avail, gpo::eDA_CALC_ON_TRACK_LATLON);
			onTrackFindInitialIdx = std::get<2>(findRes);

			// see if we crossed 'gate'. if so, then move to the next logical gate in the 'trackObjs'
			// list. we check these in a loop because sectors can have gates that are back-to-back,
			// in which case we want to quickly progress through them and look for the next one that
			// hasn't be crossed yet.
			bool movedToNextObject = false;
			size_t numMoves = 0;
			do
			{
				movedToNextObject = false;
				bool crossed = ii != 0 && gate.detect(prevCoord,foundCoord);
				if (crossed && gateType == gpo::GateType_E::eGT_Start)
				{
					lapStartTimeOffset = samp.t_offset;
					if (currLap == -1)
					{
						currLap = 1;
					}
					else
					{
						currLap++;
					}
				}
				else if (crossed && gateType == gpo::GateType_E::eGT_Finish)
				{
					currLap = -1;
				}
				else if (crossed && gateType == gpo::GateType_E::eGT_NOT_A_GATE)
				{
					if (isEntry)
					{
						currSector = sectorSeq;
						sectorStartTimeOffset = samp.t_offset;
					}
					else
					{
						currSector = -1;
						sectorSeq++;
					}
				}

				// determine next gate to monitor for cross detection
				if (crossed && ! isSector)
				{
					if (++tpoItr != trackObjs.end())
					{
						// crossed regular gate, so look for next track object now
						gateType = (*tpoItr)->getGateType();
						gate = (*tpoItr)->getEntryGate();
						isSector = (*tpoItr)->isSector();
						isEntry = true;
						movedToNextObject = true;
						numMoves++;
					}
					else
					{
						// crossed last track object, so loop back to first
						sectorSeq = 1;
						tpoItr = trackObjs.begin();
						gateType = (*tpoItr)->getGateType();
						gate = (*tpoItr)->getEntryGate();
						isSector = (*tpoItr)->isSector();
						isEntry = true;
						movedToNextObject = true;
						numMoves++;
					}
				}
				else if (crossed && isSector)
				{
					if (isEntry)
					{
						// crossed sector entry, so look for exit now
						gate = (*tpoItr)->getExitGate();
						isEntry = false;
						movedToNextObject = true;
						numMoves++;
					}
					else if (++tpoItr != trackObjs.end())
					{
						// crossed sector exit, so look for next track object now
						gateType = (*tpoItr)->getGateType();
						gate = (*tpoItr)->getEntryGate();
						isSector = (*tpoItr)->isSector();
						isEntry = true;
						movedToNextObject = true;
						numMoves++;
					}
					else
					{
						// crossed last track object, so loop back to first
						sectorSeq = 1;
						tpoItr = trackObjs.begin();
						gateType = (*tpoItr)->getGateType();
						gate = (*tpoItr)->getEntryGate();
						isSector = (*tpoItr)->isSector();
						isEntry = true;
						movedToNextObject = true;
						numMoves++;
					}
				}
			}
			while (movedToNextObject && numMoves < (totalGatesInTrack - 1));

			// update telemetry sample
			calcSamp.lap = currLap;
			calcSamp.lapTimeOffset = (currLap == -1 ? 0.0 : samp.t_offset - lapStartTimeOffset);
			if (currLap != -1)
			{
				bitset_set_bit(avail, gpo::eDA_CALC_LAP);
				bitset_set_bit(avail, gpo::eDA_CALC_LAP_TIME_OFFSET);
			}
			calcSamp.sector = currSector;
			calcSamp.sectorTimeOffset = (currSector == -1 ? 0.0 : samp.t_offset - sectorStartTimeOffset);
			if (currSector != -1)
			{
				bitset_set_bit(avail, gpo::eDA_CALC_SECTOR);
				bitset_set_bit(avail, gpo::eDA_CALC_SECTOR_TIME_OFFSET);
			}

			prevCoord = foundCoord;
		}

		return true;
	}

	void
	lerp(
		gpo::ECU_Sample &out,
		const gpo::ECU_Sample &a,
		const gpo::ECU_Sample &b,
		double ratio)
	{
		out.engineSpeed_rpm = gpt::lerp(a.engineSpeed_rpm, b.engineSpeed_rpm, ratio);
		out.tps = gpt::lerp(a.tps, b.tps, ratio);
		out.boost_psi = gpt::lerp(a.boost_psi, b.boost_psi, ratio);
	}

	void
	lerp(
		gpo::ECU_TimedSample &out,
		const gpo::ECU_TimedSample &a,
		const gpo::ECU_TimedSample &b,
		double ratio)
	{
		out.t_offset = gpt::lerp(a.t_offset, b.t_offset, ratio);
		lerp(out.sample, a.sample, b.sample, ratio);
	}

	#define FIELD_LERP(OUT,A,B,RATIO,FIELD) OUT.FIELD = gpt::lerp(A.FIELD, B.FIELD, RATIO)
	#define FIELD_LERP_ROUNDED(OUT,A,B,RATIO,FIELD) OUT.FIELD = std::round(gpt::lerp(A.FIELD, B.FIELD, RATIO))

	void
	lerp(
		gpo::TelemetrySample &out,
		const gpo::TelemetrySample &a,
		const gpo::TelemetrySample &b,
		double ratio)
	{
		static bool warnedOnce = false;
		if ( ! warnedOnce)
		{
			spdlog::warn("lerp() on TelemetrySample is not a complete implementation yet");
			warnedOnce = true;
		}
		
		FIELD_LERP(out,a,b,ratio,t_offset);
		// lerp(out.gpSamp, a.gpSamp, b.gpSamp, ratio);
		lerp(out.ecuSamp, a.ecuSamp, b.ecuSamp, ratio);
		// FIELD_LERP(out,a,b,ratio,onTrackLL);
		FIELD_LERP_ROUNDED(out,a,b,ratio,calcSamp.lap);
		FIELD_LERP(out,a,b,ratio,calcSamp.lapTimeOffset);
		FIELD_LERP_ROUNDED(out,a,b,ratio,calcSamp.sector);
		FIELD_LERP(out,a,b,ratio,calcSamp.sectorTimeOffset);
		lerp(out.calcSamp.smoothAccl, a.calcSamp.smoothAccl, b.calcSamp.smoothAccl, ratio);
		FIELD_LERP(out,a,b,ratio,calcSamp.vehiAccl.lat_g);
		FIELD_LERP(out,a,b,ratio,calcSamp.vehiAccl.lon_g);
	}

	void
	resample(
		std::vector<gpo::ECU_TimedSample> &out,
		const std::vector<gpo::ECU_TimedSample> &in,
		double outRate_hz)
	{
		if (in.empty())
		{
			return;
		}

		double duration_sec = in.back().t_offset;
		size_t nSampsOut = round(outRate_hz * duration_sec);
		out.resize(nSampsOut);
		double outDt_sec = 1.0 / outRate_hz;

		size_t takeIdx = 0;
		double outTime_sec = 0.0;
		for (size_t outIdx=0; outIdx<nSampsOut; outIdx++)
		{
			bool found = gpt::findLerpIndex(takeIdx,in,outTime_sec);

			if (found)
			{
				const auto &sampA = in.at(takeIdx);
				const auto &sampB = in.at(takeIdx+1);
				const double dt = sampB.t_offset - sampA.t_offset;
				const double ratio = (outTime_sec - sampA.t_offset) / dt;
				lerp(out.at(outIdx),sampA,sampB,ratio);
			}
			else if (takeIdx == 0)
			{
				out.at(outIdx) = in.at(takeIdx);
			}
			else
			{
				out.at(outIdx) = in.back();
			}
			outTime_sec += outDt_sec;
		}
	}
}
