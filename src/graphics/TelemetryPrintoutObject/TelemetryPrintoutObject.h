#pragma once

#include "TelemetryObject.h"

namespace gpo
{
	class TelemetryPrintoutObject : public TelemetryObject
	{
	public:
		TelemetryPrintoutObject();

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
			int originX, int originY);

		virtual
		void
		render(
			cv::Mat &intoImg,
			int originX, int originY,
			cv::Size renderSize);

	private:
		int fontFace_;
		cv::Scalar fontColor_;

	};
}