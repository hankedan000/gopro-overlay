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
		calcVehicleAcceleration(
			size_t smoothingWindowSize = 30);

		bool
		reprocessDatumTrack();

		int
		lapCount() const;

		bool
		hasTelemetry() const;

		bool
		hasVideo() const;

		const DataAvailableBitSet &
		dataAvailable() const;

		double
		getTelemetryRate_hz() const;

		void
		resampleTelemetry(
			double newRate_hz);

		DataSourcePtr
		duplicate() const;

		/**
		 * Save the current state of the telemetry samples, allowing
		 * you to restore them via restoreTelemetry().
		 * 
		 * @return
		 * true if telemetry samples were backed up
		 */
		bool
		backupTelemetry();

		/**
		 * Removes the telemetry backup
		 */
		void
		deleteTelemetryBackup();

		bool
		hasBackup();

		/**
		 * Restores the previously saved backup samples.
		 */
		bool
		restoreTelemetry();

		/**
		 * Produced a Track object from telemetry data.
		 * 
		 * @return
		 * If successful, a pointer to a newed Track object.
		 * nullptr on failure or if telemetry data is not present.
		 */
		Track *
		makeTrack() const;

		/**
		 * Attempt to load the data from a file. It will attempt to auto
		 * detect the data source type based on the file extension.
		 * 
		 * @return
		 * nullptr if the load failed. valid pointer otherwise.
		 */
		static
		DataSourcePtr
		loadDataFromFile(
			const std::filesystem::path &sourceFile);

		static
		DataSourcePtr
		loadDataFromVideo(
			const std::filesystem::path &videoFile);

		static
		DataSourcePtr
		loadDataFromMegaSquirtLog(
			const std::filesystem::path &logFile);

		static
		DataSourcePtr
		loadDataFromSoloStormCSV(
			const std::filesystem::path &csvFile);

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

		/**
		 * Merges all available telemetry data from another source into this
		 * one. For this to be successful, the two sources need to have similar
		 * data rates.
		 * 
		 * This method simply redirects to the other mergeTelemetryIn() method,
		 * but takes all the sample data available from 'srcData'.
		 */
		size_t
		mergeTelemetryIn(
			const DataSourcePtr srcData,
			size_t srcStartIdx,
			size_t dstStartIdx,
			bool growVector);

		/**
		 * Merges telemetry data from another source into this one. For this
		 * to be successful, the two sources need to have similar data rates.
		 * 
		 * @param[in] srcData
		 * the DataSource to take telemetry samples from
		 * 
		 * @param[in] srcStartIdx
		 * the index within 'srcData' to begin taking samples from
		 * 
		 * @param[in] dstStartIdx
		 * the index within this DataSource to begin merging sample data
		 * 
		 * @param[in] gpDataToTake
		 * set of which GoProTelemetry samples to merge in
		 * 
		 * @param[in] ecuDataToTake
		 * set of which ECU samples to merge in
		 * 
		 * @param[in] trackDataToTake
		 * set of which track samples to merge in
		 * 
		 * @param[in] growVector
		 * if set true, and the merged number of samples extends past the end
		 * of the current telemetry vector's length, then the telemetry vector
		 * will be resized to fit the new samples. otherwise those samples will
		 * be left unmerged.
		 * 
		 * @return
		 * the number of samples merged
		 */
		size_t
		mergeTelemetryIn(
			const DataSourcePtr srcData,
			size_t srcStartIdx,
			size_t dstStartIdx,
			DataAvailableBitSet dataToTake,
			bool growVector);

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

		// user can backup telemetry samples, and this is where they are stored
		TelemetrySamples backupSamples_;

		// bitset defining which fields are valid in 'TelemetrySample'
		// query bits using gpo::DataAvailable enum literals
		DataAvailableBitSet dataAvail_;

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