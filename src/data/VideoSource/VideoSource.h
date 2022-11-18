#pragma once

#include <memory>
#include <opencv2/opencv.hpp>

#include "TelemetrySample.h"
#include "TelemetrySeeker.h"

namespace gpo
{
	class VideoSource
	{
	public:
		VideoSource(
			const cv::VideoCapture &capture,
			TelemetrySeekerPtr seeker);

		int
		frameWidth() const;

		int
		frameHeight() const;

		cv::Size
		frameSize() const;

		bool
		getFrame(
			cv::Mat &outImg,
			size_t idx);

		size_t
		seekedIdx() const;

		size_t
		frameCount();

	private:
		cv::VideoCapture vCapture_;
		TelemetrySeekerPtr seeker_;
		cv::Size frameSize_;

	};

	using VideoSourcePtr = std::shared_ptr<VideoSource>;
}