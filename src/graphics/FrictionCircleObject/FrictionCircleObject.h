#pragma once

#include "TelemetryObject.h"

namespace gpo
{
	class FrictionCircleObject : public TelemetryObject
	{
	public:
		FrictionCircleObject();

		void
		setTailLength(
			size_t tailLength);

		virtual
		void
		render(
			cv::Mat &intoImg,
			int originX, int originY,
			cv::Size renderSize);

	private:
		// map outline
		cv::Mat outlineImg_;

		size_t tailLength_;

		int radius_px_;
		int margin_px_;
		cv::Point center_;

		cv::Scalar borderColor_;
		cv::Scalar currentDotColor_;

	};
}