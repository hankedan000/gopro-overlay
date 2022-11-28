#pragma once

#include "GoProOverlay/graphics/RenderedObject.h"

namespace gpo
{
	class SpeedometerObject : public RenderedObject
	{
	public:
		SpeedometerObject();

		virtual
		DataSourceRequirements
		dataSourceRequirements() const override;

		virtual
		void
		render(
			cv::Mat &intoImg,
			int originX, int originY,
			cv::Size renderSize) override;

	private:

	};
}