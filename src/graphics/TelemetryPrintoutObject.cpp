#include "GoProOverlay/graphics/TelemetryPrintoutObject.h"

namespace gpo
{

	const int PRINTOUT_RENDERED_WIDTH = 1000;
	const int PRINTOUT_RENDERED_HEIGHT = 200;

	TelemetryPrintoutObject::TelemetryPrintoutObject()
	 : RenderedObject(PRINTOUT_RENDERED_WIDTH,PRINTOUT_RENDERED_HEIGHT)
	 , fontFace_(cv::FONT_HERSHEY_DUPLEX)
	 , fontColor_(RGBA_COLOR(0,255,0,255))
	{
	}

	std::string
	TelemetryPrintoutObject::typeName() const
	{
		return "TelemetryPrintoutObject";
	}

	DataSourceRequirements
	TelemetryPrintoutObject::dataSourceRequirements() const
	{
		return DataSourceRequirements(0,1,0);
	}

	void
	TelemetryPrintoutObject::setFontFace(
		int face)
	{
		fontFace_ = face;
	}

	void
	TelemetryPrintoutObject::setFontColor(
		cv::Scalar color)
	{
		fontColor_ = color;
	}

	void
	TelemetryPrintoutObject::render(
		cv::Mat &intoImg,
		int originX, int originY)
	{
		render(intoImg,originX,originY,outImg_.size());
	}

	void
	TelemetryPrintoutObject::render(
		cv::Mat &intoImg,
		int originX, int originY,
		cv::Size renderSize)
	{
		if ( ! requirementsMet())
		{
			return;
		}
		auto telemSrc = tSources_.front();
		auto frameIdx = telemSrc->seekedIdx();
		auto telemSamp = telemSrc->at(frameIdx);

		outImg_.setTo(RGBA_COLOR(0,0,0,0));

		char tmpStr[1024];
		sprintf(tmpStr,"frameIdx: %ld",frameIdx);
		cv::putText(
			outImg_, //target image
			tmpStr, //text
			cv::Point(10, 30), // position
			fontFace_,// font face
			1.0,// font scale
			fontColor_, //font color
			1);// thickness

		sprintf(tmpStr,"time_offset: %0.3fs",telemSamp.gpSamp.t_offset);
		cv::putText(
			outImg_, //target image
			tmpStr, //text
			cv::Point(10, 30 * 2), // position
			fontFace_,// font face
			1.0,// font scale
			fontColor_, //font color
			1);// thickness

		sprintf(tmpStr,"accl: %s",telemSamp.gpSamp.accl.toString().c_str());
		cv::putText(
			outImg_, //target image
			tmpStr, //text
			cv::Point(10, 30 * 3), // position
			fontFace_,// font face
			1.0,// font scale
			fontColor_, //font color
			1);// thickness

		sprintf(tmpStr,"gps: %s",telemSamp.gpSamp.gps.coord.toString().c_str());
		cv::putText(
			outImg_, //target image
			tmpStr, //text
			cv::Point(10, 30 * 4), // position
			fontFace_,// font face
			1.0,// font scale
			fontColor_, //font color
			1);// thickness

		sprintf(tmpStr,"altitude: %fm",telemSamp.gpSamp.gps.altitude);
		cv::putText(
			outImg_, //target image
			tmpStr, //text
			cv::Point(10, 30 * 5), // position
			fontFace_,// font face
			1.0,// font scale
			fontColor_, //font color
			1);// thickness

		RenderedObject::render(intoImg,originX,originY,renderSize);
	}

	YAML::Node
	TelemetryPrintoutObject::subEncode() const
	{
		YAML::Node node;
		node["fontFace"] = fontFace_;
		node["fontColor"] = fontColor_;
		return node;
	}

	bool
	TelemetryPrintoutObject::subDecode(
		const YAML::Node& node)
	{
		YAML_TO_FIELD(node,"fontFace",fontFace_);
		YAML_TO_FIELD(node,"fontColor",fontColor_);
		return true;
	}

}