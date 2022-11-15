#include "RenderedObject.h"

namespace gpo
{
	RenderedObject::RenderedObject(
		int width,
		int height)
	 : outImg_(height,width,CV_8UC4,cv::Scalar(0,0,0,0))
	 , visible_(true)
	 , boundingBoxVisible_(false)
	{
	}

	const cv::Mat &
	RenderedObject::getImage() const
	{
		return outImg_;
	}

	void
	RenderedObject::render(
		cv::Mat &intoImg,
		int originX, int originY,
		float scale)
	{
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
					cv::Vec3b &vb = roi(r,c);
					vb[0] = alpha * vf[0] + (1 - alpha) * vb[0];
					vb[1] = alpha * vf[1] + (1 - alpha) * vb[1];
					vb[2] = alpha * vf[2] + (1 - alpha) * vb[2];
				}
			}
		}

		if (boundingBoxVisible_)
		{
			cv::rectangle(intoImg,rect,CV_RGB(255,255,255));
		}
	}

	int
	RenderedObject::getRenderedWidth() const
	{
		return outImg_.cols;
	}

	int
	RenderedObject::getRenderedHeight() const
	{
		return outImg_.rows;
	}

	void
	RenderedObject::setVisible(
		bool visible)
	{
		visible_ = visible;
	}

	bool
	RenderedObject::isVisible() const
	{
		return visible_;
	}

	void
	RenderedObject::setBoundingBoxVisible(
		bool visible)
	{
		boundingBoxVisible_ = visible;
	}

	bool
	RenderedObject::isBoundingBoxVisible() const
	{
		return boundingBoxVisible_;
	}

}