#include "GoProOverlay/data/DataSource.h"

#include <GoProTelem/GoProTelem.h>
#include <GoProOverlay/utils/DataProcessingUtils.h>

namespace gpo
{
	DataSource::DataSource()
	 : seeker(nullptr)
	 , telemSrc(nullptr)
	 , videoSrc(nullptr)
	 , datumTrack_(nullptr)
	 , lapCount_(0)
	{
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
		lapCount_ = 0;
		if (datumTrack_ == nullptr)
		{
			// nothing to process
			return true;
		}

		bool okay = utils::computeTrackTimes(datumTrack_,telemSrc);
		if (okay)
		{
			// determine lap count by starting at end of data and finding first valid lap
			for (size_t i=(telemSrc->size()-1); i>=0; i++)
			{
				const auto &samp = telemSrc->at(i);
				if (samp.lap != -1)
				{
					lapCount_ = samp.lap;
					break;
				}
			}
		}

		return okay;
	}

	int
	DataSource::lapCount() const
	{
		return lapCount_;
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

	bool
	loadDataFromVideo(
		const std::string &videoFile,
		DataSourcePtr &data)
	{
		gpt::MP4_Source mp4;
		mp4.open(videoFile);
		auto videoTelem = gpt::getCombinedSamples(mp4);
		if (videoTelem.empty())
		{
			return false;
		}
		cv::VideoCapture vCap(videoFile);
		if ( ! vCap.isOpened())
		{
			return false;
		}

		auto telemSamps = TelemetrySamplesPtr(new TelemetrySamples());
		telemSamps->resize(videoTelem.size());
		for (size_t i=0; i<videoTelem.size(); i++)
		{
			telemSamps->at(i).gpSamp = videoTelem.at(i);
		}

		data.reset(new DataSource());
		data->seeker = TelemetrySeekerPtr(new TelemetrySeeker(
			telemSamps));
		data->telemSrc = TelemetrySourcePtr(new TelemetrySource(
			telemSamps,
			data->seeker));
		data->videoSrc = VideoSourcePtr(new VideoSource(
			vCap,
			data->seeker));

		return true;
	}

	DataSourceManager::DataSourceManager()
	 : sources_()
	{
	}

	bool
	DataSourceManager::addVideo(
		const std::string &filepath)
	{
		for (const auto &entry : sources_)
		{
			if (entry.originFile == filepath)
			{
				// already imported
				return false;
			}
		}

		std::string name = "VideoSource" + std::to_string(sources_.size());
		return addVideoSourceWithName(filepath,name);
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
		sources_.at(idx).name = name;
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
		return sources_.at(idx).data;
	}

	const DataSourcePtr
	DataSourceManager::getSource(
		size_t idx) const
	{
		return sources_.at(idx).data;
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
			ySource["originFile"] = source.originFile;
			ySource["name"] = source.name;

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
			for (size_t ss=0; okay && ss<sources_.size(); ss++)
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
		DataSourcePtr data;
		bool loadOkay = loadDataFromVideo(filepath,data);
		if (loadOkay)
		{
			InternalSourceEntry newEntry;
			newEntry.originFile = filepath;
			newEntry.name = "";// TODO set from filename
			newEntry.data = data;
			sources_.push_back(newEntry);
		}
		return loadOkay;
	}

}