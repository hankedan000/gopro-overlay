#pragma once

#include "TelemetrySeeker.h"
#include "TelemetrySource.h"
#include "VideoSource.h"

namespace gpo
{
	class Data
	{
	public:
		TelemetrySeekerPtr seeker;
		TelemetrySourcePtr telemSrc;
		VideoSourcePtr videoSrc;
	};

	class DataFactory
	{
	public:
		static
		bool
		loadData(
			const std::string &videoFile,
			Data &data);

	};
}