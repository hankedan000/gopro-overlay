#pragma once

#include "TelemetryObject.h"

namespace gpo
{
	class LapTimerObject : public TelemetryObject
	{
	public:
		LapTimerObject();

		bool
		init(
			int lapStartIdx,
			int lapFinishIdx = -1);

		virtual
		void
		render(
			cv::Mat &intoImg,
			int originX, int originY,
			cv::Size renderSize);

	private:
		// background image
		cv::Mat bgImg_;

		cv::Scalar textColor_;

		int startIdx_;
		int finishIdx_;

		bool isLapFinished_;

	};
}