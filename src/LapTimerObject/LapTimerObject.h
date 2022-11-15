#pragma once

#include <GoProTelem/SampleTypes.h>
#include <opencv2/opencv.hpp>

#include "RenderedObject.h"

namespace gpo
{
	class LapTimerObject : public RenderedObject
	{
	public:
		LapTimerObject();

		bool
		init(
			int lapStartIdx,
			int lapFinishIdx = -1);

		void
		updateTimer(
			const std::vector<gpt::CombinedSample> &samples,
			size_t currentIndex);

	private:
		// background image
		cv::Mat bgImg_;

		cv::Scalar textColor_;

		int startIdx_;
		int finishIdx_;

		bool isLapFinished_;

	};
}