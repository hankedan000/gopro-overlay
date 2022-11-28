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
			TelemetrySamplesPtr samples,
			TelemetrySeekerPtr seeker,
			DataSourcePtr dSrc = nullptr);

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

		size_t
		size() const;

	private:
		TelemetrySamplesPtr samples_;
		TelemetrySeekerPtr seeker_;
		DataSourcePtr dataSrc_;

	};

	using TelemetrySourcePtr = std::shared_ptr<TelemetrySource>;
}