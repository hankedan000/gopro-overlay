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

	private:
		DataSourcePtr dataSrc_;

	};

	using TelemetrySourcePtr = std::shared_ptr<TelemetrySource>;
}