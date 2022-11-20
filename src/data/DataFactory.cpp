#include "GoProOverlay/data/DataFactory.h"

#include <GoProTelem/GoProTelem.h>

namespace gpo
{
	bool
	DataFactory::loadData(
		const std::string &videoFile,
		Data &data)
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

		data.seeker = TelemetrySeekerPtr(new TelemetrySeeker(
			telemSamps));
		data.telemSrc = TelemetrySourcePtr(new TelemetrySource(
			telemSamps,
			data.seeker));
		data.videoSrc = VideoSourcePtr(new VideoSource(
			vCap,
			data.seeker));

		return true;
	}
}