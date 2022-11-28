#include "GoProOverlay/graphics/VideoObject.h"

namespace gpo
{
	VideoObject::VideoObject(
		const VideoSourcePtr &vSrc)
	 : RenderedObject(0,0)
	 , source_(vSrc)
	 , prevRenderedFrameIdx_(-1)
	{
		outImg_.create(source_->frameSize(),CV_8UC3);
	}

	std::string
	VideoObject::typeName() const
	{
		return "VideoObject";
	}

	DataSourceRequirements
	VideoObject::dataSourceRequirements() const
	{
		return DataSourceRequirements(1,0,0);
	}

	void
	VideoObject::render(
		cv::Mat &intoImg,
		int originX, int originY,
		cv::Size renderSize)
	{
		auto frameIdx = source_->seekedIdx();
		bool needNewFrame = frameIdx != prevRenderedFrameIdx_;
		if (needNewFrame && ! source_->getFrame(outImg_,frameIdx))
		{
			throw std::runtime_error("getFrame() failed on frameIdx " + std::to_string(frameIdx));
		}

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
		prevRenderedFrameIdx_ = frameIdx;
	}

	YAML::Node
	VideoObject::subEncode() const
	{
		YAML::Node node;
		return node;
	}

	bool
	VideoObject::subDecode(
		const YAML::Node& node)
	{
		return true;
	}

}