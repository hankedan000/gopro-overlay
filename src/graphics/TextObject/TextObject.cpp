#include "TextObject.h"

namespace gpo
{
	TextObject::TextObject()
	 : RenderedObject(0,0)
	 , text_()
	 , fontFace_(cv::FONT_HERSHEY_DUPLEX)
	 , scale_(1.0)
	 , color_(RGBA_COLOR(255,255,255,255))
	 , thickness_(2)
	{
	}

	void
	TextObject::setText(
		const std::string &text)
	{
		text_ = text;
	}

	void
	TextObject::setFontFace(
		int fontFace)
	{
		fontFace_ = fontFace;
	}

	void
	TextObject::setScale(
		double scale)
	{
		scale_ = scale;
	}

	void
	TextObject::setColor(
		cv::Scalar color)
	{
		color_ = color;
	}

	void
	TextObject::setThickness(
		int thickness)
	{
		thickness_ = thickness;
	}

	void
	TextObject::render(
		cv::Mat &intoImg,
		int originX, int originY)
	{
		cv::putText(
			intoImg,
			text_.c_str(),
			cv::Point(originX,originY),
			fontFace_,
			scale_,
			color_,
			thickness_);
	}

	void
	TextObject::render(
		cv::Mat &intoImg,
		int originX, int originY,
		cv::Size renderSize)
	{
		render(intoImg,originX,originY);
	}
}