#include "GoProOverlay/graphics/LapTimerObject.h"

#include <tracy/Tracy.hpp>

namespace gpo
{

	const int LAPTIMER_RENDERED_WIDTH = 600;
	const int LAPTIMER_RENDERED_HEIGHT = 200;

	LapTimerObject::LapTimerObject()
	 : RenderedObject("LapTimerObject",LAPTIMER_RENDERED_WIDTH,LAPTIMER_RENDERED_HEIGHT)
	 , bgImg_(LAPTIMER_RENDERED_HEIGHT,LAPTIMER_RENDERED_WIDTH,CV_8UC4,RGBA_COLOR(0,0,0,0))
	 , textColor_(RGBA_COLOR(255,255,255,255))
	 , lapTime_(0.0)
	{
	}

	DataSourceRequirements
	LapTimerObject::dataSourceRequirements() const
	{
		return DataSourceRequirements(0,1,0);
	}

	void
	LapTimerObject::subRender()
	{
		ZoneScopedN("LapTimerObject::subRender()");
		bgImg_.copyTo(outImg_);
		if ( ! requirementsMet())
		{
			return;
		}

		auto telemSrc = tSources_.front();
		const auto &trackData = telemSrc->at(telemSrc->seekedIdx()).calcSamp;
		if (trackData.lap != -1)
		{
			lapTime_ = trackData.lapTimeOffset;
		}

		char lapTimeStr[1024];
		sprintf(lapTimeStr,"Lap Time: %0.3fs", lapTime_);
		cv::putText(
			outImg_, // target image
			lapTimeStr, // text
			cv::Point(0,60),
			cv::FONT_HERSHEY_DUPLEX,// font face
			2.0,// font scale
			textColor_, //font color
			2);// thickness
	}

	YAML::Node
	LapTimerObject::subEncode() const
	{
		YAML::Node node;

		node["textColor"] = textColor_;

		return node;
	}

	bool
	LapTimerObject::subDecode(
		const YAML::Node& node)
	{
		YAML_TO_FIELD(node, "textColor", textColor_);
		return true;
	}

}