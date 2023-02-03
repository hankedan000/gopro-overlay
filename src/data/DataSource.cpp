#include "GoProOverlay/data/DataSource.h"

#include <filesystem>
#include <spdlog/spdlog.h>

#include <GoProTelem/GoProTelem.h>
#include <GoProOverlay/utils/DataProcessingUtils.h>

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

		bool okay = utils::computeTrackTimes(datumTrack_,samples_);

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

	double
	DataSource::getTelemetryRate_hz() const
	{
		if ( ! hasTelemetry() || samples_->size() < 2)
		{
			return 0.0;
		}
		return samples_->back().t_offset / (samples_->size() - 1);
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

#if 0
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
			bool found = gpt::findLerpIndex(takeIdx,in,outTime_sec);

			if (found)
			{
				const auto &sampA = in.at(takeIdx);
				const auto &sampB = in.at(takeIdx+1);
				const double dt = sampB.t_offset - sampA.t_offset;
				const double ratio = (outTime_sec - sampA.t_offset) / dt;
				lerp(out.at(outIdx),sampA,sampB,ratio);
			}
			else if (takeIdx == 0)
			{
				out.at(outIdx) = in.at(takeIdx);
			}
			else
			{
				out.at(outIdx) = in.back();
			}
			outTime_sec += outDt_sec;
		}
#endif
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
		auto videoTelem = gpt::getCombinedSamples(mp4);
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
			newSrc->samples_->at(i).gpSamp = videoTelem.at(i);
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
		auto res = utils::readMegaSquirtLog(logFile,ecuTelem);
		if ( ! res.first)
		{
			return nullptr;
		}

		auto newSrc = std::make_shared<DataSource>();
		newSrc->originFile_ = logFile;
		newSrc->samples_ = std::make_shared<TelemetrySamples>();
		newSrc->samples_->resize(ecuTelem.size());
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