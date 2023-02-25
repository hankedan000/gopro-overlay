#include "GoProOverlay/data/TrackDataObjects.h"

#include <spdlog/spdlog.h>
#include <stdexcept>
#include <fstream>

#include "GoProOverlay/utils/LineSegmentUtils.h"

namespace gpo
{

	DetectionGate::DetectionGate()
	 : a_()
	 , b_()
	{
	}

	DetectionGate::DetectionGate(
		cv::Vec2d a,
		cv::Vec2d b)
	 : a_(a)
	 , b_(b)
	{
	}
	
	bool
	DetectionGate::detect(
		cv::Vec2d c1,
		cv::Vec2d c2)
	{
		return utils::doIntersect(a_,b_,c1,c2);
	}

	const cv::Vec2d &
	DetectionGate::a() const
	{
		return a_;
	}

	const cv::Vec2d &
	DetectionGate::b() const
	{
		return b_;
	}

	cv::Vec2d &
	DetectionGate::a()
	{
		return a_;
	}

	cv::Vec2d &
	DetectionGate::b()
	{
		return b_;
	}

	TrackPathObject::TrackPathObject(
		Track *track,
		std::string name)
	 : track_(track)
	 , name_(name)
	{
	}

	TrackPathObject::~TrackPathObject()
	{
	}

	Track*
	TrackPathObject::getTrack()
	{
		return track_;
	}

	bool
	TrackPathObject::isGate() const
	{
		return false;
	}

	bool
	TrackPathObject::isSector() const
	{
		return false;
	}

	GateType_E
	TrackPathObject::getGateType() const
	{
		return GateType_E::eGT_NOT_A_GATE;
	}

	const std::string &
	TrackPathObject::getName() const
	{
		return name_;
	}

	void
	TrackPathObject::setName(
		std::string name)
	{
		name_ = name;
	}


	TrackSector::TrackSector(
		Track *track,
		std::string name,
		size_t entryIdx,
		size_t exitIdx)
	 : TrackSector(track,name,entryIdx,exitIdx,DEFAULT_GATE_WIDTH)
	{
	}

	TrackSector::TrackSector(
		Track *track,
		std::string name,
		size_t entryIdx,
		size_t exitIdx,
		double gateWidth_meters)
	 : TrackPathObject(track,name)
	 , entryIdx_(entryIdx)
	 , exitIdx_(exitIdx)
	 , gateWidth_meters_(gateWidth_meters)
	{
	}

	TrackSector::~TrackSector()
	{
	}

	void
	TrackSector::setWidth(
		double width_meters)
	{
		gateWidth_meters_ = width_meters;
	}

	double
	TrackSector::getWidth() const
	{
		return gateWidth_meters_;
	}

	bool
	TrackSector::isSector() const
	{
		return true;
	}

	void
	TrackSector::setEntryIdx(
		size_t pathIdx)
	{
		entryIdx_ = pathIdx;
	}

	size_t
	TrackSector::getEntryIdx() const
	{
		return entryIdx_;
	}

	void
	TrackSector::setExitIdx(
		size_t pathIdx)
	{
		exitIdx_ = pathIdx;
	}

	size_t
	TrackSector::getExitIdx() const
	{
		return exitIdx_;
	}

	DetectionGate
	TrackSector::getEntryGate() const
	{
		return track_->getDetectionGate(entryIdx_,gateWidth_meters_);
	}

	DetectionGate
	TrackSector::getExitGate() const
	{
		return track_->getDetectionGate(exitIdx_,gateWidth_meters_);
	}

	// YAML encode/decode
	YAML::Node
	TrackSector::encode() const
	{
		YAML::Node node;
		node["name"] = name_;
		node["entryPathIdx"] = entryIdx_;
		node["exitPathIdx"] = exitIdx_;
		node["gateWidth_meters"] = gateWidth_meters_;

		return node;
	}

	bool
	TrackSector::decode(
		const YAML::Node& node)
	{
		name_ = node["name"].as<std::string>();
		entryIdx_ = node["entryPathIdx"].as<size_t>();
		exitIdx_ = node["exitPathIdx"].as<size_t>();
		gateWidth_meters_ = node["gateWidth_meters"].as<double>();

		return true;
	}

	TrackGate::TrackGate(
		Track *track,
		std::string name,
		size_t pathIdx,
		GateType_E type)
	 : TrackGate::TrackGate(track,name,pathIdx,type,DEFAULT_GATE_WIDTH)
	{
	}

