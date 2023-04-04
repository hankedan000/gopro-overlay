#include "GoProOverlay/utils/DataProcessingUtils.h"

#include "GoProOverlay/utils/LineSegmentUtils.h"
#include "GoProTelem/SampleMath.h"// for lerp()
#include <spdlog/spdlog.h>

namespace utils
{

	double GRAVITY = 9.80665;
	
	bool
	computeTrackTimes(
		const gpo::Track *track,
		gpo::TelemetrySamplesPtr tSamps,
		gpo::DataAvailableBitSet &avail)
	{
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

		bitset_clear(avail);
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
		std::pair<size_t,size_t> onTrackFindWindow = {0,tSamps->size()};// search everywhere initially
		for (size_t ii=0; ii<tSamps->size(); ii++)
		{
			auto &samp = tSamps->at(ii);
			auto &calcSamp = samp.calcSamp;
			cv::Vec2d currCoord(samp.gpSamp.gps.coord.lat,samp.gpSamp.gps.coord.lon);
			auto findRes = track->findClosestPointWithIdx(
						currCoord,
						onTrackFindInitialIdx,
						onTrackFindWindow);
			const auto &foundCoord = std::get<1>(findRes);
			calcSamp.onTrackLL.lat = foundCoord[0];
			calcSamp.onTrackLL.lon = foundCoord[1];
			bitset_set_bit(avail, gpo::eDA_TRACK_ON_TRACK_LATLON);
			onTrackFindInitialIdx = std::get<2>(findRes);
			onTrackFindWindow = {5,100};// reduce search space once we've found initial location

			// ---------------------
			// compute vehicle g-forces
			calcSamp.vehiAccl.lat_g = samp.gpSamp.accl.x / GRAVITY;
			calcSamp.vehiAccl.lon_g = samp.gpSamp.accl.y / -GRAVITY;

			// ---------------------

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
				bitset_set_bit(avail, gpo::eDA_TRACK_LAP);
				bitset_set_bit(avail, gpo::eDA_TRACK_LAP_TIME_OFFSET);
			}
			calcSamp.sector = currSector;
			calcSamp.sectorTimeOffset = (currSector == -1 ? 0.0 : samp.t_offset - sectorStartTimeOffset);
			if (currSector != -1)
			{
				bitset_set_bit(avail, gpo::eDA_TRACK_SECTOR);
				bitset_set_bit(avail, gpo::eDA_TRACK_SECTOR_TIME_OFFSET);
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
