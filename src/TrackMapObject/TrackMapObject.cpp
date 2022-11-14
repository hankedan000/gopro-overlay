#include "TrackMapObject.h"

namespace gpo
{
	TrackMapObject::TrackMapObject(
		int width,
		int height)
	 : outImg_(height,width,CV_8UC4,cv::Scalar(0,0,0,0))
	 , outlineImg_(height,width,CV_8UC4,cv::Scalar(0,0,0,0))
	 , ulCoord_()
	 , lrCoord_()
	 , pxPerDeg_(1.0)
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
			pxPerDeg_ = (getHeight() - PX_MARGIN * 2) / deltaLat;
		}
		else
		{
			// track is wider than it is tall
			pxPerDeg_ = (getWidth() - PX_MARGIN * 2) / deltaLon;
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
					cv::Scalar(255,255,255,255),
					2,
					cv::LINE_4);
			}

			prevPoint = currPoint;
		}
		cv::imwrite("outlineImg_.png",outlineImg_);

		return true;
	}

	const cv::Mat &
	TrackMapObject::getImage() const
	{
		return outImg_;
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
		cv::circle(outImg_,dotPoint,5,cv::Scalar(255,0,0,255),cv::FILLED);

		// draw final output to user image
		cv::Rect rect(cv::Point(originX,originY), outImg_.size());
		cv::Mat3b roi = intoImg(rect);
		double alpha = 1.0; // alpha in [0,1]
		for (int r = 0; r < roi.rows; ++r)
		{
			for (int c = 0; c < roi.cols; ++c)
			{
				auto vf = outImg_.at<cv::Vec4b>(r,c);
				// Blending
				if (vf[3] > 0)
				{
					cv::Vec3b &vb = roi(r,c);// GBR
					vb[2] = alpha * vf[0] + (1 - alpha) * vb[2];
					vb[0] = alpha * vf[1] + (1 - alpha) * vb[0];
					vb[1] = alpha * vf[2] + (1 - alpha) * vb[1];
				}
			}
		}
	}

	int
	TrackMapObject::getWidth() const
	{
		return outImg_.cols;
	}

	int
	TrackMapObject::getHeight() const
	{
		return outImg_.rows;
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