#pragma once

#include "GoProOverlay/graphics/RenderedObject.h"

namespace gpo
{
	class TelemetryPrintoutObject : public RenderedObject
	{
	public:
		TelemetryPrintoutObject();

		virtual
		DataSourceRequirements
		dataSourceRequirements() const override;

		void
		setFontFace(
			int face);

		void
		setFontColor(
			cv::Scalar color);

		virtual
		void
		render(
			cv::Mat &intoImg,
			int originX, int originY) override;

		virtual
		void
		render(
			cv::Mat &intoImg,
			int originX, int originY,
			cv::Size renderSize) override;

	private:
		int fontFace_;
		cv::Scalar fontColor_;

	};
}