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
		frameSize_.width = dataSrc_->vCapture_.get(cv::CAP_PROP_FRAME_WIDTH);
		frameSize_.height = dataSrc_->vCapture_.get(cv::CAP_PROP_FRAME_HEIGHT);
	}

	std::string
	VideoSource::getDataSourceName() const
	{
		if (dataSrc_)
		{
			return dataSrc_->getSourceName();
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
		return dataSrc_->vCapture_.get(cv::CAP_PROP_FPS);
	}

	bool
	VideoSource::getFrame(
		cv::Mat &outImg,
		size_t idx)
	{
		// seeking can be constly, so avoid it if reading consecutive frames
		if (idx != (prevFrameIdxRead_ + 1))
		{
			dataSrc_->vCapture_.set(cv::CAP_PROP_POS_FRAMES, idx);
		}
		prevFrameIdxRead_ = idx;
		return dataSrc_->vCapture_.read(outImg);
	}

	size_t
	VideoSource::seekedIdx() const
	{
		return dataSrc_->seeker->seekedIdx();
	}

	TelemetrySeekerPtr
	VideoSource::seeker()
	{
		return dataSrc_->seeker;
	}

	size_t
	VideoSource::frameCount()
	{
		return dataSrc_->vCapture_.get(cv::CAP_PROP_FRAME_COUNT);
	}
}