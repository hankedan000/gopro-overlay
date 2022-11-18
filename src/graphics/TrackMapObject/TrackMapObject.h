#pragma once

#include <GoProTelem/SampleTypes.h>

#include "TelemetryObject.h"

namespace gpo
{
	class TrackMapObject : public TelemetryObject
	{
	public:
		TrackMapObject();

		bool
		initMap(
			int trackStartIdx = 0,
			int trackEndIdx = -1);

		virtual
		void
		render(
			cv::Mat &intoImg,
			int originX, int originY,
			cv::Size renderSize);

	private:
		cv::Point
		coordToPoint(
			const gpt::CoordLL &coord);

	private:
		const int PX_MARGIN = 20;

		// map outline
		cv::Mat outlineImg_;

		// coordinates of upper-left and lower-right corners
		gpt::CoordLL ulCoord_;
		gpt::CoordLL lrCoord_;

		double pxPerDeg_;

		const double DEFAULT_TRACK_THICKNESS_RATIO = 2.0 / 300.0;// 'line thickness' over 'rendered trackmap width'
		int trackThickness_px_;

		const double DEFAULT_DOT_RADIUS_RATIO = 5.0 / 300.0;// 'dot radius' over 'rendered trackmap width'
		int dotRadius_px_;

	};
}