	TrackGate::TrackGate(
		Track *track,
		std::string name,
		size_t pathIdx,
		GateType_E type,
		double gateWidth_meters)
	 : TrackPathObject(track,name)
	 , pathIdx_(pathIdx)
	 , type_(type)
	 , gateWidth_meters_(gateWidth_meters)
	{
	}

	TrackGate::~TrackGate()
	{
	}

	void
	TrackGate::setWidth(
		double width_meters)
	{
		gateWidth_meters_ = width_meters;
	}

	double
	TrackGate::getWidth() const
	{
		return gateWidth_meters_;
	}

	bool
	TrackGate::isGate() const
	{
		return true;
	}

	GateType_E
	TrackGate::getGateType() const
	{
		return type_;
	}

	void
	TrackGate::setPathIdx(
		size_t pathIdx)
	{
		pathIdx_ = pathIdx;
	}

	size_t
	TrackGate::getEntryIdx() const
	{
		return pathIdx_;
	}

	size_t
	TrackGate::getExitIdx() const
	{
		return pathIdx_;
	}

	DetectionGate
	TrackGate::getEntryGate() const
	{
		return track_->getDetectionGate(pathIdx_,gateWidth_meters_);
	}

	DetectionGate
	TrackGate::getExitGate() const
	{
		return track_->getDetectionGate(pathIdx_,gateWidth_meters_);
	}

	// YAML encode/decode
	YAML::Node
	TrackGate::encode() const
	{
		YAML::Node node;
		node["name"] = name_;
		node["pathIdx"] = pathIdx_;
		node["gateWidth_meters"] = gateWidth_meters_;

		return node;
	}

	bool
	TrackGate::decode(
		const YAML::Node& node)
	{
		name_ = node["name"].as<std::string>();
		pathIdx_ = node["pathIdx"].as<size_t>();
		gateWidth_meters_ = node["gateWidth_meters"].as<double>();

		return true;
	}

	Track::Track()
	 : Track(std::vector<cv::Vec2d>())
	{
	}

	Track::Track(
		const std::vector<cv::Vec2d> &path)
	 : ModifiableObject("Track")
	 , start_(new TrackGate(this, "startGate", 0, GateType_E::eGT_Start))
	 , finish_(new TrackGate(this, "finishGate", path.size() - 1, GateType_E::eGT_Finish))
	 , sectors_()
	 , path_(path)
	{
	}

	Track::~Track()
	{
		delete start_;
		delete finish_;
		for (auto sector : sectors_)
		{
			delete sector;
		}
	}

	// start/finish related methods
	void
	Track::setStart(
		size_t pathIdx)
	{
		start_->setPathIdx(pathIdx);
		markObjectModified();
	}

	const TrackGate *
	Track::getStart() const
	{
		return start_;
	}

	void
	Track::setFinish(
		size_t pathIdx)
	{
		finish_->setPathIdx(pathIdx);
		markObjectModified();
	}

	const TrackGate *
	Track::getFinish() const
	{
		return finish_;
	}

	// sector related methods
	std::pair<Track::RetCode, size_t>
	Track::findSectorInsertionIdx(
		size_t entryIdx,
		size_t exitIdx)
	{
		// check input arguments to make sure it's a valid gate
		if (entryIdx == exitIdx)
		{
			return {RetCode::E_SECTOR_NO_WIDTH,-1};
		}
		else if (exitIdx < entryIdx)
		{
			return {RetCode::E_EXIT_BEFORE_ENTRY,-1};
		}

		// handle corner cases where we have 0 or 1 sector
		int insertIdx = -1;
		if (sectors_.size() == 0)
		{
			insertIdx = 0;
		}
		else if (sectors_.size() == 1)
		{
			const auto &s = sectors_.at(0);
			if (entryIdx < s->getEntryIdx() && exitIdx <= s->getEntryIdx())
			{
				// insert sector before existing sector
				insertIdx = 0;
			}
			else if (s->getEntryIdx() < entryIdx && entryIdx < s->getExitIdx())
			{
				// entry gate falls within existing sector
				return {RetCode::E_OVERLAP,-1};
			}
			else if (s->getEntryIdx() < exitIdx && exitIdx < s->getExitIdx())
			{
				// exit gate falls within existing sector
				return {RetCode::E_OVERLAP,-1};
			}
			else if (entryIdx <= s->getEntryIdx() && s->getExitIdx() <= exitIdx)
			{
				// sector straddles the current sector
				return {RetCode::E_OVERLAP,-1};
			}
			else
			{
				// insert sector after existing sector.
				// we know this is safe because there's no sectors after this one
				insertIdx = 1;
			}
		}

		if (insertIdx >= 0)
		{
			// found insertion point based on corner cases. we're done!
			return {RetCode::SUCCESS,insertIdx};
		}
		
		// by now, sectors_.size() >= 2
		// find insertion point based on this criteria
		for (size_t i=0; i<sectors_.size(); i++)
		{
			const auto &s = sectors_.at(i);
			if (entryIdx < s->getEntryIdx() && exitIdx <= s->getEntryIdx())
			{
				// insert sector before current sector
				insertIdx = i;
				break;
			}
			else if (s->getEntryIdx() < entryIdx && entryIdx < s->getExitIdx())
			{
				// entry gate falls within existing sector
				return {RetCode::E_OVERLAP,-1};
			}
			else if (s->getEntryIdx() < exitIdx && exitIdx < s->getExitIdx())
			{
				// exit gate falls within existing sector
				return {RetCode::E_OVERLAP,-1};
			}
			else if (entryIdx <= s->getEntryIdx() && s->getExitIdx() <= exitIdx)
			{
				// sector straddles the current sector
				return {RetCode::E_OVERLAP,-1};
			}
			else if ((i + 1) == sectors_.size())
			{
				// at the end of the list.
				// sector is valid, then it must be inserted after the current.
				insertIdx = (i + 1);
				break;
			}
			else
			{
				// sector could land anywhere after the current one.
				// keep searching...
			}
		}

		// if we're here, then insertion point is valid
		return {RetCode::SUCCESS,insertIdx};
	}

