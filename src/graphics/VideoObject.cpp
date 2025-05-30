#include "GoProOverlay/graphics/VideoObject.h"

#include <opencv2/imgproc.hpp>
#include <tracy/Tracy.hpp>

namespace gpo
{
	VideoObject::VideoObject()
	 : RenderedObject("VideoObject",1,1)// gets resized in sourcesValid()
	 , prevRenderedFrameIdx_(-1)
	{
	}

	DataSourceRequirements
	VideoObject::dataSourceRequirements() const
	{
		return DataSourceRequirements(1,0,0);
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

	void
	VideoObject::subRender()
	{
		ZoneScopedN("VideoObject::subRender()");
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

	YAML::Node
	VideoObject::subEncode() const
	{
		YAML::Node node;
		return node;
	}

	bool
	VideoObject::subDecode(
		const YAML::Node & /* node */)
	{
		// nothing was encoded, so there's nothing to decode
		return true;
	}

}