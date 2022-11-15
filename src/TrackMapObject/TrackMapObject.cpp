#include "TrackMapObject.h"

namespace gpo
{
	const int TRACK_MAP_RENDER_WIDTH = 600;
	const int TRACK_MAP_RENDER_HEIGHT = 600;

	TrackMapObject::TrackMapObject()
	 : RenderedObject(TRACK_MAP_RENDER_WIDTH,TRACK_MAP_RENDER_HEIGHT)
	 , outlineImg_(TRACK_MAP_RENDER_HEIGHT,TRACK_MAP_RENDER_WIDTH,CV_8UC4,RGBA_COLOR(0,0,0,0))
	 , ulCoord_()
	 , lrCoord_()
	 , pxPerDeg_(1.0)
	 , trackThickness_px_(DEFAULT_TRACK_THICKNESS_RATIO * TRACK_MAP_RENDER_WIDTH)
	 , dotRadius_px_(DEFAULT_DOT_RADIUS_RATIO * TRACK_MAP_RENDER_WIDTH)
	{
	}

	bool
	TrackMapObject::initMap(
		const std::vector<gpt::CombinedSample> &samples,
		int trackStartIdx,
		int trackEndIdx)
	{
		if (samples.empty())
		{
			return true;
		}

		if (trackStartIdx < 0)
		{
			trackStartIdx = 0;
		}
		if (trackEndIdx < 0)
		{
			trackEndIdx = samples.size();
		}

		ulCoord_.lat = -10000;
		lrCoord_.lat = +10000;
		ulCoord_.lon = +10000;
		lrCoord_.lon = -10000;
		for (size_t i=trackStartIdx; i<trackEndIdx; i++)
		{
			auto &samp = samples.at(i);
			if (samp.gps.coord.lat > ulCoord_.lat)
			{
				ulCoord_.lat = samp.gps.coord.lat;
			}
			if (samp.gps.coord.lon < ulCoord_.lon)
			{
				ulCoord_.lon = samp.gps.coord.lon;
			}
			if (samp.gps.coord.lat < lrCoord_.lat)
			{
				lrCoord_.lat = samp.gps.coord.lat;
			}
			if (samp.gps.coord.lon > lrCoord_.lon)
			{
				lrCoord_.lon = samp.gps.coord.lon;
			}
		}

		double deltaLat = ulCoord_.lat - lrCoord_.lat;
		double deltaLon = lrCoord_.lon - ulCoord_.lon;
		pxPerDeg_ = 1.0;
		if (deltaLat > deltaLon)
		{
			// track is taller than it is wide
			pxPerDeg_ = (getRenderedHeight() - PX_MARGIN * 2) / deltaLat;
		}
		else
		{
			// track is wider than it is tall
			pxPerDeg_ = (getRenderedWidth() - PX_MARGIN * 2) / deltaLon;
		}

		cv::Point prevPoint;
		for (size_t i=trackStartIdx; i<trackEndIdx; i++)
		{
			auto &samp = samples.at(i);
			auto currPoint = coordToPoint(samp.gps.coord);

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

		return true;
	}

	void
	TrackMapObject::setLocation(
		const gpt::CoordLL &loc)
	{
		currLocation_ = loc;
	}

	void
	TrackMapObject::render(
		cv::Mat &intoImg,
		int originX, int originY,
		float scale)
	{
		outlineImg_.copyTo(outImg_);

		auto dotPoint = coordToPoint(currLocation_);
		cv::circle(outImg_,dotPoint,dotRadius_px_,RGBA_COLOR(255,0,0,255),cv::FILLED);

		// render result into final image
		RenderedObject::render(intoImg,originX,originY,scale);
	}

	cv::Point
	TrackMapObject::coordToPoint(
		const gpt::CoordLL &coord)
	{
		return cv::Point(
			PX_MARGIN + (coord.lon - ulCoord_.lon) * pxPerDeg_,
			PX_MARGIN + (coord.lat - ulCoord_.lat) * -pxPerDeg_);
	}

}