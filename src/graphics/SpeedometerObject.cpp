#include "GoProOverlay/graphics/SpeedometerObject.h"

namespace gpo
{

	const int SPEEDOMETER_RENDERED_WIDTH = 600;
	const int SPEEDOMETER_RENDERED_HEIGHT = 200;

	SpeedometerObject::SpeedometerObject()
	 : RenderedObject(SPEEDOMETER_RENDERED_WIDTH,SPEEDOMETER_RENDERED_HEIGHT)
	{
	}

	DataSourceRequirements
	SpeedometerObject::dataSourceRequirements() const
	{
		return DataSourceRequirements(0,1,0);
	}

	void
	SpeedometerObject::render(
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

		int speedMPH = round(telemSamp.gpSamp.gps.speed2D * 2.23694);// m/s to mph
		char tmpStr[1024];
		sprintf(tmpStr,"%2dmph",speedMPH);
		cv::putText(
			outImg_, // target image
			tmpStr, // text
			cv::Point(0,SPEEDOMETER_RENDERED_HEIGHT - 30), // position
			cv::FONT_HERSHEY_DUPLEX,// font face
			2.0 * 2,// font scale
			RGBA_COLOR(2,155,250,255), // font color
			2 * 2);// thickness

		// let base class perform its own rendering too
		RenderedObject::render(intoImg,originX,originY,renderSize);
	}
}