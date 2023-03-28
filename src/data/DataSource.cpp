#include "GoProOverlay/data/DataSource.h"

#include <cmath>
#include <filesystem>
#include <spdlog/spdlog.h>

#include <GoProTelem/GoProTelem.h>
#include <GoProTelem/SampleMath.h>
#include <GoProOverlay/utils/DataProcessingUtils.h>
#include <GoProOverlay/utils/io/csv.h>

namespace gpo
{
	DataSource::DataSource()
	 : ModifiableObject("DataSource",false,true)
	 , seeker(nullptr)
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
		dataAvail_.reset();
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
		std::shared_ptr<const Track> track,
		bool processNow)
	{
		datumTrack_ = track;
		if (processNow)
		{
			return reprocessDatumTrack();
		}
		return true;
	}

	const std::shared_ptr<const Track> &
	DataSource::getDatumTrack() const
	{
		return datumTrack_;
	}

	bool
	DataSource::calcVehicleAcceleration(
		size_t smoothingWindowSize)
	{
		if (samples_->empty())
		{
			// nothing to process
			return true;
		}

		// smooth accelerometer data
		const std::array<size_t, 3> inFieldOffsets = {
			offsetof(gpo::TelemetrySample, gpSamp.accl.x),
			offsetof(gpo::TelemetrySample, gpSamp.accl.y),
			offsetof(gpo::TelemetrySample, gpSamp.accl.z)
		};
		const std::array<size_t, 3> outFieldOffsets = {
			offsetof(gpo::TelemetrySample, calcSamp.smoothAccl.x),
			offsetof(gpo::TelemetrySample, calcSamp.smoothAccl.y),
			offsetof(gpo::TelemetrySample, calcSamp.smoothAccl.z)
		};
		utils::smoothMovingAvgStructured<gpo::TelemetrySample,decltype(gpt::AcclSample::x)>(
			samples_->data(),
			samples_->data(),
			inFieldOffsets,
			outFieldOffsets,
			samples_->size(),
			smoothingWindowSize);
		dataAvail_.set(DataAvailable::eDA_CALC_SMOOTH_ACCL);

		cv::Vec3f latDir = {};
		cv::Vec3f lonDir = {};
		bool okay = utils::computeVehicleDirectionVectors(samples_,dataAvail_,latDir,lonDir);
		okay = okay && utils::computeVehicleAcceleration(samples_,dataAvail_,latDir,lonDir);

		return okay;
	}

	bool
	DataSource::reprocessDatumTrack()
	{
		if (datumTrack_ == nullptr || samples_->empty())
		{
			// nothing to process
			return true;
		}

		bool okay = utils::computeTrackTimes(datumTrack_,samples_,dataAvail_);

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
			if (gpt::findLerpIndex(takeIdx,oldSamps,outTime_sec))
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
	DataSource::hasBackup() const
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
		*samples_ = backupSamples_;
		return true;
	}

	TrackPtr
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
			newSrc->dataAvail_.set(gpo::eDA_GOPRO_ACCL);
		}
		if (mp4.getSensorInfo(gpt::GPMF_KEY_GYRO, sensorInfo))
		{
			newSrc->dataAvail_.set(gpo::eDA_GOPRO_GYRO);
		}
		if (mp4.getSensorInfo(gpt::GPMF_KEY_GRAV, sensorInfo))
		{
			newSrc->dataAvail_.set(gpo::eDA_GOPRO_GRAV);
		}
		if (mp4.getSensorInfo(gpt::GPMF_KEY_CORI, sensorInfo))
		{
			newSrc->dataAvail_.set(gpo::eDA_GOPRO_CORI);
		}
		if (mp4.getSensorInfo(gpt::GPMF_KEY_GPS5, sensorInfo))
		{
			newSrc->dataAvail_.set(gpo::eDA_GOPRO_GPS_LATLON);
			newSrc->dataAvail_.set(gpo::eDA_GOPRO_GPS_ALTITUDE);
			newSrc->dataAvail_.set(gpo::eDA_GOPRO_GPS_SPEED2D);
			newSrc->dataAvail_.set(gpo::eDA_GOPRO_GPS_SPEED3D);
		}

		// smooth acceleration and compute lateral/logitudinal acceleration
		newSrc->calcVehicleAcceleration(30);

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
		auto [readOkay, dataAvail] = utils::io::readMegaSquirtLog(logFile,ecuTelem);
		if ( ! readOkay)
		{
			return nullptr;
		}

		auto newSrc = std::make_shared<DataSource>();
		newSrc->originFile_ = logFile;
		newSrc->sourceName_ = logFile.filename();
		newSrc->samples_ = std::make_shared<TelemetrySamples>();
		newSrc->samples_->resize(ecuTelem.size());
		newSrc->dataAvail_ = dataAvail;
		newSrc->dataAvail_.reset(eDA_ECU_TIME);// don't want to track this here
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
	DataSource::loadDataFromSoloStormCSV(
		const std::filesystem::path &csvFile)
	{
		auto newSrc = std::make_shared<DataSource>();
		newSrc->originFile_ = csvFile;
		newSrc->sourceName_ = csvFile.filename();
		newSrc->samples_ = std::make_shared<TelemetrySamples>();
		utils::io::readTelemetryFromSoloStormCSV(
			csvFile,
			newSrc->samples_,
			newSrc->dataAvail_);

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
		const size_t srcStartIdx,
		const size_t dstStartIdx,
		bool growVector)
	{
		return mergeTelemetryIn(
			srcData,
			srcStartIdx,
			dstStartIdx,
			srcData->dataAvailable(),
			growVector);
	}

	#define TAKE_DATA_IF_AVAIL(bitToTake, dataPath)    \
			if (dataToTake.test(bitToTake))  \
			{                                          \
				dstSamp.dataPath = srcSamp.dataPath; \
				dataAvail_.set(bitToTake); \
				dataToTake.reset(bitToTake); \
			}

	size_t
	DataSource::mergeTelemetryIn(
		const DataSourcePtr srcData,
		const size_t srcStartIdx,
		const size_t dstStartIdx,
		DataAvailableBitSet dataToTake,
		const bool growVector)
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

		const auto &dstSamps = samples_;
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
			TAKE_DATA_IF_AVAIL(eDA_GOPRO_ACCL, gpSamp.accl)
			TAKE_DATA_IF_AVAIL(eDA_GOPRO_GRAV, gpSamp.gyro)
			TAKE_DATA_IF_AVAIL(eDA_GOPRO_GYRO, gpSamp.grav)
			TAKE_DATA_IF_AVAIL(eDA_GOPRO_CORI, gpSamp.cori)
			TAKE_DATA_IF_AVAIL(eDA_GOPRO_GPS_LATLON, gpSamp.gps.coord)
			TAKE_DATA_IF_AVAIL(eDA_GOPRO_GPS_ALTITUDE, gpSamp.gps.altitude)
			TAKE_DATA_IF_AVAIL(eDA_GOPRO_GPS_SPEED2D, gpSamp.gps.speed2D)
			TAKE_DATA_IF_AVAIL(eDA_GOPRO_GPS_SPEED3D, gpSamp.gps.speed3D)

			// merge in ECU samples
			TAKE_DATA_IF_AVAIL(eDA_ECU_ENGINE_SPEED, ecuSamp.engineSpeed_rpm)
			TAKE_DATA_IF_AVAIL(eDA_ECU_TPS, ecuSamp.tps)
			TAKE_DATA_IF_AVAIL(eDA_ECU_BOOST, ecuSamp.boost_psi)

			// merge in Track samples
			TAKE_DATA_IF_AVAIL(eDA_CALC_ON_TRACK_LATLON, calcSamp.onTrackLL)
			TAKE_DATA_IF_AVAIL(eDA_CALC_LAP, calcSamp.lap)
			TAKE_DATA_IF_AVAIL(eDA_CALC_LAP_TIME_OFFSET, calcSamp.lapTimeOffset)
			TAKE_DATA_IF_AVAIL(eDA_CALC_SECTOR, calcSamp.sector)
			TAKE_DATA_IF_AVAIL(eDA_CALC_SECTOR_TIME_OFFSET, calcSamp.sectorTimeOffset)
		}

		// make sure everything was merged (future proofing logic in case fields are added)
		if (dataToTake.any())
		{
			spdlog::warn(
				"unmerged GoPro samples remain. seems like {}() implementation is incomplete.",
				__func__);
		}
		if (dataToTake.any())
		{
			spdlog::warn(
				"unmerged ECU samples remain. seems like {}() implementation is incomplete.",
				__func__);
		}
		if (dataToTake.any())
		{
			spdlog::warn(
				"unmerged Track samples remain. seems like {}() implementation is incomplete.",
				__func__);
		}

		return mergedSamples;
	}

	bool
	DataSource::subclassApplyModifications(
        bool unnecessaryIsOkay)
	{
		return true;
	}

	bool
	DataSource::subclassSaveModifications(
        bool unnecessaryIsOkay)
	{
		return true;
	}

	DataSourceManager::DataSourceManager()
	 : ModifiableObject("DataSourceManager", false, true)
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
		auto &src = sources_.at(idx);
		src->sourceName_ = name;
		src->markObjectModified(false,true);
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
	DataSourceManager::subclassApplyModifications(
        bool unnecessaryIsOkay)
	{
		return true;
	}

	bool
	DataSourceManager::subclassSaveModifications(
        bool unnecessaryIsOkay)
	{
		for (auto &source : sources_)
		{
			source->saveModifications(unnecessaryIsOkay);
		}
		
		return true;
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
			dataSrc->addObserver(this);
		}
		return dataSrc != nullptr;
	}

	void
	DataSourceManager::onModified(
		ModifiableObject *modifiable)
	{
		markObjectModified(
			modifiable->hasApplyableModifications(),
			modifiable->hasSavableModifications());
	}

}