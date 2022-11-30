#include "GoProOverlay/data/TelemetrySource.h"
#include "GoProOverlay/data/DataSource.h"

#include <stdexcept>

namespace gpo
{
	TelemetrySource::TelemetrySource(
		DataSourcePtr dSrc)
	 : dataSrc_(dSrc)
	{
	}

	std::string
	TelemetrySource::getDataSourceName() const
	{
		if (dataSrc_)
		{
			return dataSrc_->getSourceName();
		}
		return "SOURCE_UNKNOWN";
	}

	const TelemetrySample &
	TelemetrySource::at(
		size_t idx) const
	{
		return dataSrc_->samples_->at(idx);
	}

	TelemetrySample &
	TelemetrySource::at(
		size_t idx)
	{
		return dataSrc_->samples_->at(idx);
	}

	size_t
	TelemetrySource::seekedIdx() const
	{
		return dataSrc_->seeker->seekedIdx();
	}

	TelemetrySeekerPtr
	TelemetrySource::seeker()
	{
		return dataSrc_->seeker;
	}


	size_t
	TelemetrySource::size() const
	{
		return dataSrc_->samples_->size();
	}
}