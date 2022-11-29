#pragma once

#include <memory>
#include <vector>
#include <yaml-cpp/yaml.h>

#include "TelemetrySeeker.h"
#include "TelemetrySource.h"
#include "TrackDataObjects.h"
#include "VideoSource.h"

namespace gpo
{
	// forward declarations
	// class DataSourceManager;

	class DataSource
	{
	public:
		DataSource();

		std::string
		getSourceName() const;

		std::string
		getOrigin() const;

		bool
		setDatumTrack(
			const Track *track,
			bool processNow = true);

		const Track *
		getDatumTrack() const;

		bool
		reprocessDatumTrack();

		int
		lapCount() const;

		bool
		hasTelemetry() const;

		bool
		hasVideo() const;

		/**
		 * Produced a Track object from telemetry data.
		 * 
		 * @return
		 * If successful, a pointer to a newed Track object.
		 * nullptr on failure or if telemetry data is not present.
		 */
		Track *
		makeTrack() const;

	public:
		TelemetrySeekerPtr seeker;
		TelemetrySourcePtr telemSrc;
		VideoSourcePtr videoSrc;

	private:
		// allow DataSourceManager to modify sourceName_ and originFile_
		friend class DataSourceManager;

		std::string sourceName_;
		std::string originFile_;
		const Track *datumTrack_;
		int lapCount_;

	};

	using DataSourcePtr = std::shared_ptr<DataSource>;

	bool
	loadDataFromVideo(
		const std::string &videoFile,
		DataSourcePtr &data);

	class DataSourceManager
	{
	public:
		DataSourceManager();

		bool
		addVideo(
			const std::string &filepath);

		void
		removeSource(
			size_t idx);

		void
		setSourceName(
			size_t idx,
			const std::string &name);

		std::string
		getSourceName(
			size_t idx) const;

		std::string
		getSourceOrigin(
			size_t idx) const;

		size_t
		sourceCount() const;

		DataSourcePtr
		getSource(
			size_t idx);

		const DataSourcePtr
		getSource(
			size_t idx) const;

		DataSourcePtr
		getSourceByName(
			const std::string &sourceName);

		const DataSourcePtr
		getSourceByName(
			const std::string &sourceName) const;

		// YAML encode/decode
		YAML::Node
		encode() const;

		bool
		decode(
			const YAML::Node& node);

	private:
		bool
		addVideoSourceWithName(
			const std::string &filepath,
			const std::string &name);

	private:
		std::vector<DataSourcePtr> sources_;

	};
}