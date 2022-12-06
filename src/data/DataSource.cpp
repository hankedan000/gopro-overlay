#include "GoProOverlay/data/DataSource.h"

#include <filesystem>

#include <GoProTelem/GoProTelem.h>
#include <GoProOverlay/utils/DataProcessingUtils.h>

namespace gpo
{
	DataSource::DataSource()
	 : seeker(nullptr)
	 , telemSrc(nullptr)
	 , videoSrc(nullptr)
	 , vCapture_()
	 , samples_()
	 , sourceName_("")
	 , originFile_("")
	 , datumTrack_(nullptr)
	{
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

		auto newSrc = DataSourcePtr(new DataSource());
		newSrc->vCapture_ = vCap;
		newSrc->samples_ = TelemetrySamplesPtr(new TelemetrySamples());
		newSrc->samples_->resize(videoTelem.size());
		for (size_t i=0; i<videoTelem.size(); i++)
		{
			newSrc->samples_->at(i).gpSamp = videoTelem.at(i);
		}

		newSrc->seeker = TelemetrySeekerPtr(new TelemetrySeeker(newSrc));
		newSrc->telemSrc = TelemetrySourcePtr(new TelemetrySource(newSrc));
		newSrc->videoSrc = VideoSourcePtr(new VideoSource(newSrc));

		return newSrc;
	}

	DataSourcePtr
	DataSource::makeDataFromTelemetry(
		const gpo::TelemetrySamples &tSamps)
	{
		auto newSrc = DataSourcePtr(new DataSource());
		newSrc->samples_ = TelemetrySamplesPtr(new TelemetrySamples());
		newSrc->samples_->resize(tSamps.size());
		newSrc->samples_->assign(tSamps.begin(),tSamps.end());

		newSrc->seeker = TelemetrySeekerPtr(new TelemetrySeeker(newSrc));
		newSrc->telemSrc = TelemetrySourcePtr(new TelemetrySource(newSrc));
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
			dataSrc->originFile_ = filepath;
			sources_.push_back(dataSrc);
		}
		return dataSrc != nullptr;
	}

}