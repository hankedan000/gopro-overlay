#pragma once

#include "GoProOverlay/graphics/RenderedObject.h"

namespace gpo
{
	class LapTimerObject : public RenderedObject
	{
	public:
		LapTimerObject();

		virtual
		DataSourceRequirements
		dataSourceRequirements() const override;

		bool
		init(
			int lapStartIdx,
			int lapFinishIdx = -1);

		virtual
		void
		render(
			cv::Mat &intoImg,
			int originX, int originY,
			cv::Size renderSize) override;

	private:
		// background image
		cv::Mat bgImg_;

		cv::Scalar textColor_;

		int startIdx_;
		int finishIdx_;

		bool isLapFinished_;

	};
}