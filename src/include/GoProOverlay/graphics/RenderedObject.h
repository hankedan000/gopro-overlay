#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

#include "GoProOverlay/data/DataSource.h"
#include "GoProOverlay/data/VideoSource.h"
#include "GoProOverlay/data/TelemetrySource.h"
#include "GoProOverlay/data/TrackDataObjects.h"

namespace gpo
{
	#define RGBA_COLOR(R,G,B,A) cv::Scalar(B,G,R,A)

	// use INTER_NEAREST because images that contain alpha channels tend
	// to get distortion around edges with default of INTER_LINEAR
	#define ALPHA_SAFE_RESIZE(SRC_IMG,DST_IMG,DST_SIZE) \
		cv::resize(SRC_IMG,DST_IMG,DST_SIZE,0,0,cv::INTER_NEAREST)

	const int DSR_ONE_OR_MORE = -1;
	const int DSR_ZERO_OR_MORE = -2;

	struct DataSourceRequirements
	{
	public:
		DataSourceRequirements()
		 : DataSourceRequirements(0,0,0)
		{}

		DataSourceRequirements(int nvs, int nts, int nt)
		 : numVideoSources(nvs)
		 , numTelemetrySources(nts)
		 , numTracks(nt)
		{}

		unsigned int
		minVideos() const
		{
			return minFromDSR(numVideoSources);
		}

		unsigned int
		minTelemetry() const
		{
			return minFromDSR(numTelemetrySources);
		}

		unsigned int
		minTracks() const
		{
			return minFromDSR(numTracks);
		}

		static
		unsigned int
		minFromDSR(
			int count)
		{
			if (count == DSR_ZERO_OR_MORE)
				return 0;
			else if (count == DSR_ONE_OR_MORE)
				return 1;
			return count;
		}

		// these can also be set to 'DSR_*' constants
		int numVideoSources;
		int numTelemetrySources;
		int numTracks;
	};

	class RenderedObject
	{
	public:
		RenderedObject(
			int width,
			int height);

		virtual
		std::string
		typeName() const = 0;

		const cv::Mat &
		getImage() const;

		virtual
		void
		render() = 0;

		virtual
		void
		drawInto(
			cv::Mat &intoImg,
			int originX, int originY);

		virtual
		void
		drawInto(
			cv::Mat &intoImg,
			int originX, int originY,
			cv::Size renderSize);

		int
		getNativeWidth() const;

		int
		getNativeHeight() const;

		cv::Size
		getNativeSize() const;

		cv::Size
		getScaledSizeFromTargetHeight(
			int targetHeight) const;

		void
		setVisible(
			bool visible);

		bool
		isVisible() const;

		void
		setBoundingBoxVisible(
			bool visible);

		bool
		isBoundingBoxVisible() const;

		virtual
		DataSourceRequirements
		dataSourceRequirements() const;

		bool
		requirementsMet() const;

		bool
		addVideoSource(
			VideoSourcePtr vSrc);

		size_t
		numVideoSources() const;

		void
		setVideoSource(
			size_t idx,
			VideoSourcePtr vSrc);

		VideoSourcePtr
		getVideoSource(
			size_t idx);

		void
		removeVideoSource(
			size_t idx);

		bool
		addTelemetrySource(
			TelemetrySourcePtr tSrc);

		size_t
		numTelemetrySources() const;

		void
		setTelemetrySource(
			size_t idx,
			TelemetrySourcePtr tSrc);

		TelemetrySourcePtr
		getTelemetrySource(
			size_t idx);

		void
		removeTelemetrySource(
			size_t idx);

		bool
		setTrack(
			const Track *track);

		const Track *
		getTrack() const;

		YAML::Node
		encode() const;

		bool
		decode(
			const YAML::Node& node,
			const DataSourceManager &dsm);

	protected:
		// callback to notify subclass that all source are valid and set
		virtual
		void
		sourcesValid();

		bool
		videoReqsMet() const;

		bool
		telemetryReqsMet() const;

		bool
		trackReqsMet() const;

		virtual
		YAML::Node
		subEncode() const = 0;

		virtual
		bool
		subDecode(
			const YAML::Node& node) = 0;

	private:

		void
		checkAndNotifyRequirementsMet();

	protected:
		// final rendered image
		cv::Mat outImg_;

		bool visible_;
		bool boundingBoxVisible_;

		std::vector<VideoSourcePtr> vSources_;
		std::vector<TelemetrySourcePtr> tSources_;
		const Track *track_;

	private:
		cv::Mat scaledImg_;

	};
}