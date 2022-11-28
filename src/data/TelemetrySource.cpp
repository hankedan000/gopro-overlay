#include "GoProOverlay/data/TelemetrySource.h"
#include "GoProOverlay/data/DataSource.h"

#include <stdexcept>

namespace gpo
{
	TelemetrySource::TelemetrySource(
		TelemetrySamplesPtr samples,
		TelemetrySeekerPtr seeker,
		DataSourcePtr dSrc)
	 : samples_(samples)
	 , seeker_(seeker)
	 , dataSrc_(dSrc)
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
		return samples_->at(idx);
	}

	TelemetrySample &
	TelemetrySource::at(
		size_t idx)
	{
		return samples_->at(idx);
	}

	size_t
	TelemetrySource::seekedIdx() const
	{
		return seeker_->seekedIdx();
	}

	size_t
	TelemetrySource::size() const
	{
		return samples_->size();
	}
}