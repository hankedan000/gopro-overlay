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
	 , backupSamples_()
	 , dataAvail_()
	 , sourceName_("")
	 , originFile_("")
	 , datumTrack_(nullptr)
	{
		bitset_clear(dataAvail_);
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

		bool okay = utils::computeTrackTimes(datumTrack_,samples_,dataAvail_);

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

	const DataAvailableBitSet &
	DataSource::dataAvailable() const
	{
		return dataAvail_;
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

	DataSourcePtr
	DataSource::duplicate() const
	{
		auto dup = std::make_shared<DataSource>();
		dup->vCapture_ = vCapture_;
		dup->samples_ = std::make_shared<TelemetrySamples>();
		*(dup->samples_) = *samples_;
		dup->backupSamples_ = backupSamples_;
		dup->dataAvail_ = dataAvail_;
		dup->sourceName_ = sourceName_;
		dup->originFile_ = originFile_;
		dup->datumTrack_ = datumTrack_;

		dup->seeker = std::make_shared<TelemetrySeeker>(dup);
		if (telemSrc)
		{
			dup->telemSrc = std::make_shared<TelemetrySource>(dup);
		}
		if (videoSrc)
		{
			dup->videoSrc = std::make_shared<VideoSource>(dup);
		}
		return dup;
	}		
	
	bool
	DataSource::backupTelemetry()
	{
		if (samples_ == nullptr)
		{
			return false;
		}
		backupSamples_ = *(samples_);
		return true;
	}

	void
	DataSource::deleteTelemetryBackup()
	{
		backupSamples_.clear();
	}

	bool
	DataSource::hasBackup()
	{
		return backupSamples_.size() > 0;
	}

	bool
	DataSource::restoreTelemetry()
	{
		if (samples_ == nullptr)
		{
			return false;
		}
		*(samples_) = backupSamples_;
		return true;
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
	DataSource::loadDataFromFile(
		const std::filesystem::path &sourceFile)
	{
		// get the file extension in all lower case
		std::string fileExt = sourceFile.extension();
		std::transform(
					fileExt.begin(),
					fileExt.end(),
					fileExt.begin(),
					[](unsigned char c){ return std::tolower(c); });
		spdlog::debug("{} - adding '{}' with extension '{}'",
			__func__,
			sourceFile.c_str(),
			fileExt);

		// open the data source based on the file extension
		gpo::DataSourcePtr dSrc = nullptr;
		if (fileExt == ".mp4")
		{
			dSrc = gpo::DataSource::loadDataFromVideo(sourceFile);
		}
		else if (fileExt == ".msl")
		{
			dSrc = gpo::DataSource::loadDataFromMegaSquirtLog(sourceFile);
		}
		else if (fileExt == ".csv")
		{
			dSrc = gpo::DataSource::loadTelemetryFromCSV(sourceFile);
		}
		else
		{
			spdlog::warn("{} - unsupported source file extension '{}'",
						__func__,
						fileExt);
		}
		
		return dSrc;
	}

	DataSourcePtr
	DataSource::loadDataFromVideo(
		const std::filesystem::path &videoFile)
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
		newSrc->sourceName_ = videoFile.filename();
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
			bitset_set_bit(newSrc->dataAvail_, gpo::eDA_GOPRO_ACCL);
		}
		if (mp4.getSensorInfo(gpt::GPMF_KEY_GYRO, sensorInfo))
		{
			bitset_set_bit(newSrc->dataAvail_, gpo::eDA_GOPRO_GYRO);
		}
		if (mp4.getSensorInfo(gpt::GPMF_KEY_GRAV, sensorInfo))
		{
			bitset_set_bit(newSrc->dataAvail_, gpo::eDA_GOPRO_GRAV);
		}
		if (mp4.getSensorInfo(gpt::GPMF_KEY_CORI, sensorInfo))
		{
			bitset_set_bit(newSrc->dataAvail_, gpo::eDA_GOPRO_CORI);
		}
		if (mp4.getSensorInfo(gpt::GPMF_KEY_GPS5, sensorInfo))
		{
			bitset_set_bit(newSrc->dataAvail_, gpo::eDA_GOPRO_GPS_LATLON);
			bitset_set_bit(newSrc->dataAvail_, gpo::eDA_GOPRO_GPS_ALTITUDE);
			bitset_set_bit(newSrc->dataAvail_, gpo::eDA_GOPRO_GPS_SPEED2D);
			bitset_set_bit(newSrc->dataAvail_, gpo::eDA_GOPRO_GPS_SPEED3D);
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
		newSrc->sourceName_ = logFile.filename();
		newSrc->samples_ = std::make_shared<TelemetrySamples>();
		newSrc->samples_->resize(ecuTelem.size());
		newSrc->dataAvail_ = res.second;
		bitset_clr_bit(newSrc->dataAvail_, eDA_ECU_TIME);// don't want to track this here
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
		newSrc->sourceName_ = csvFile.filename();
		newSrc->samples_ = std::make_shared<TelemetrySamples>();
		utils::io::readTelemetryFromCSV(
			csvFile,
			newSrc->samples_,
			newSrc->dataAvail_);

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
			dataAvail_);
	}

	size_t
	DataSource::mergeTelemetryIn(
		const DataSourcePtr srcData,
		size_t srcStartIdx,
		size_t dstStartIdx,
		bool growVector)
	{
		return mergeTelemetryIn(
			srcData,
			srcStartIdx,
			dstStartIdx,
			srcData->dataAvailable(),
			growVector);
	}

	size_t
	DataSource::mergeTelemetryIn(
		const DataSourcePtr srcData,
		size_t srcStartIdx,
		size_t dstStartIdx,
		DataAvailableBitSet dataToTake,
		bool growVector)
	{
		if ( ! hasTelemetry())
		{
			spdlog::warn(
				"DataSource doesn't have telemetry. Unable to perform merge.");
			return 0;
		}

		const auto &srcSamps = srcData->samples_;
		if (srcStartIdx >= srcSamps->size())
		{
			spdlog::error(
				"srcStartIdx ({}) is >= source sample count ({})",
				srcStartIdx,
				srcSamps->size());
			return 0;
		}

		auto &dstSamps = samples_;
		if (dstStartIdx >= dstSamps->size())
		{
			spdlog::error(
				"dstStartIdx ({}) is >= destination sample count ({})",
				dstStartIdx,
				dstSamps->size());
			return 0;
		}

		const auto srcRate_hz = srcData->getTelemetryRate_hz();
		const auto dstRate_hz = this->getTelemetryRate_hz();
		const auto deltaRate_hz = srcRate_hz - dstRate_hz;
		const double RATE_THRESHOLD_HZ = 0.1;
		if (std::abs(deltaRate_hz) > RATE_THRESHOLD_HZ)
		{
			spdlog::error(
				"source and destination data rates differ by more than {}Hz.\n"
				" srcRate = {}Hz;\n"
				" dstRate = {}Hz;",
				RATE_THRESHOLD_HZ,
				srcRate_hz,
				dstRate_hz);
			return 0;
		}

		// compute number of samples we could merge into without growing vector
		const size_t nSampsIngress = srcSamps->size() - srcStartIdx;
		const size_t nSampsWeCouldTakeWithoutGrowth = dstSamps->size() - dstStartIdx;
		// compute the number of samples we plan to merge based on growthability
		size_t nSampsToMerge = nSampsIngress;
		if (nSampsToMerge > nSampsWeCouldTakeWithoutGrowth)
		{
			if (growVector)
			{
				size_t growthNeeded = nSampsToMerge - nSampsWeCouldTakeWithoutGrowth;
				samples_->resize(samples_->size() + growthNeeded);
			}
			else
			{
				nSampsToMerge = nSampsWeCouldTakeWithoutGrowth;
			}
		}
		spdlog::debug(
			"srcSamps->size(): {}; srcStartIdx: {}\n"
			"dstSamps->size(): {}; dstStartIdx: {}\n"
			"nSampsIngress: {}\n"
			"nSampsWeCouldTakeWithoutGrowth: {}\n"
			"growVector: {}\n"
			"nSampsToMerge: {}",
			srcSamps->size(), srcStartIdx,
			dstSamps->size(), dstStartIdx,
			nSampsIngress,
			nSampsWeCouldTakeWithoutGrowth,
			growVector,
			nSampsToMerge);

		size_t mergedSamples = 0;
		for (size_t i=0; i<nSampsToMerge; i++, mergedSamples++)
		{
			auto &dstSamp = dstSamps->at(dstStartIdx + i);
			const auto &srcSamp = srcSamps->at(srcStartIdx + i);

			// merge in GoPro samples
			auto bitToTake = eDA_GOPRO_ACCL;
			if (bitset_is_set(dataToTake, bitToTake))
			{
				dstSamp.gpSamp.accl = srcSamp.gpSamp.accl;
				bitset_set_bit(dataAvail_, bitToTake);
				bitset_clr_bit(dataToTake, bitToTake);
			}
			bitToTake = eDA_GOPRO_GYRO;
			if (bitset_is_set(dataToTake, bitToTake))
			{
				dstSamp.gpSamp.gyro = srcSamp.gpSamp.gyro;
				bitset_set_bit(dataAvail_, bitToTake);
				bitset_clr_bit(dataToTake, bitToTake);
			}
			bitToTake = eDA_GOPRO_GRAV;
			if (bitset_is_set(dataToTake, bitToTake))
			{
				dstSamp.gpSamp.grav = srcSamp.gpSamp.grav;
				bitset_set_bit(dataAvail_, bitToTake);
				bitset_clr_bit(dataToTake, bitToTake);
			}
			bitToTake = eDA_GOPRO_CORI;
			if (bitset_is_set(dataToTake, bitToTake))
			{
				dstSamp.gpSamp.cori = srcSamp.gpSamp.cori;
				bitset_set_bit(dataAvail_, bitToTake);
				bitset_clr_bit(dataToTake, bitToTake);
			}
			bitToTake = eDA_GOPRO_GPS_LATLON;
			if (bitset_is_set(dataToTake, bitToTake))
			{
				dstSamp.gpSamp.gps.coord = srcSamp.gpSamp.gps.coord;
				bitset_set_bit(dataAvail_, bitToTake);
				bitset_clr_bit(dataToTake, bitToTake);
			}
			bitToTake = eDA_GOPRO_GPS_ALTITUDE;
			if (bitset_is_set(dataToTake, bitToTake))
			{
				dstSamp.gpSamp.gps.altitude = srcSamp.gpSamp.gps.altitude;
				bitset_set_bit(dataAvail_, bitToTake);
				bitset_clr_bit(dataToTake, bitToTake);
			}
			bitToTake = eDA_GOPRO_GPS_SPEED2D;
			if (bitset_is_set(dataToTake, bitToTake))
			{
				dstSamp.gpSamp.gps.speed2D = srcSamp.gpSamp.gps.speed3D;
				bitset_set_bit(dataAvail_, bitToTake);
				bitset_clr_bit(dataToTake, bitToTake);
			}
			bitToTake = eDA_GOPRO_GPS_SPEED3D;
			if (bitset_is_set(dataToTake, bitToTake))
			{
				dstSamp.gpSamp.gps.speed3D = srcSamp.gpSamp.gps.speed3D;
				bitset_set_bit(dataAvail_, bitToTake);
				bitset_clr_bit(dataToTake, bitToTake);
			}

			// merge in ECU samples
			bitToTake = eDA_ECU_ENGINE_SPEED;
			if (bitset_is_set(dataToTake, bitToTake))
			{
				dstSamp.ecuSamp.engineSpeed_rpm = srcSamp.ecuSamp.engineSpeed_rpm;
				bitset_set_bit(dataAvail_, bitToTake);
				bitset_clr_bit(dataToTake, bitToTake);
			}
			bitToTake = eDA_ECU_TPS;
			if (bitset_is_set(dataToTake, bitToTake))
			{
				dstSamp.ecuSamp.tps = srcSamp.ecuSamp.tps;
				bitset_set_bit(dataAvail_, bitToTake);
				bitset_clr_bit(dataToTake, bitToTake);
			}
			bitToTake = eDA_ECU_BOOST;
			if (bitset_is_set(dataToTake, bitToTake))
			{
				dstSamp.ecuSamp.boost_psi = srcSamp.ecuSamp.boost_psi;
				bitset_set_bit(dataAvail_, bitToTake);
				bitset_clr_bit(dataToTake, bitToTake);
			}

			// merge in Track samples
			bitToTake = eDA_TRACK_ON_TRACK_LATLON;
			if (bitset_is_set(dataToTake, bitToTake))
			{
				dstSamp.trackData.onTrackLL = srcSamp.trackData.onTrackLL;
				bitset_set_bit(dataAvail_, bitToTake);
				bitset_clr_bit(dataToTake, bitToTake);
			}
			bitToTake = eDA_TRACK_LAP;
			if (bitset_is_set(dataToTake, bitToTake))
			{
				dstSamp.trackData.lap = srcSamp.trackData.lap;
				bitset_set_bit(dataAvail_, bitToTake);
				bitset_clr_bit(dataToTake, bitToTake);
			}
			bitToTake = eDA_TRACK_LAP_TIME_OFFSET;
			if (bitset_is_set(dataToTake, bitToTake))
			{
				dstSamp.trackData.lapTimeOffset = srcSamp.trackData.lapTimeOffset;
				bitset_set_bit(dataAvail_, bitToTake);
				bitset_clr_bit(dataToTake, bitToTake);
			}
			bitToTake = eDA_TRACK_SECTOR;
			if (bitset_is_set(dataToTake, bitToTake))
			{
				dstSamp.trackData.sector = srcSamp.trackData.sector;
				bitset_set_bit(dataAvail_, bitToTake);
				bitset_clr_bit(dataToTake, bitToTake);
			}
			bitToTake = eDA_TRACK_SECTOR_TIME_OFFSET;
			if (bitset_is_set(dataToTake, bitToTake))
			{
				dstSamp.trackData.sectorTimeOffset = srcSamp.trackData.sectorTimeOffset;
				bitset_set_bit(dataAvail_, bitToTake);
				bitset_clr_bit(dataToTake, bitToTake);
			}
		}

		// make sure everything was merged (future proofing logic in case fields are added)
		if (bitset_is_any_set(dataToTake))
		{
			spdlog::warn(
				"unmerged GoPro samples remain. seems like {}() implementation is incomplete.",
				__func__);
		}
		if (bitset_is_any_set(dataToTake))
		{
			spdlog::warn(
				"unmerged ECU samples remain. seems like {}() implementation is incomplete.",
				__func__);
		}
		if (bitset_is_any_set(dataToTake))
		{
			spdlog::warn(
				"unmerged Track samples remain. seems like {}() implementation is incomplete.",
				__func__);
		}

		return mergedSamples;
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