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
		auto dataSrcPtr = dataSrc_.lock();
		if (dataSrcPtr)
		{
			return dataSrcPtr->getSourceName();
		}
		return "SOURCE_UNKNOWN";
	}

	const TelemetrySample &
	TelemetrySource::at(
		size_t idx) const
	{
		return dataSrc_.lock()->samples_->at(idx);
	}

	TelemetrySample &
	TelemetrySource::at(
		size_t idx)
	{
		return dataSrc_.lock()->samples_->at(idx);
	}

	size_t
	TelemetrySource::seekedIdx() const
	{
		return dataSrc_.lock()->seeker->seekedIdx();
	}

	TelemetrySeekerPtr
	TelemetrySource::seeker()
	{
		return dataSrc_.lock()->seeker;
	}

	size_t
	TelemetrySource::size() const
	{
		return dataSrc_.lock()->samples_->size();
	}

	size_t
	TelemetrySource::size_bytes() const
	{
		return size() * sizeof(TelemetrySample);
	}

	double
	TelemetrySource::getTelemetryRate_hz() const
	{
		return dataSrc_.lock()->getTelemetryRate_hz();
	}

	const GoProDataAvailBitSet &
	TelemetrySource::gpDataAvail() const
	{
		return dataSrc_.lock()->gpDataAvail();
	}

	const ECU_DataAvailBitSet &
	TelemetrySource::ecuDataAvail() const
	{
		return dataSrc_.lock()->ecuDataAvail();
	}
}