	std::pair<Track::RetCode, size_t>
	Track::addSector(
		std::string name,
		size_t entryIdx,
		size_t exitIdx)
	{
		auto res = findSectorInsertionIdx(entryIdx,exitIdx);
		if (res.first == RetCode::SUCCESS)
		{
			sectors_.insert(
				std::next(sectors_.begin(),res.second),
				new TrackSector(this,name,entryIdx,exitIdx));
			markObjectModified();
		}
		return res;
	}

	void
	Track::removeSector(
		size_t idx)
	{
		sectors_.erase(std::next(sectors_.begin(), idx));
		markObjectModified();
	}

	void
	Track::setSectorName(
		size_t idx,
		std::string name)
	{
		sectors_.at(idx)->setName(name);
		markObjectModified();
	}

	const TrackSector *
	Track::getSector(
		size_t idx)
	{
		return sectors_.at(idx);
	}

	size_t
	Track::sectorCount() const
	{
		return sectors_.size();
	}

	size_t
	Track::pathCount() const
	{
		return path_.size();
	}

	cv::Vec2d
	Track::getPathPoint(
		size_t idx) const
	{
		return path_.at(idx);
	}

	DetectionGate
	Track::getDetectionGate(
		size_t pathIdx,
		double width_meters) const
	{
		// find two points before and after the pathIdx point.
		// we'll use the points to form a line and then find its normal.
		// the normal will be used to base the DetectionGate from.
		cv::Vec2d pA,pB;
		pA = pB = path_.at(pathIdx);
		if (pathIdx != 0)
		{
			pA = path_.at(pathIdx-1);
		}
		if ((pathIdx+1) < path_.size())
		{
			pB = path_.at(pathIdx+1);
		}

		double halfGateWidth_dd = m2dd(width_meters) / 2.0;
		double a,b;
		if (pA[0] == pB[0])
		{
			// latitudes equal; path is a horizontal line at pathIdx
			a = halfGateWidth_dd;
			b = 0.0;
		}
		else if (pA[1] == pB[1])
		{
			// longitudes equal; path is a vertical line at pathIdx
			a = 0.0;
			b = halfGateWidth_dd;
		}
		else
		{
			double pathSlope = (pB[0]-pA[0])/(pB[1]-pA[1]);// slope in lat/lon
			double normSlope = -1.0 / pathSlope;// if slope of line = m; its normal's slope = -1 / m

			// a^2 + b^2 = c^2
			// normSlope = a / b
			b = sqrt(
				(halfGateWidth_dd * halfGateWidth_dd) /
				(normSlope * normSlope + 1));
			a = normSlope * b;
		}

		auto gateCenter = path_.at(pathIdx);
		auto gateHalf = cv::Vec2d(a,b);
		return DetectionGate(
			gateCenter+gateHalf,
			gateCenter-gateHalf);
	}

	DetectionGate
	Track::getNearestDetectionGate(
		cv::Vec2d p,
		double width_meters) const
	{
		auto findRes = findClosestPointWithIdx(p);
		if (std::get<0>(findRes))
		{
			return getDetectionGate(std::get<2>(findRes),width_meters);
		}
		throw std::runtime_error("nearest detection gate not found!");
	}

