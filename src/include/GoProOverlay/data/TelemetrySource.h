#pragma once

#include <memory>

#include "TelemetrySample.h"
#include "TelemetrySeeker.h"

namespace gpo
{
	// forward declaration
	class DataSource;
	using DataSourcePtr = std::shared_ptr<DataSource>;

	class TelemetrySource
	{
	public:
		explicit
		TelemetrySource(
			DataSourcePtr dSrc);

		std::string
		getDataSourceName() const;

		const TelemetrySample &
		at(
			size_t idx) const;

		TelemetrySample &
		at(
			size_t idx);

		size_t
		seekedIdx() const;

		TelemetrySeekerPtr
		seeker();

		size_t
		size() const;

		size_t
		size_bytes() const;

		double
		getTelemetryRate_hz() const;

		const DataAvailableBitSet &
		dataAvailable() const;

	private:
		std::weak_ptr<DataSource> dataSrc_;

	};

	using TelemetrySourcePtr = std::shared_ptr<TelemetrySource>;
}