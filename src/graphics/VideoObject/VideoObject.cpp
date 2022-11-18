#include "VideoObject.h"

namespace gpo
{
	VideoObject::VideoObject(
		const VideoSourcePtr &vSrc)
	 : RenderedObject(0,0)
	 , source_(vSrc)
	{
		outImg_.create(source_->frameSize(),CV_8UC3);
	}

	void
	VideoObject::render(
		cv::Mat &intoImg,
		int originX, int originY,
		cv::Size renderSize)
	{
		auto frameIdx = source_->seekedIdx();
		source_->getFrame(outImg_,frameIdx);

		cv::Mat *imgToRender = &outImg_;
		if (renderSize.width != getNativeWidth() || renderSize.height != getNativeHeight())
		{
			cv::resize(outImg_,resizedFrame_,renderSize);
			imgToRender = &resizedFrame_;
		}

		// draw final output to user image
		cv::Rect roi(cv::Point(originX,originY), imgToRender->size());
		cv::Mat destROI = intoImg(roi);
		imgToRender->copyTo(destROI);

		if (boundingBoxVisible_)
		{
			cv::rectangle(intoImg,roi,CV_RGB(255,255,255));
		}
	}
}