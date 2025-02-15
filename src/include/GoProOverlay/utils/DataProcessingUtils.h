#pragma once

#include <array>
#include "GoProOverlay/data/TelemetrySample.h"
#include "GoProOverlay/data/TrackDataObjects.h"
#include "GoProOverlay/utils/RingFIFO.hpp"
#include <vector>

namespace constants
{
	inline const double GRAVITY = 9.80665;
}

static constexpr size_t VEC3_X = 0;
static constexpr size_t VEC3_Y = 1;
static constexpr size_t VEC3_Z = 2;

namespace utils
{
	
	float
	magnitude(
		const cv::Vec3f &v);
	
	cv::Vec3f
	normalize(
		const cv::Vec3f &v);

	float
	dot(
		const cv::Vec3f &a,
		const cv::Vec3f &b);
	
	/**
	 * @param[in] vec
	 * Vector to project.
	 * 
	 * @param[in] vecOnto
	 * Vector to project onto. Must be normalized.
	 * 
	 * @return
	 * The projected vector of 'a' onto 'b'
	 */
	cv::Vec3f
	projection(
		const cv::Vec3f &vec,
		const cv::Vec3f &vecOnto);

	/**
	 * Calculates the vehicle's lateral & longitudinal acceleration vectors.
	 * 
	 * @param[in] tSamps
	 * Telemetery samples to process
	 * 
	 * @param[in] avail
	 * The data that's available in the samples
	 * 
	 * @param[out] latDir
	 * The computed lateral direction vector
	 * 
	 * @param[out] lonDir
	 * The computed longitudinal direction vector
	 * 
	 * @return
	 * true if the calculation was successful, false otherwise
	 */
	bool
	computeVehicleDirectionVectors(
		gpo::TelemetrySamplesPtr tSamps,
		const gpo::DataAvailableBitSet &avail,
		cv::Vec3f &latDir,
		cv::Vec3f &lonDir);

	bool
	computeVehicleAcceleration(
		gpo::TelemetrySamplesPtr tSamps,
		gpo::DataAvailableBitSet &avail,
		const cv::Vec3f &latDir,
		const cv::Vec3f &lonDir);

	bool
	computeTrackTimes(
		const std::shared_ptr<const gpo::Track> &track,
		gpo::TelemetrySamplesPtr tSamps,
		gpo::DataAvailableBitSet &avail);

	template <typename T>
	void
	smoothMovingAvg(
		const void *inVector,
		void *outVector,
		const size_t nElements,
		size_t windowSize,
		const size_t inStride = sizeof(T),
		const size_t outStride = sizeof(T))
	{
		// ensure window size is odd
		if (windowSize % 2 == 0)
		{
			windowSize++;
		}

		std::vector<T> windowBuffer(windowSize);
		RingFIFO windowFIFO(windowBuffer.data(), windowBuffer.size());
		T windowSum = 0;
		const size_t halfWindow = (windowSize - 1) / 2;

		// calculate the start and end indices for the moving average window
		size_t endIdx = std::min(halfWindow, nElements - 1);

		// prefill averaging filter
		auto inElement = static_cast<const T *>(inVector);
		for (size_t i=0; i<=endIdx; i++)
		{
			windowFIFO.push(*inElement);
			windowSum += *inElement;
			inElement = (const T *)((const char *)(inElement) + inStride);
		}

		auto outElement = static_cast<T *>(outVector);
		size_t i = 0;
		for (i=0; i<(nElements - halfWindow); i++)
		{
			// compute the mean and place in i'th position
			*outElement = windowSum / windowFIFO.size();
			outElement = reinterpret_cast<T *>(reinterpret_cast<char *>(outElement) + outStride);

			if (i < (nElements - halfWindow - 1))
			{
				if (windowFIFO.size() == windowSize)
				{
					windowSum -= windowFIFO.back();
					windowFIFO.pop();
				}
				endIdx++;
				auto endElement = reinterpret_cast<const T *>(static_cast<const char *>(inVector) + endIdx * inStride);
				windowSum += *endElement;
				windowFIFO.push(*endElement);
			}
		}

		while (i++ < nElements)
		{
			windowSum -= windowFIFO.back();
			windowFIFO.pop();
			*outElement = windowSum / windowFIFO.size();
			outElement = reinterpret_cast<T *>(reinterpret_cast<char *>(outElement) + outStride);
		}
	}

	template <typename STRUCT_T, typename FIELD_T, size_t N_FIELDS>
	void
	smoothMovingAvgStructured(
		const STRUCT_T *inVector,
		STRUCT_T *outVector,
		const std::array<size_t, N_FIELDS> &inFieldOffsets,
		const std::array<size_t, N_FIELDS> &outFieldOffsets,
		size_t nElements,
		size_t windowSize)
	{
		for (size_t ff=0; ff<N_FIELDS; ff++)
		{
			smoothMovingAvg<FIELD_T>(
				(const void *)((const char *)(inVector) + inFieldOffsets[ff]),
				(void *)((char *)(outVector) + outFieldOffsets[ff]),
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