#include "GoProOverlay/data/DataSource.h"

#include <filesystem>
#include <spdlog/spdlog.h>

#include <GoProTelem/GoProTelem.h>
#include <GoProTelem/SampleMath.h>
#include <GoProOverlay/utils/DataProcessingUtils.h>
#include <GoProOverlay/utils/io/CSV_Utils.h>

namespace gpo
{
	DataSource::DataSource()
	 : seeker(nullptr)
	 , telemSrc(nullptr)
	 , videoSrc(nullptr)
	 , vCapture_()
	 , samples_(nullptr)
	 , gpDataAvail_()
	 , ecuDataAvail_()
	 , sourceName_("")
	 , originFile_("")
	 , datumTrack_(nullptr)
	{
		bitset_clear(gpDataAvail_);
		bitset_clear(ecuDataAvail_);
		bitset_clear(trackAvail_);
	}

	std::string
	DataSource::getSourceName() const
	{
		return sourceName_;
	}

	std::string
	DataSource::getOrigin() const
	{
		return originFile_;
	}

	bool
	DataSource::setDatumTrack(
		const Track *track,
		bool processNow)
	{
		datumTrack_ = track;
		if (processNow)
		{
			return reprocessDatumTrack();
		}
		return true;
	}

	const Track *
	DataSource::getDatumTrack() const
	{
		return datumTrack_;
	}

	bool
	DataSource::reprocessDatumTrack()
	{
		if (datumTrack_ == nullptr)
		{
			// nothing to process
			return true;
		}

		bool okay = utils::computeTrackTimes(datumTrack_,samples_,trackAvail_);

		// smooth accelerometer data
		if (false)
		{
			// TODO put this in a better place
			size_t acclFieldOffsets[] = {
				offsetof(gpo::TelemetrySample, gpSamp.accl.x),
				offsetof(gpo::TelemetrySample, gpSamp.accl.y),
				offsetof(gpo::TelemetrySample, gpSamp.accl.z)
			};
			utils::smoothMovingAvgStructured<gpo::TelemetrySample,decltype(gpt::AcclSample::x)>(
				samples_->data(),
				samples_->data(),
				acclFieldOffsets,
				3,// 3 fields; x,y,z
				samples_->size(),
				30);
		}

		if (okay)
		{
			seeker->analyze();
		}

		return okay;
	}

	int
	DataSource::lapCount() const
	{
		return seeker->lapCount();
	}

	bool
	DataSource::hasTelemetry() const
	{
		return telemSrc != nullptr;
	}

	bool
	DataSource::hasVideo() const
	{
		return videoSrc != nullptr;
	}

	const GoProDataAvailBitSet &
	DataSource::gpDataAvail() const
	{
		// TODO need to populate this structure. update GoProTelem library???
		throw std::runtime_error("gpDataAvail_ hasn't been populated yet");
		return gpDataAvail_;
	}

	const ECU_DataAvailBitSet &
	DataSource::ecuDataAvail() const
	{
		return ecuDataAvail_;
	}

	const TrackDataAvailBitSet &
	DataSource::trackDataAvail() const
	{
		return trackAvail_;
	}

	double
	DataSource::getTelemetryRate_hz() const
	{
		if ( ! hasTelemetry() || samples_->size() < 2)
		{
			return 0.0;
		}
		return (samples_->size() - 1) / samples_->back().t_offset;
	}

	void
	DataSource::resampleTelemetry(
		double newRate_hz)
	{
		if ( ! hasTelemetry())
		{
			spdlog::warn("DataSource doesn't have any telemetry data. ignoring request.");
			return;
		}
		else if (hasVideo())
		{
			spdlog::error("can't resample DataSource that has a video associated with it.");
			return;
		}
		else if (samples_->empty())
		{
			// no samples means nothing resample!
			return;
		}

		// copy old samples
		TelemetrySamples oldSamps = *samples_;

		double duration_sec = oldSamps.back().t_offset;
		size_t nSampsOut = round(newRate_hz * duration_sec);
		samples_->resize(nSampsOut);
		double outDt_sec = 1.0 / newRate_hz;

		size_t takeIdx = 0;
		double outTime_sec = 0.0;
		for (size_t outIdx=0; outIdx<nSampsOut; outIdx++)
		{
			bool found = gpt::findLerpIndex(takeIdx,oldSamps,outTime_sec);

			if (found)
			{
				const auto &sampA = oldSamps.at(takeIdx);
				const auto &sampB = oldSamps.at(takeIdx+1);
				const double dt = sampB.t_offset - sampA.t_offset;
				const double ratio = (outTime_sec - sampA.t_offset) / dt;
				utils::lerp(samples_->at(outIdx),sampA,sampB,ratio);
			}
			else if (takeIdx == 0)
			{
				samples_->at(outIdx) = oldSamps.at(takeIdx);
			}
			else
			{
				samples_->at(outIdx) = oldSamps.back();
			}
			outTime_sec += outDt_sec;
		}
	}

