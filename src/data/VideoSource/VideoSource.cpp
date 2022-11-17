#include "VideoSource.h"

namespace gpo
{
	VideoSource::VideoSource(
		const cv::VideoCapture &capture,
		TelemetrySeekerPtr seeker)
	 : vCapture_(capture)
	 , seeker_(seeker)
	{
	}

	bool
	VideoSource::getFrame(
		cv::Mat &outImg,
		size_t idx)
	{
		vCapture_.set(cv::CAP_PROP_POS_FRAMES, idx);
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