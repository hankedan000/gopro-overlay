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
		int originX, int originY)
	{
		render(intoImg,originX,originY,outImg_.size());
	}

	void
	RenderedObject::render(
		cv::Mat &intoImg,
		int originX, int originY,
		cv::Size renderSize)
	{
		cv::Mat *imgToRender = &outImg_;
		if (renderSize.width != getNativeWidth() || renderSize.height != getNativeHeight())
		{
			ALPHA_SAFE_RESIZE(outImg_,scaledImg_,renderSize);
			imgToRender = &scaledImg_;
		}

		// draw final output to user image
		cv::Rect roi(cv::Point(originX,originY), imgToRender->size());
		cv::Mat3b destROI = intoImg(roi);
		double alpha = 1.0; // alpha in [0,1]
		for (int r = 0; r < destROI.rows; ++r)
		{
			for (int c = 0; c < destROI.cols; ++c)
			{
				auto vf = imgToRender->at<cv::Vec4b>(r,c);
				// Blending
				if (vf[3] > 0)
				{
					cv::Vec3b &vb = destROI(r,c);
					vb[0] = alpha * vf[0] + (1 - alpha) * vb[0];
					vb[1] = alpha * vf[1] + (1 - alpha) * vb[1];
					vb[2] = alpha * vf[2] + (1 - alpha) * vb[2];
				}
			}
		}

		if (boundingBoxVisible_)
		{
			cv::rectangle(intoImg,roi,CV_RGB(255,255,255));
		}
	}

	int
	RenderedObject::getNativeWidth() const
	{
		return outImg_.cols;
	}

	int
	RenderedObject::getNativeHeight() const
	{
		return outImg_.rows;
	}

	cv::Size
	RenderedObject::getNativeSize() const
	{
		return outImg_.size();
	}

	cv::Size
	RenderedObject::getScaledSizeFromTargetHeight(
		int targetHeight_px) const
	{
		double scale = (double)(targetHeight_px) / getNativeHeight();
		return cv::Size(getNativeWidth()*scale,getNativeHeight()*scale);
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