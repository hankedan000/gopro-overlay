#pragma once

#include <GoProTelem/SampleTypes.h>
#include <opencv2/opencv.hpp>

#include "RenderedObject.h"

namespace gpo
{
	class FrictionCircleObject : public RenderedObject
	{
	public:
		FrictionCircleObject(
			int radius_px,
			int margin_px);

		bool
		init();

		void
		updateTail(
			const std::vector<gpt::CombinedSample> &samples,
			size_t currentIndex,
			size_t tailLength);

	private:
		// map outline
		cv::Mat outlineImg_;

		int radius_px_;
		int margin_px_;
		cv::Point center_;

	};
}