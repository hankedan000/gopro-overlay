#include "GoProOverlay/graphics/VideoObject.h"

namespace gpo
{
	VideoObject::VideoObject()
	 : RenderedObject(0,0)
	 , prevRenderedFrameIdx_(-1)
	{
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
	VideoObject::render()
	{
		if ( ! requirementsMet())
		{
			return;
		}
		auto vSource = vSources_.front();

		auto frameIdx = vSource->seekedIdx();
		bool needNewFrame = frameIdx != prevRenderedFrameIdx_;
		if (needNewFrame && ! vSource->getFrame(outImg_,frameIdx))
		{
			throw std::runtime_error("getFrame() failed on frameIdx " + std::to_string(frameIdx));
		}
		prevRenderedFrameIdx_ = frameIdx;
	}

	void
    VideoObject::drawInto(
		cv::UMat &intoImg,
		int originX, int originY,
		cv::Size renderSize)
	{
		cv::UMat *imgToRender = &outImg_;
		if (renderSize.width != getNativeWidth() || renderSize.height != getNativeHeight())
		{
			cv::resize(outImg_,resizedFrame_,renderSize);
			imgToRender = &resizedFrame_;
		}

		// draw final output to user image
		cv::Rect roi(cv::Point(originX,originY), imgToRender->size());
		cv::UMat destROI = intoImg(roi);
		imgToRender->copyTo(destROI);

		if (boundingBoxVisible_)
		{
			cv::rectangle(intoImg,roi,CV_RGB(255,255,255),boundingBoxThickness_);
		}
	}

	void
	VideoObject::sourcesValid()
	{
		outImg_.create(vSources_.front()->frameSize(),CV_8UC3);
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