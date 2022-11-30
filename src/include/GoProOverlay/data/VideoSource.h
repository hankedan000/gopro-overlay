#pragma once

#include <memory>
#include <opencv2/opencv.hpp>

#include "TelemetrySample.h"
#include "TelemetrySeeker.h"

namespace gpo
{
	// forward declaration
	class DataSource;
	using DataSourcePtr = std::shared_ptr<DataSource>;

	class VideoSource
	{
	public:
		VideoSource(
			DataSourcePtr dSrc);

		std::string
		getDataSourceName() const;

		int
		frameWidth() const;

		int
		frameHeight() const;

		cv::Size
		frameSize() const;

		double
		fps();

		bool
		getFrame(
			cv::Mat &outImg,
			size_t idx);

		size_t
		seekedIdx() const;

		TelemetrySeekerPtr
		seeker();

		size_t
		frameCount();

	private:
		DataSourcePtr dataSrc_;
		cv::Size frameSize_;
		size_t prevFrameIdxRead_;

	};

	using VideoSourcePtr = std::shared_ptr<VideoSource>;
}