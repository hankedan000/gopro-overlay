#pragma once

#include <memory>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp> // for cv::Size

#include "TelemetrySeeker.h"

namespace gpo
{
	// forward declaration
	class DataSource;
	using DataSourcePtr = std::shared_ptr<DataSource>;

	class VideoSource
	{
	public:
		explicit
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
			cv::UMat &outImg,
			size_t idx);

		size_t
		seekedIdx() const;

		TelemetrySeekerPtr
		seeker();

		size_t
		frameCount();

	private:
		std::weak_ptr<DataSource> dataSrc_;
		cv::Size frameSize_;
		size_t prevFrameIdxRead_;

	};

	using VideoSourcePtr = std::shared_ptr<VideoSource>;
}