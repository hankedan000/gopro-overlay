#pragma once

#include "GoProOverlay/data/TrackDataObjects.h"

namespace utils
{
	bool
	computeTrackTimes(
		const gpo::Track *track,
		gpo::TelemetrySamplesPtr tSamps);

	template <typename T>
	class RingFIFO
	{
	public:
		RingFIFO(
			T *buffer,
			size_t capacity)
		 : buffer_(buffer)
		 , capacity_(capacity)
		 , size_(0)
		 , head_(0)
		 , tail_(0)
		{
			if (capacity == 0)
			{
				throw std::runtime_error("capacity can't be zero");
			}
		}

		T front() const
		{
			return buffer_[head_];
		}

		T back() const
		{
			return buffer_[tail_];
		}

		bool empty() const
		{
			return size_ == 0;
		}

		size_t size() const
		{
			return size_;
		}

		size_t capacity() const
		{
			return capacity_;
		}

		void push(const T &value)
		{
			if (size_ == 0)
			{
				// buffer empty. safe to store at current head location.
				buffer_[head_] = value;
				size_ = 1;
			}
			else if (size_ == capacity_)
			{
				// buffer is full. move head and tail forward and store value in the
				// tail's previous location. effectively deletes oldest value.
				head_ = tail_;
				tail_ = nextIndex(tail_);
				buffer_[head_] = value;
			}
			else
			{
				// buffer not empty, but not full yet. jump to next head and store there.
				head_ = nextIndex(head_);
				buffer_[head_] = value;
				size_++;
			}
		}

		void pop()
		{
			if (size_ == 0)
			{
				// do nothing
			}
			else
			{
				tail_ = nextIndex(tail_);
				size_--;
			}
		}

	private:
		size_t nextIndex(size_t index) const
		{
			index++;
			if (index >= capacity_)
				return 0;
			return index;
		}

	private:
		T *buffer_;
		size_t capacity_;
		size_t size_;
		size_t head_;// front
		size_t tail_;// back

	};

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
}