#pragma once

#include "GoProOverlay/graphics/TelemetryObject.h"

namespace gpo
{
	class SpeedometerObject : public TelemetryObject
	{
	public:
		SpeedometerObject();

		virtual
		void
		render(
			cv::Mat &intoImg,
			int originX, int originY,
			cv::Size renderSize);

	private:

	};
}