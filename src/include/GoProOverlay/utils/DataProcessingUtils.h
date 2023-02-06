#pragma once

#include "GoProOverlay/data/TelemetrySample.h"
#include "GoProOverlay/data/TrackDataObjects.h"
#include "GoProOverlay/utils/RingFIFO.hpp"

namespace utils
{

	bool
	computeTrackTimes(
		const gpo::Track *track,
		gpo::TelemetrySamplesPtr tSamps);

	template <typename T>
	void
	smoothMovingAvg(
		const void *inVector,
		void *outVector,
		size_t nElements,
		size_t windowSize,
		size_t inStride = sizeof(T),
		size_t outStride = sizeof(T))
	{
		// ensure window size is odd
		if (windowSize % 2 == 0)
		{
			windowSize++;
		}

		T windowBuffer[windowSize];
		RingFIFO windowFIFO(windowBuffer, windowSize);
		T windowSum = 0;
		size_t halfWindow = (windowSize - 1) / 2;

		// calculate the start and end indices for the moving average window
		size_t startIdx = 0;
		size_t endIdx = std::min(halfWindow, nElements - 1);

		// prefill averaging filter
		const T *inElement = (const T *)(inVector);
		for (size_t i=0; i<=endIdx; i++)
		{
			windowFIFO.push(*inElement);
			windowSum += *inElement;
			inElement = (const T *)((const char *)(inElement) + inStride);
		}

		T *outElement = (T *)(outVector);
		size_t i = 0;
		for (i=0; i<(nElements - halfWindow); i++)
		{
			// compute the mean and place in i'th position
			*outElement = windowSum / windowFIFO.size();
			outElement = (T *)((char *)(outElement) + outStride);

			if (i < (nElements - halfWindow - 1))
			{
				if (windowFIFO.size() == windowSize)
				{
					windowSum -= windowFIFO.back();
					windowFIFO.pop();

					startIdx++;
					endIdx++;
				}
				else
				{
					endIdx++;
				}
				const T *endElement = (const T *)((const char *)(inVector) + endIdx * inStride);
				windowSum += *endElement;
				windowFIFO.push(*endElement);
			}
		}

		while (i++ < nElements)
		{
			windowSum -= windowFIFO.back();
			windowFIFO.pop();
			*outElement = windowSum / windowFIFO.size();
			outElement = (T *)((char *)(outElement) + outStride);
		}
	}

	template <typename STRUCT_T, typename FIELD_T>
	void
	smoothMovingAvgStructured(
		const STRUCT_T *inVector,
		STRUCT_T *outVector,
		size_t *fieldOffsets,
		size_t nFields,
		size_t nElements,
		size_t windowSize)
	{
		for (size_t ff=0; ff<nFields; ff++)
		{
			smoothMovingAvg<FIELD_T>(
				(const void *)((const char *)(inVector) + fieldOffsets[ff]),
				(void *)((char *)(outVector) + fieldOffsets[ff]),
				nElements,
				windowSize,
				sizeof(STRUCT_T),
				sizeof(STRUCT_T));
		}
	}

	template <typename TimedSample_T>
	double
	getAvgRate(
		const std::vector<TimedSample_T> &samps)
	{
		if (samps.size() <= 1)
		{
			return 0.0;
		}
		double nCycles = samps.size() - 1;
		return nCycles / samps.back().t_offset;
	}

	void
	lerp(
		gpo::ECU_Sample &out,
		const gpo::ECU_Sample &a,
		const gpo::ECU_Sample &b,
		double ratio);

	void
	lerp(
		gpo::ECU_Sample &out,
		const gpo::ECU_TimedSample &a,
		const gpo::ECU_TimedSample &b,
		double ratio);

	void
	lerp(
		gpo::TelemetrySample &out,
		const gpo::TelemetrySample &a,
		const gpo::TelemetrySample &b,
		double ratio);

	void
	resample(
		std::vector<gpo::ECU_TimedSample> &out,
		const std::vector<gpo::ECU_TimedSample> &in,
		double outRate_hz);
}