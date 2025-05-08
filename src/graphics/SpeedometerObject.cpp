#include "GoProOverlay/graphics/SpeedometerObject.h"

#include <opencv2/imgproc.hpp>
#include <tracy/Tracy.hpp>

#include "GoProOverlay/utils/OpenCV_Utils.h"

namespace gpo
{

	const int SPEEDOMETER_RENDERED_WIDTH = 600;
	const int SPEEDOMETER_RENDERED_HEIGHT = 200;

	SpeedometerObject::SpeedometerObject()
	 : RenderedObject("SpeedometerObject",SPEEDOMETER_RENDERED_WIDTH,SPEEDOMETER_RENDERED_HEIGHT)
	{
	}

	DataSourceRequirements
	SpeedometerObject::dataSourceRequirements() const
	{
		return DataSourceRequirements(0,1,0);
	}

	void
	SpeedometerObject::subRender()
	{
		ZoneScopedN("SpeedometerObject::subRender()");
		if ( ! requirementsMet())
		{
			return;
		}
		auto telemSrc = tSources_.front();
		auto frameIdx = telemSrc->seekedIdx();
		auto telemSamp = telemSrc->at(frameIdx);

		outImg_.setTo(RGBA_COLOR(0,0,0,0));

		// add a grey translucent background
		int bgWidth = outImg_.size().width;
		int bgHeight = outImg_.size().height;
		cv::rounded_rectangle(
			outImg_,
			cv::Point(0,0),
			cv::Point(bgWidth,bgHeight),
			BACKGROUND_COLOR,
			cv::FILLED,
			cv::LINE_AA,
			BACKGROUND_RADIUS);

		int speedMPH = round(telemSamp.gpSamp.gps.speed2D * 2.23694);// m/s to mph
		char tmpStr[1024];
		sprintf(tmpStr,"%3dmph",speedMPH);
		cv::putText(
			outImg_, // target image
			tmpStr, // text
			cv::Point(0,SPEEDOMETER_RENDERED_HEIGHT - 30), // position
			cv::FONT_HERSHEY_DUPLEX,// font face
			2.0 * 2,// font scale
			RGBA_COLOR(2,155,250,255), // font color
			2 * 2);// thickness
	}

	YAML::Node
	SpeedometerObject::subEncode() const
	{
		YAML::Node node;
		return node;
	}

	bool
	SpeedometerObject::subDecode(
		const YAML::Node & /* node */)
	{
		// nothing was encoded, so there's nothing to decode
		return true;
	}

}