	Track *
	DataSource::makeTrack() const
	{
		if ( ! hasTelemetry())
		{
			return nullptr;
		}

		return makeTrackFromTelemetry(telemSrc);
	}

	DataSourcePtr
	DataSource::loadDataFromVideo(
		const std::string &videoFile)
	{
		gpt::MP4_Source mp4;
		mp4.open(videoFile);
		auto videoTelem = gpt::getCombinedTimedSamples(mp4);
		if (videoTelem.empty())
		{
			return nullptr;
		}
		cv::VideoCapture vCap(videoFile);
		if ( ! vCap.isOpened())
		{
			return nullptr;
		}

		auto newSrc = std::make_shared<DataSource>();
		newSrc->originFile_ = videoFile;
		newSrc->vCapture_ = vCap;
		newSrc->samples_ = std::make_shared<TelemetrySamples>();
		newSrc->samples_->resize(videoTelem.size());
		for (size_t i=0; i<videoTelem.size(); i++)
		{
			auto &outSamp = newSrc->samples_->at(i);
			const auto &gpSamp = videoTelem.at(i);
			outSamp.t_offset = gpSamp.t_offset;
			outSamp.gpSamp = gpSamp.sample;
		}

		// populate GoPro data availability
		gpt::MP4_SensorInfo sensorInfo;
		if (mp4.getSensorInfo(gpt::GPMF_KEY_ACCL, sensorInfo))
		{
			bitset_set_bit(newSrc->gpDataAvail_, gpo::GOPRO_AVAIL_ACCL);
		}
		if (mp4.getSensorInfo(gpt::GPMF_KEY_GYRO, sensorInfo))
		{
			bitset_set_bit(newSrc->gpDataAvail_, gpo::GOPRO_AVAIL_GYRO);
		}
		if (mp4.getSensorInfo(gpt::GPMF_KEY_GRAV, sensorInfo))
		{
			bitset_set_bit(newSrc->gpDataAvail_, gpo::GOPRO_AVAIL_GRAV);
		}
		if (mp4.getSensorInfo(gpt::GPMF_KEY_CORI, sensorInfo))
		{
			bitset_set_bit(newSrc->gpDataAvail_, gpo::GOPRO_AVAIL_CORI);
		}
		if (mp4.getSensorInfo(gpt::GPMF_KEY_GPS5, sensorInfo))
		{
			bitset_set_bit(newSrc->gpDataAvail_, gpo::GOPRO_AVAIL_GPS_LATLON);
			bitset_set_bit(newSrc->gpDataAvail_, gpo::GOPRO_AVAIL_GPS_ALTITUDE);
			bitset_set_bit(newSrc->gpDataAvail_, gpo::GOPRO_AVAIL_GPS_SPEED2D);
			bitset_set_bit(newSrc->gpDataAvail_, gpo::GOPRO_AVAIL_GPS_SPEED3D);
		}

		newSrc->seeker = std::make_shared<TelemetrySeeker>(newSrc);
		newSrc->telemSrc = std::make_shared<TelemetrySource>(newSrc);
		newSrc->videoSrc = std::make_shared<VideoSource>(newSrc);

		return newSrc;
	}

	DataSourcePtr
	DataSource::loadDataFromMegaSquirtLog(
		const std::filesystem::path &logFile)
	{
		std::vector<gpo::ECU_TimedSample> ecuTelem;
		auto res = utils::io::readMegaSquirtLog(logFile,ecuTelem);
		if ( ! res.first)
		{
			return nullptr;
		}

		auto newSrc = std::make_shared<DataSource>();
		newSrc->originFile_ = logFile;
		newSrc->samples_ = std::make_shared<TelemetrySamples>();
		newSrc->samples_->resize(ecuTelem.size());
		newSrc->ecuDataAvail_ = res.second;
		for (size_t i=0; i<ecuTelem.size(); i++)
		{
			auto &sampOut = newSrc->samples_->at(i);
			const auto &ecuSamp = ecuTelem.at(i);
			sampOut.t_offset = ecuSamp.t_offset;
			sampOut.ecuSamp = ecuSamp.sample;
		}

		newSrc->seeker = std::make_shared<TelemetrySeeker>(newSrc);
		newSrc->telemSrc = std::make_shared<TelemetrySource>(newSrc);

		return newSrc;
	}

