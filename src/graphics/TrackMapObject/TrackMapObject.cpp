#include "TrackMapObject.h"

namespace gpo
{
	const int TRACK_MAP_RENDER_WIDTH = 600;
	const int TRACK_MAP_RENDER_HEIGHT = 600;

	TrackMapObject::TrackMapObject()
	 : TelemetryObject(TRACK_MAP_RENDER_WIDTH,TRACK_MAP_RENDER_HEIGHT)
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
		int trackStartIdx,
		int trackEndIdx)
	{
		if (sources_.empty())
		{
			return true;
		}

		// use first source as the basis for the track map outline
		auto telemSrc = sources_.front();

		if (trackStartIdx < 0)
		{
			trackStartIdx = 0;
		}
		if (trackEndIdx < 0)
		{
			trackEndIdx = telemSrc->size();
		}

		ulCoord_.lat = -10000;
		lrCoord_.lat = +10000;
		ulCoord_.lon = +10000;
		lrCoord_.lon = -10000;
		for (size_t i=trackStartIdx; i<trackEndIdx; i++)
		{
			auto &samp = telemSrc->at(i).gpSamp;
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
			pxPerDeg_ = (getNativeHeight() - PX_MARGIN * 2) / deltaLat;
		}
		else
		{
			// track is wider than it is tall
			pxPerDeg_ = (getNativeWidth() - PX_MARGIN * 2) / deltaLon;
		}

		cv::Point prevPoint;
		for (size_t i=trackStartIdx; i<trackEndIdx; i++)
		{
			auto &samp = telemSrc->at(i).gpSamp;
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
	TrackMapObject::render(
		cv::Mat &intoImg,
		int originX, int originY,
		cv::Size renderSize)
	{
		outlineImg_.copyTo(outImg_);

		if (sources_.empty())
		{
			return;
		}

		const size_t NUM_DOT_COLORS = 3;
		const cv::Scalar DOT_COLORS[] = {
			RGBA_COLOR(255,0,0,255),
			RGBA_COLOR(0,255,0,255),
			RGBA_COLOR(0,0,255,255)
		};
		for (unsigned int ss=0; ss<sources_.size(); ss++)
		{
			auto telemSrc = sources_.at(ss);

			const auto &currSample = telemSrc->at(telemSrc->seekedIdx()).gpSamp;
			auto dotPoint = coordToPoint(currSample.gps.coord);
			auto dotColor = DOT_COLORS[ss%NUM_DOT_COLORS];
			cv::circle(outImg_,dotPoint,dotRadius_px_,dotColor,cv::FILLED);
		}

		// render result into final image
		TelemetryObject::render(intoImg,originX,originY,renderSize);
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