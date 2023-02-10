#include "GoProOverlay/data/VideoSource.h"
#include "GoProOverlay/data/DataSource.h"

namespace gpo
{
	VideoSource::VideoSource(
		DataSourcePtr dSrc)
	 : dataSrc_(dSrc)
	 , frameSize_()
	 , prevFrameIdxRead_(-1)
	{
		auto dataSrcPtr = dataSrc_.lock();
		frameSize_.width = dataSrcPtr->vCapture_.get(cv::CAP_PROP_FRAME_WIDTH);
		frameSize_.height = dataSrcPtr->vCapture_.get(cv::CAP_PROP_FRAME_HEIGHT);
	}

	std::string
	VideoSource::getDataSourceName() const
	{
		auto dataSrcPtr = dataSrc_.lock();
		if (dataSrcPtr)
		{
			return dataSrcPtr->getSourceName();
		}
		return "SOURCE_UNKNOWN";
	}

	int
	VideoSource::frameWidth() const
	{
		return frameSize_.width;
	}

	int
	VideoSource::frameHeight() const
	{
		return frameSize_.height;
	}

	cv::Size
	VideoSource::frameSize() const
	{
		return frameSize_;
	}

	double
	VideoSource::fps()
	{
		return dataSrc_.lock()->vCapture_.get(cv::CAP_PROP_FPS);
	}

	bool
	VideoSource::getFrame(
		cv::UMat &outImg,
		size_t idx)
	{
		auto dataSrcPtr = dataSrc_.lock();
		// seeking can be constly, so avoid it if reading consecutive frames
		if (idx != (prevFrameIdxRead_ + 1))
		{
			dataSrcPtr->vCapture_.set(cv::CAP_PROP_POS_FRAMES, idx);
		}
		prevFrameIdxRead_ = idx;
		return dataSrcPtr->vCapture_.read(outImg);
	}

	size_t
	VideoSource::seekedIdx() const
	{
		return dataSrc_.lock()->seeker->seekedIdx();
	}

	TelemetrySeekerPtr
	VideoSource::seeker()
	{
		return dataSrc_.lock()->seeker;
	}

	size_t
	VideoSource::frameCount()
	{
		return dataSrc_.lock()->vCapture_.get(cv::CAP_PROP_FRAME_COUNT);
	}
}