	DataSourcePtr
	DataSource::loadTelemetryFromCSV(
		const std::filesystem::path &csvFile)
	{
		auto newSrc = std::make_shared<DataSource>();
		newSrc->originFile_ = csvFile;
		newSrc->samples_ = std::make_shared<TelemetrySamples>();
		utils::io::readTelemetryFromCSV(
			csvFile,
			newSrc->samples_,
			newSrc->gpDataAvail_,
			newSrc->ecuDataAvail_,
			newSrc->trackAvail_);

		newSrc->seeker = std::make_shared<TelemetrySeeker>(newSrc);
		newSrc->telemSrc = std::make_shared<TelemetrySource>(newSrc);

		return newSrc;
	}

	DataSourcePtr
	DataSource::makeDataFromTelemetry(
		const gpo::TelemetrySamples &tSamps)
	{
		auto newSrc = std::make_shared<DataSource>();
		newSrc->samples_ = std::make_shared<TelemetrySamples>();
		newSrc->samples_->resize(tSamps.size());
		newSrc->samples_->assign(tSamps.begin(),tSamps.end());

		newSrc->seeker = std::make_shared<TelemetrySeeker>(newSrc);
		newSrc->telemSrc = std::make_shared<TelemetrySource>(newSrc);
		newSrc->videoSrc = nullptr;

		return newSrc;
	}

	bool
	DataSource::writeTelemetryToCSV(
		const std::filesystem::path &csvFilepath) const
	{
		if ( ! hasTelemetry())
		{
			return false;
		}
		return utils::io::writeTelemetryToCSV(
			samples_,
			csvFilepath,
			gpDataAvail_,
			ecuDataAvail_,
			trackAvail_);
	}

	DataSourceManager::DataSourceManager()
	 : sources_()
	{
	}

	void
	DataSourceManager::clear()
	{
		sources_.clear();
	}

	bool
	DataSourceManager::addVideo(
		const std::string &filepath)
	{
		for (const auto &source : sources_)
		{
			if (source->originFile_ == filepath)
			{
				// already imported
				return false;
			}
		}

		std::filesystem::path p(filepath);
		return addVideoSourceWithName(p,p.filename());
	}

	void
	DataSourceManager::removeSource(
		size_t idx)
	{
		sources_.erase(std::next(sources_.begin(), idx));
	}

	void
	DataSourceManager::setSourceName(
		size_t idx,
		const std::string &name)
	{
		sources_.at(idx)->sourceName_ = name;
	}

	std::string
	DataSourceManager::getSourceName(
		size_t idx) const
	{
		return sources_.at(idx)->sourceName_;
	}

	std::string
	DataSourceManager::getSourceOrigin(
		size_t idx) const
	{
		return sources_.at(idx)->originFile_;
	}

	size_t
	DataSourceManager::sourceCount() const
	{
		return sources_.size();
	}

	DataSourcePtr
	DataSourceManager::getSource(
		size_t idx)
	{
		return sources_.at(idx);
	}

	const DataSourcePtr
	DataSourceManager::getSource(
		size_t idx) const
	{
		return sources_.at(idx);
	}

	DataSourcePtr
	DataSourceManager::getSourceByName(
		const std::string &sourceName)
	{
		for (auto src : sources_)
		{
			if (src->sourceName_ == sourceName)
			{
				return src;
			}
		}
		return nullptr;
	}

	const DataSourcePtr
	DataSourceManager::getSourceByName(
		const std::string &sourceName) const
	{
		for (auto src : sources_)
		{
			if (src->sourceName_ == sourceName)
			{
				return src;
			}
		}
		return nullptr;
	}

	// YAML encode/decode
	YAML::Node
	DataSourceManager::encode() const
	{
		YAML::Node node;

		YAML::Node ySources = node["sources"];
		for (const auto &source : sources_)
		{
			YAML::Node ySource;
			ySource["originFile"] = source->originFile_;
			ySource["name"] = source->sourceName_;

			ySources.push_back(ySource);
		}

		return node;
	}

	bool
	DataSourceManager::decode(
		const YAML::Node& node)
	{
		bool okay = true;

		sources_.clear();
		if (node["sources"])// not all files will have sources
		{
			const YAML::Node &ySources = node["sources"];
			sources_.reserve(ySources.size());
			for (size_t ss=0; okay && ss<ySources.size(); ss++)
			{
				const YAML::Node &ySource = ySources[ss];
				okay = addVideoSourceWithName(
					ySource["originFile"].as<std::string>(),
					ySource["name"].as<std::string>()) && okay;
			}
		}

		return okay;
	}

	bool
	DataSourceManager::addVideoSourceWithName(
		const std::string &filepath,
		const std::string &name)
	{
		auto dataSrc = DataSource::loadDataFromVideo(filepath);
		if (dataSrc)
		{
			dataSrc->sourceName_ = name;
			sources_.push_back(dataSrc);
		}
		return dataSrc != nullptr;
	}

}