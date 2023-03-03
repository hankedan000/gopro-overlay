#include "GoProOverlay/graphics/TrackMapObject.h"

namespace gpo
{
	const int TRACK_MAP_RENDER_WIDTH = 600;
	const int TRACK_MAP_RENDER_HEIGHT = 600;

	const size_t NUM_DEFAULT_DOT_COLORS = 3;
	const cv::Scalar DEFAULT_DOT_COLORS[] = {
		RGBA_COLOR(255,0,0,255),
		RGBA_COLOR(0,255,0,255),
		RGBA_COLOR(0,0,255,255)
	};

	TrackMapObject::TrackMapObject()
	 : RenderedObject("TrackMapObject",TRACK_MAP_RENDER_WIDTH,TRACK_MAP_RENDER_HEIGHT)
	 , outlineImg_(TRACK_MAP_RENDER_HEIGHT,TRACK_MAP_RENDER_WIDTH,CV_8UC4,RGBA_COLOR(0,0,0,0))
	 , ulCoord_()
	 , lrCoord_()
	 , pxPerDeg_(1.0)
	 , trackThickness_px_(DEFAULT_TRACK_THICKNESS_RATIO * TRACK_MAP_RENDER_WIDTH)
	 , dotRadius_px_(DEFAULT_DOT_RADIUS_RATIO * TRACK_MAP_RENDER_WIDTH)
	 , dotColors_()
	{
	}

	DataSourceRequirements
	TrackMapObject::dataSourceRequirements() const
	{
		return DataSourceRequirements(0,DSR_ONE_OR_MORE,1);
	}

	void
	TrackMapObject::setDotColor(
		size_t sourceIdx,
		cv::Scalar color)
	{
		dotColors_.at(sourceIdx) = color;
		markNeedsRedraw();
		markObjectModified();
	}

	void
	TrackMapObject::sourcesValid()
	{
		// init dot colors based on number of sources
		if (dotColors_.size() > tSources_.size())
		{
			// remove excess dot colors
			dotColors_.resize(tSources_.size());
		}
		else
		{
			size_t newColorsNeeded = tSources_.size() - dotColors_.size();
			for (size_t i=0; i<newColorsNeeded; i++)
			{
				dotColors_.push_back(DEFAULT_DOT_COLORS[dotColors_.size()%NUM_DEFAULT_DOT_COLORS]);
			}
		}

		// use first source as the basis for the track map outline
		auto telemSrc = tSources_.front();

		size_t trackStartIdx = track_->getStart()->getEntryIdx();
		size_t trackEndIdx = track_->getFinish()->getEntryIdx();

		ulCoord_.lat = -10000;
		lrCoord_.lat = +10000;
		ulCoord_.lon = +10000;
		lrCoord_.lon = -10000;
		for (size_t i=trackStartIdx; i<trackEndIdx; i++)
		{
			const auto &coord = track_->getPathPoint(i);
			if (coord[0] > ulCoord_.lat)
			{
				ulCoord_.lat = coord[0];
			}
			if (coord[1] < ulCoord_.lon)
			{
				ulCoord_.lon = coord[1];
			}
			if (coord[0] < lrCoord_.lat)
			{
				lrCoord_.lat = coord[0];
			}
			if (coord[1] > lrCoord_.lon)
			{
				lrCoord_.lon = coord[1];
			}
		}

		double deltaLat = ulCoord_.lat - lrCoord_.lat;
		double deltaLon = lrCoord_.lon - ulCoord_.lon;
		pxPerDeg_ = 1.0;
		if (deltaLat > deltaLon)
		{
			// track is taller than it is wide
			pxPerDeg_ = (getNativeHeight() - PX_MARGIN * 2) / deltaLat;
		}
		else
		{
			// track is wider than it is tall
			pxPerDeg_ = (getNativeWidth() - PX_MARGIN * 2) / deltaLon;
		}

		const bool drawBackground = false;
		if (drawBackground)
		{
			int bgWidth = deltaLon * pxPerDeg_ + PX_MARGIN * 2;
			int bgHeight = deltaLat * pxPerDeg_ + PX_MARGIN * 2;
			cv::rectangle(
				outlineImg_,
				cv::Point(0,0),
				cv::Point(bgWidth,bgHeight),
				RGBA_COLOR(0,0,0,100),
				cv::FILLED);
		}

		cv::Point prevPoint;
		for (size_t i=trackStartIdx; i<trackEndIdx; i++)
		{
			const auto &coord = track_->getPathPoint(i);
			auto currPoint = coordToPoint({coord[0],coord[1]});

			if (i != trackStartIdx)
			{
				cv::line(
					outlineImg_,
					prevPoint,
					currPoint,
					RGBA_COLOR(255,255,255,255),
					trackThickness_px_,
					cv::LINE_4);
			}

			prevPoint = currPoint;
		}
	}

	cv::Point
	TrackMapObject::coordToPoint(
		const gpt::CoordLL &coord)
	{
		return cv::Point(
			PX_MARGIN + (coord.lon - ulCoord_.lon) * pxPerDeg_,
			PX_MARGIN + (coord.lat - ulCoord_.lat) * -pxPerDeg_);
	}

	void
	TrackMapObject::subRender()
	{
		outlineImg_.copyTo(outImg_);
		if ( ! requirementsMet())
		{
			return;
		}

		for (unsigned int ss=0; ss<tSources_.size(); ss++)
		{
			auto telemSrc = tSources_.at(ss);

			const auto &currSample = telemSrc->at(telemSrc->seekedIdx());
			auto dotPoint = coordToPoint(currSample.trackData.onTrackLL);
			cv::circle(outImg_,dotPoint,dotRadius_px_,dotColors_.at(ss),cv::FILLED);
		}
	}

	YAML::Node
	TrackMapObject::subEncode() const
	{
		YAML::Node node;
		node["trackThickness_px"] = trackThickness_px_;
		node["dotRadius_px"] = dotRadius_px_;

		YAML::Node dotColorsNode;
		for (auto color : dotColors_)
		{
			dotColorsNode.push_back(color);
		}
		node["dotColors"] = dotColorsNode;

		return node;
	}

	bool
	TrackMapObject::subDecode(
		const YAML::Node& node)
	{
		YAML_TO_FIELD(node,"trackThickness_px",trackThickness_px_);
		YAML_TO_FIELD(node,"dotRadius_px",dotRadius_px_);

		if (node["dotColors"])
		{
			const auto &dotColorsNode = node["dotColors"];
			dotColors_.clear();
			dotColors_.reserve(dotColorsNode.size());
			for (size_t dd=0; dd<dotColorsNode.size(); dd++)
			{
				dotColors_.push_back(dotColorsNode[dd].as<cv::Scalar>());
			}
		}

		return true;
	}

}
