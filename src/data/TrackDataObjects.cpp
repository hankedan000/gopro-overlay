#include "GoProOverlay/data/TrackDataObjects.h"

#include <stdexcept>

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

	Sector::Sector(
		const std::string &name,
		const DetectionGate &entry,
		const DetectionGate &exit)
	 : name_(name)
	 , entry_(entry)
	 , exit_(exit)
	{
	}

	const std::string &
	Sector::name() const
	{
		return name_;
	}

	const DetectionGate &
	Sector::entry() const
	{
		return entry_;
	}
	
	const DetectionGate &
	Sector::exit() const
	{
		return exit_;
	}

	Track::Track()
	 : Track(std::vector<cv::Vec2d>())
	{
	}

	Track::Track(
		const std::vector<cv::Vec2d> &path)
	 : start_()
	 , finish_()
	 , sectors_()
	 , path_(path)
	{
	}

	void
	Track::setStart(
		const DetectionGate &start)
	{
		start_ = start;
	}

	const DetectionGate &
	Track::getStart() const
	{
		return start_;
	}

	void
	Track::setFinish(
		const DetectionGate &finish)
	{
		finish_ = finish;
	}

	const DetectionGate &
	Track::getFinish() const
	{
		return finish_;
	}

	void
	Track::addSector(
		const Sector &s)
	{
		sectors_.push_back(s);
	}

	bool
	Track::removeSector(
		Sector *s)
	{
		bool removed = false;
		for (auto itr=sectors_.begin(); itr!=sectors_.end(); itr++)
		{
			if (s == &(*itr))
			{
				sectors_.erase(itr);
				removed = true;
			}
		}
		return removed;
	}

	bool
	Track::removeSector(
		size_t idx)
	{
		bool removed = false;
		if (idx < sectors_.size())
		{
			sectors_.erase(sectors_.begin()+(idx-1));
			removed = true;
		}
		return removed;
	}

	Sector *
	Track::getSector(
		size_t idx)
	{
		if (idx < sectors_.size())
		{
			return &sectors_[idx];
		}
		return nullptr;
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
		size_t idx)
	{
		return path_.at(idx);
	}

	DetectionGate
	Track::getDetectionGate(
		size_t pathIdx,
		double width_meters)
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
			a = 0.0;halfGateWidth_dd;
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
		auto gateHalf = cv::Vec2d(-a,b);
		return DetectionGate(
			gateCenter+gateHalf,
			gateCenter-gateHalf);
	}

	DetectionGate
	Track::getNearestDetectionGate(
		cv::Vec2d p,
		double width_meters)
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
		cv::Vec2d p)
	{
		auto res = findClosestPointWithIdx(p);
		return std::pair(std::get<0>(res),std::get<1>(res));
	}

	std::tuple<bool,cv::Vec2d, size_t>
	Track::findClosestPointWithIdx(
		cv::Vec2d p)
	{
		double closestDist = -1;
		cv::Vec2d closestPoint;
		size_t closestIdx;
		for (size_t i=0; i<path_.size(); i++)
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

	Track
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
		return Track(path);
	}

}