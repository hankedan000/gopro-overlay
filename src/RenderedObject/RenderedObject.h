#pragma once

#include <opencv2/opencv.hpp>

namespace gpo
{
	class RenderedObject
	{
	public:
		RenderedObject(
			int width,
			int height);

		const cv::Mat &
		getImage() const;

		virtual
		void
		render(
			cv::Mat &intoImg,
			int originX, int originY,
			float scale = 1.0);

		int
		getWidth() const;

		int
		getHeight() const;

	protected:
		// final rendered image
		cv::Mat outImg_;

	};
}