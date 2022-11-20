#pragma once

#include <opencv2/opencv.hpp>

namespace gpo
{
	#define RGBA_COLOR(R,G,B,A) cv::Scalar(B,G,R,A)

	// use INTER_NEAREST because images that contain alpha channels tend
	// to get distortion around edges with default of INTER_LINEAR
	#define ALPHA_SAFE_RESIZE(SRC_IMG,DST_IMG,DST_SIZE) \
		cv::resize(SRC_IMG,DST_IMG,DST_SIZE,0,0,cv::INTER_NEAREST)

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
			int originX, int originY);

		virtual
		void
		render(
			cv::Mat &intoImg,
			int originX, int originY,
			cv::Size renderSize);

		int
		getNativeWidth() const;

		int
		getNativeHeight() const;

		cv::Size
		getNativeSize() const;

		cv::Size
		getScaledSizeFromTargetHeight(
			int targetHeight) const;

		void
		setVisible(
			bool visible);

		bool
		isVisible() const;

		void
		setBoundingBoxVisible(
			bool visible);

		bool
		isBoundingBoxVisible() const;

	protected:
		// final rendered image
		cv::Mat outImg_;

		bool visible_;
		bool boundingBoxVisible_;

	private:
		cv::Mat scaledImg_;

	};
}