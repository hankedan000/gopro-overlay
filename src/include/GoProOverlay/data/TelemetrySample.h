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
		int lap;
		double lapTimeOffset;
		int sector;
		double sectorTimeOffset;

	};

	using TelemetrySamples = std::vector<TelemetrySample>;
	using TelemetrySamplesPtr = std::shared_ptr<TelemetrySamples>;
}