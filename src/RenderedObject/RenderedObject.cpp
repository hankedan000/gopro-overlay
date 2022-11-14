#include "RenderedObject.h"

namespace gpo
{
	RenderedObject::RenderedObject(
		int width,
		int height)
	 : outImg_(height,width,CV_8UC4,cv::Scalar(0,0,0,0))
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
					cv::Vec3b &vb = roi(r,c);// GBR
					vb[2] = alpha * vf[0] + (1 - alpha) * vb[2];
					vb[0] = alpha * vf[1] + (1 - alpha) * vb[0];
					vb[1] = alpha * vf[2] + (1 - alpha) * vb[1];
				}
			}
		}
	}

	int
	RenderedObject::getWidth() const
	{
		return outImg_.cols;
	}

	int
	RenderedObject::getHeight() const
	{
		return outImg_.rows;
	}

}