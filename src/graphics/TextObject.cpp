#include "GoProOverlay/graphics/TextObject.h"

#include <opencv2/imgproc.hpp>
#include <tracy/Tracy.hpp>

namespace gpo
{
	TextObject::TextObject()
	 : RenderedObject("TextObject",1,1)// cv::UMat seg faults if matrix is sized [0,0] :(
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
		markNeedsRedraw();
		markObjectModified(false,true);
	}

	void
	TextObject::setFontFace(
		int fontFace)
	{
		fontFace_ = fontFace;
		markNeedsRedraw();
		markObjectModified(false,true);
	}

	void
	TextObject::setScale(
		double scale)
	{
		scale_ = scale;
		markNeedsRedraw();
		markObjectModified(false,true);
	}

	void
	TextObject::setColor(
		cv::Scalar color)
	{
		color_ = color;
		markNeedsRedraw();
		markObjectModified(false,true);
	}

	void
	TextObject::setThickness(
		int thickness)
	{
		thickness_ = thickness;
		markNeedsRedraw();
		markObjectModified(false,true);
	}

	void
	TextObject::drawInto(
		cv::UMat &intoImg,
		int originX, int originY)
	{
		ZoneScopedN("TextObject::drawInto()");
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
		cv::Size /* renderSize */)
	{
		drawInto(intoImg,originX,originY);
	}

	void
	TextObject::subRender()
	{
		// do no rendering. we draw text directly into the image in drawInto()
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