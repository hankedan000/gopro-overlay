#pragma once

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

	};
}