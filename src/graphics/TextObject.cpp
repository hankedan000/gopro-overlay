#include "GoProOverlay/graphics/TextObject.h"

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

	std::string
	TextObject::typeName() const
	{
		return "TextObject";
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
	TextObject::render()
	{
		// do no rendering. we draw text directly into the image in drawInto()
	}

	void
	TextObject::drawInto(
		cv::UMat &intoImg,
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
	TextObject::drawInto(
		cv::UMat &intoImg,
		int originX, int originY,
		cv::Size renderSize)
	{
		drawInto(intoImg,originX,originY);
	}

	YAML::Node
	TextObject::subEncode() const
	{
		YAML::Node node;
		node["text"] = text_;
		node["fontFace"] = fontFace_;
		node["scale"] = scale_;
		node["color"] = color_;
		node["thickness"] = thickness_;
		return node;
	}

	bool
	TextObject::subDecode(
		const YAML::Node& node)
	{
		YAML_TO_FIELD(node,"text",text_);
		YAML_TO_FIELD(node,"fontFace",fontFace_);
		YAML_TO_FIELD(node,"scale",scale_);
		YAML_TO_FIELD(node,"color",color_);
		YAML_TO_FIELD(node,"thickness",thickness_);
		return true;
	}

}