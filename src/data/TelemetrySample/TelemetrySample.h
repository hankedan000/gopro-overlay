#pragma once

#include <GoProTelem/SampleTypes.h>
#include <memory>
#include <vector>

namespace gpo
{
	class TelemetrySample
	{
	public:
		gpt::CombinedSample gpSamp;
		size_t lap;
		double lapTimeOffset;
		size_t sector;

	};

	using TelemetrySamples = std::vector<TelemetrySample>;
	using TelemetrySamplesPtr = std::shared_ptr<TelemetrySamples>;
}