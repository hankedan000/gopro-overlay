#pragma once

#include <string>
#include <vector>

#include "GoProOverlay/graphics/RenderedObject.h"

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

		void
		drawInto(
			cv::UMat &intoImg,
			int originX, int originY) override;

		void
		drawInto(
			cv::UMat &intoImg,
			int originX, int originY,
			cv::Size renderSize) override;

	protected:
		void
		subRender() override;

		YAML::Node
		subEncode() const override;

		bool
		subDecode(
			const YAML::Node& node) override;

	protected:
		std::string text_;
		int fontFace_;
		double scale_;
		cv::Scalar color_;
		int thickness_;

	};
}