#pragma once

#include <filesystem>
#include <memory>
#include <vector>
#include <yaml-cpp/yaml.h>

#include "TelemetrySeeker.h"
#include "TelemetrySource.h"
#include "TrackDataObjects.h"
#include "VideoSource.h"

namespace gpo
{
	// forward declaration
	class DataSource;

	using DataSourcePtr = std::shared_ptr<DataSource>;

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

		const GoProDataAvailBitSet &
		gpDataAvail() const;

		const ECU_DataAvailBitSet &
		ecuDataAvail() const;

		const TrackDataAvailBitSet &
		trackDataAvail() const;

		double
		getTelemetryRate_hz() const;

		void
		resampleTelemetry(
			double newRate_hz);

		/**
		 * Produced a Track object from telemetry data.
		 * 
		 * @return
		 * If successful, a pointer to a newed Track object.
		 * nullptr on failure or if telemetry data is not present.
		 */
		Track *
		makeTrack() const;

		static
		DataSourcePtr
		loadDataFromVideo(
			const std::string &videoFile);

		static
		DataSourcePtr
		loadDataFromMegaSquirtLog(
			const std::filesystem::path &logFile);

		static
		DataSourcePtr
		loadTelemetryFromCSV(
			const std::filesystem::path &csvFile);

		static
		DataSourcePtr
		makeDataFromTelemetry(
			const gpo::TelemetrySamples &tSamps);

		bool
		writeTelemetryToCSV(
			const std::filesystem::path &csvFilepath) const;

	public:
		TelemetrySeekerPtr seeker;
		TelemetrySourcePtr telemSrc;
		VideoSourcePtr videoSrc;

	private:
		// allow DataSourceManager to modify sourceName_ and originFile_
		friend class DataSourceManager;

		friend class TelemetrySeeker;
		friend class TelemetrySource;
		friend class VideoSource;

		cv::VideoCapture vCapture_;
		TelemetrySamplesPtr samples_;

		// bitset defining which fields are valid in 'TelemetrySample::gpSamp'
		// query bits using gpo::GOPRO_AVAIL_* constants
		GoProDataAvailBitSet gpDataAvail_;

		// bitset defining which fields are valid in 'TelemetrySample::ecuSamp'
		// query bits using gpo::ECU_AVAIL_* constants
		ECU_DataAvailBitSet ecuDataAvail_;

		// bitset defining which fields are valid in 'TelemetrySample::trackData'
		// query bits using gpo::TRACK_AVAIL_* constants
		TrackDataAvailBitSet trackAvail_;

		std::string sourceName_;
		std::string originFile_;
		const Track *datumTrack_;

	};

	class DataSourceManager
	{
	public:
		DataSourceManager();

		void
		clear();

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