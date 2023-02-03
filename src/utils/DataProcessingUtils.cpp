#include "GoProOverlay/utils/DataProcessingUtils.h"

#include <csv.hpp>
#include "GoProOverlay/utils/LineSegmentUtils.h"
#include "GoProTelem/SampleMath.h"// for lerp()
#include <spdlog/spdlog.h>

namespace utils
{
	bool
	readMegaSquirtLog(
		const std::string mslPath,
		std::vector<gpo::VehicleTelemetry> &vehicleTelem)
	{
		bool okay = true;

		csv::CSVReader reader(mslPath);

		auto colNames = reader.get_col_names();
		int timeColIdx = -1;
		int engineRPM_ColIdx = -1;
		int tpsColIdx = -1;
		int boostColIdx = -1;
		for (size_t cc=0; cc<colNames.size(); cc++)
		{
			const auto &colName = colNames.at(cc);
			if (timeColIdx < 0 && colName.compare("Time") == 0)
			{
				timeColIdx = cc;
			}
			else if (engineRPM_ColIdx < 0 && colName.compare("RPM") == 0)
			{
				engineRPM_ColIdx = cc;
			}
			else if (tpsColIdx < 0 && colName.compare("TPS") == 0)
			{
				tpsColIdx = cc;
			}
			else if (boostColIdx < 0 && colName.compare("Boost psi") == 0)
			{
				boostColIdx = cc;
			}
		}
		spdlog::debug("timeColIdx = {}", timeColIdx);
		spdlog::debug("engineRPM_ColIdx = {}", engineRPM_ColIdx);
		spdlog::debug("tpsColIdx = {}", tpsColIdx);
		spdlog::debug("boostColIdx = {}", boostColIdx);
		if (timeColIdx < 0)
		{
			spdlog::warn("could not determine 't_offset' (time) column index");
		}
		if (engineRPM_ColIdx < 0)
		{
			spdlog::warn("could not determine 'engineRPM' column index");
		}
		if (tpsColIdx < 0)
		{
			spdlog::warn("could not determine 'tps' column index");
		}
		if (boostColIdx < 0)
		{
			spdlog::warn("could not determine 'boost_psi' column index");
		}

		size_t rowIdx = 0;
		vehicleTelem.clear();
		vehicleTelem.reserve(1024);// prealloc memory for ~100s log (assume 10Hz data rate)
		for (auto rowItr=reader.begin(); rowItr!=reader.end(); rowItr++,rowIdx++)
		{
			if (rowIdx == 0)
			{
				// 2nd row contains units
				continue;
			}

			gpo::VehicleTelemetry vTelem;
			if (timeColIdx >= 0)
			{
				vTelem.t_offset = (*rowItr)[timeColIdx].get<float>();
			}
			if (engineRPM_ColIdx >= 0)
			{
				vTelem.engineSpeed_rpm = (*rowItr)[engineRPM_ColIdx].get<float>();
			}
			if (tpsColIdx >= 0)
			{
				vTelem.tps = (*rowItr)[tpsColIdx].get<float>();
			}
			if (boostColIdx >= 0)
			{
				vTelem.boost_psi = (*rowItr)[boostColIdx].get<float>();
			}
			vehicleTelem.push_back(vTelem);
		}

		return okay;
	}
	
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

	void
	lerp(
		gpo::VehicleTelemetry &out,
		const gpo::VehicleTelemetry &a,
		const gpo::VehicleTelemetry &b,
		double ratio)
	{
		out.t_offset = gpt::lerp(a.t_offset, b.t_offset, ratio);
		out.engineSpeed_rpm = gpt::lerp(a.engineSpeed_rpm, b.engineSpeed_rpm, ratio);
		out.tps = gpt::lerp(a.tps, b.tps, ratio);
		out.boost_psi = gpt::lerp(a.boost_psi, b.boost_psi, ratio);
	}

	void
	resample(
		std::vector<gpo::VehicleTelemetry> &out,
		const std::vector<gpo::VehicleTelemetry> &in,
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
