#pragma once

#include <string>
#include <vector>

#include "RenderedObject.h"

namespace gpo
{
	class TextObject : public RenderedObject
	{
	public:
		TextObject();

		void
		setText(
			const std::string &text);

		void
		setFontFace(
			int fontFace);

		void
		setScale(
			double scale);

		void
		setColor(
			cv::Scalar color);

		void
		setThickness(
			int thickness);

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

	protected:
		std::string text_;
		int fontFace_;
		double scale_;
		cv::Scalar color_;
		int thickness_;

	};
}