	std::pair<bool,cv::Vec2d>
	Track::findClosestPoint(
		cv::Vec2d p) const
	{
		auto res = findClosestPointWithIdx(p);
		return std::pair(std::get<0>(res),std::get<1>(res));
	}

	std::tuple<bool,cv::Vec2d, size_t>
	Track::findClosestPointWithIdx(
		cv::Vec2d p) const
	{
		return findClosestPointWithIdx(p,0,{0,path_.size()});
	}

	std::tuple<bool,cv::Vec2d, size_t>
	Track::findClosestPointWithIdx(
		cv::Vec2d p,
		size_t initialIdx,
		std::pair<size_t,size_t> window) const
	{
		double closestDist = -1;
		cv::Vec2d closestPoint;
		size_t closestIdx;
		size_t startIdx = initialIdx - window.first;
		if (window.first > initialIdx)
		{
			startIdx = 0;
		}
		size_t endIdx = std::min(initialIdx + window.second,path_.size());
		for (size_t i=startIdx; i<endIdx; i++)
		{
			const auto &point = path_[i];
			double dist = cv::norm(p,point,cv::NORM_L2);
			if (closestDist < 0 || dist < closestDist)
			{
				closestDist = dist;
				closestPoint = point;
				closestIdx = i;
			}
		}
		return std::tuple(closestDist!=-1,closestPoint,closestIdx);
	}

	bool
	Track::getSortedPathObjects(
		std::vector<const TrackPathObject *> &objs) const
	{
		objs.clear();

		// sort objects by entry path index
		std::map<size_t,const TrackPathObject *> objsByEntryIdx;
		objsByEntryIdx.insert({start_->getEntryIdx(),start_});
		objsByEntryIdx.insert({finish_->getEntryIdx(),finish_});
		for (const auto &sector : sectors_)
		{
			objsByEntryIdx.insert({sector->getEntryIdx(),sector});
		}

		for (auto entry : objsByEntryIdx)
		{
			objs.push_back(entry.second);
		}

		return true;
	}

	// YAML encode/decode
	YAML::Node
	Track::encode() const
	{
		YAML::Node node;
		node["start"] = start_->encode();
		node["finish"] = finish_->encode();

		YAML::Node ySectors = node["sectors"];
		for (const auto &sector : sectors_)
		{
			ySectors.push_back(sector->encode());
		}

		YAML::Node yPath = node["path"];
		for (const auto &pathVec : path_)
		{
			yPath.push_back(pathVec);
		}

		return node;
	}

	bool
	Track::decode(
		const YAML::Node& node)
	{
		bool okay = true;
		okay = okay && start_->decode(node["start"]);
		okay = okay && finish_->decode(node["finish"]);

		sectors_.clear();
		if (node["sectors"])// not all files will have sectors
		{
			const YAML::Node &ySectors = node["sectors"];
			sectors_.resize(ySectors.size());
			for (size_t ss=0; okay && ss<sectors_.size(); ss++)
			{
				const YAML::Node &ySector = ySectors[ss];
				auto newSector = new TrackSector(this,"",0,0);
				if (newSector->decode(ySector))
				{
					sectors_.at(ss) = newSector;
				}
				else
				{
					delete newSector;
					sectors_.clear();
					okay = false;
				}
			}
		}

		const YAML::Node &yPath = node["path"];
		path_.resize(yPath.size());
		for (size_t pp=0; okay && pp<path_.size(); pp++)
		{
			const YAML::Node &yVec = yPath[pp];
			path_.at(pp) = yVec.as<cv::Vec2d>();
		}

		return okay;
	}

	//--------------------------------------------------------------
	// Track protected methods
	//--------------------------------------------------------------
	bool
	Track::subclassApplyModifications()
	{
		// apply means nothing to use right now
		return true;
	}

	bool
	Track::subclassSaveModifications()
	{
		auto path = getSavePath();
		try
		{
			std::ofstream ofs(path);
			YAML::Node node = encode();
			ofs << node;
			ofs.close();
		}
		catch (const std::exception &e)
		{
			spdlog::error(
				"caught std::exception while trying to save track to path '{}'. what() = {}",
				path.c_str(),
				e.what());
			return false;
		}
		return true;
	}

	Track *
	makeTrackFromTelemetry(
		TelemetrySourcePtr tSrc)
	{
		std::vector<cv::Vec2d> path;
		path.resize(tSrc->size());
		for (size_t i=0; i<path.size(); i++)
		{
			path[i][0] = tSrc->at(i).gpSamp.gps.coord.lat;
			path[i][1] = tSrc->at(i).gpSamp.gps.coord.lon;
		}
		return new Track(path);
	}

}