#pragma once

#include <GoProTelem/SampleTypes.h>
#include <opencv2/opencv.hpp>

namespace gpo
{
	class TrackMapObject
	{
	public:
		TrackMapObject(
			int width,
			int height);

		bool
		initMap(
			const std::vector<gpt::CombinedSample> &samples,
			int trackStartIdx = 0,
			int trackEndIdx = -1);

		const cv::Mat &
		getImage() const;

		void
		setLocation(
			const gpt::CoordLL &loc);

		void
		render(
			cv::Mat &intoImg,
			int originX, int originY,
			float scale = 1.0);

		int
		getWidth() const;

		int
		getHeight() const;

	private:
		cv::Point
		coordToPoint(
			const gpt::CoordLL &coord);

	private:
		const int PX_MARGIN = 20;

		// final rendered image
		cv::Mat outImg_;

		// map outline
		cv::Mat outlineImg_;

		// coordinates of upper-left and lower-right corners
		gpt::CoordLL ulCoord_;
		gpt::CoordLL lrCoord_;

		gpt::CoordLL currLocation_;

		double pxPerDeg_;

	};
}