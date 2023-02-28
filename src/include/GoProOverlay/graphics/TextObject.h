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

		virtual
		std::string
		typeName() const override;

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
		drawInto(
			cv::UMat &intoImg,
			int originX, int originY);

		virtual
		void
		drawInto(
			cv::UMat &intoImg,
			int originX, int originY,
			cv::Size renderSize);

	protected:
		virtual
		void
		subRender() override;

		virtual
		YAML::Node
		subEncode() const override;

		virtual
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