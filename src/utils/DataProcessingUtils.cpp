#include "GoProOverlay/utils/DataProcessingUtils.h"

#include "GoProOverlay/utils/LineSegmentUtils.h"

namespace utils
{
	bool
	computeTrackTimes(
		const gpo::Track *track,
		gpo::TelemetrySamplesPtr tSamps)
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

		int currLap = -1;
		int sectorSeq = 1;// increments everytime we exit a sector
		int currSector = -1;
		cv::Vec2d prevCoord;
		auto tpoItr = trackObjs.begin();
		bool isSector = (*tpoItr)->isSector();
		bool isEntry = true;
		bool finished = false;
		double lapStartTimeOffset = 0;
		double sectorStartTimeOffset = 0;
		gpo::GateType_E gateType = (*tpoItr)->getGateType();
		gpo::DetectionGate gate = (*tpoItr)->getEntryGate();
		size_t onTrackFindInitialIdx = 0;
		std::pair<size_t,size_t> onTrackFindWindow = {0,tSamps->size()};// search everywhere initially
		for (size_t ii=0; ii<tSamps->size(); ii++)
		{
			auto &samp = tSamps->at(ii);
			cv::Vec2d currCoord(samp.gpSamp.gps.coord.lat,samp.gpSamp.gps.coord.lon);
			auto findRes = track->findClosestPointWithIdx(
						currCoord,
						onTrackFindInitialIdx,
						onTrackFindWindow);
			samp.onTrackLL = std::get<1>(findRes);
			onTrackFindInitialIdx = std::get<2>(findRes);
			onTrackFindWindow = {5,100};// reduce search space once we've found initial location

			bool movedToNextObject = false;
			do
			{
				movedToNextObject = false;
				bool crossed = ii != 0 && gate.detect(prevCoord,samp.onTrackLL);
				if (crossed && gateType == gpo::GateType_E::eGT_Start)
				{
					lapStartTimeOffset = samp.gpSamp.t_offset;
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
					finished = true;
					currLap = -1;
				}
				else if (crossed && gateType == gpo::GateType_E::eGT_NOT_A_GATE)
				{
					if (isEntry)
					{
						currSector = sectorSeq;
						sectorStartTimeOffset = samp.gpSamp.t_offset;
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
					}
					else if (++tpoItr != trackObjs.end())
					{
						// crossed sector exit, so look for next track object now
						gateType = (*tpoItr)->getGateType();
						gate = (*tpoItr)->getEntryGate();
						isSector = (*tpoItr)->isSector();
						isEntry = true;
						movedToNextObject = true;
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
					}
				}
			}
			while (movedToNextObject);

			// update telemetry sample
			samp.lap = currLap;
			samp.lapTimeOffset = (currLap == -1 ? 0.0 : samp.gpSamp.t_offset - lapStartTimeOffset);
			samp.sector = currSector;
			samp.sectorTimeOffset = (currSector == -1 ? 0.0 : samp.gpSamp.t_offset - sectorStartTimeOffset);

			prevCoord = samp.onTrackLL;
		}

		return true;
	}
}
