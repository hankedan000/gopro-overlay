#include "GoProOverlay/data/VideoSource.h"

namespace gpo
{
	VideoSource::VideoSource(
		const cv::VideoCapture &capture,
		TelemetrySeekerPtr seeker)
	 : vCapture_(capture)
	 , seeker_(seeker)
	 , frameSize_()
	 , prevFrameIdxRead_(-1)
	{
		frameSize_.width = vCapture_.get(cv::CAP_PROP_FRAME_WIDTH);
		frameSize_.height = vCapture_.get(cv::CAP_PROP_FRAME_HEIGHT);
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
		return vCapture_.get(cv::CAP_PROP_FPS);
	}

	bool
	VideoSource::getFrame(
		cv::Mat &outImg,
		size_t idx)
	{
		// seeking can be constly, so avoid it if reading consecutive frames
		if (idx != (prevFrameIdxRead_ + 1))
		{
			vCapture_.set(cv::CAP_PROP_POS_FRAMES, idx);
		}
		prevFrameIdxRead_ = idx;
		return vCapture_.read(outImg);
	}

	size_t
	VideoSource::seekedIdx() const
	{
		return seeker_->seekedIdx();
	}

	size_t
	VideoSource::frameCount()
	{
		return vCapture_.get(cv::CAP_PROP_FRAME_COUNT);
	}
}