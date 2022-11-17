#include "FrictionCircleObject.h"

namespace gpo
{

	const int F_CIRCLE_RENDER_WIDTH = 480;
	const int F_CIRCLE_RENDER_HEIGHT = 480;

	FrictionCircleObject::FrictionCircleObject()
	 : RenderedObject(F_CIRCLE_RENDER_WIDTH,F_CIRCLE_RENDER_HEIGHT)
	 , outlineImg_(F_CIRCLE_RENDER_HEIGHT,F_CIRCLE_RENDER_WIDTH,CV_8UC4,RGBA_COLOR(0,0,0,0))
	 , radius_px_(200)
	 , margin_px_(40)
	 , center_(radius_px_+margin_px_,radius_px_+margin_px_)
	 , borderColor_(RGBA_COLOR(2,155,250,255))
	 , currentDotColor_(RGBA_COLOR(255,255,255,255))
	{
	}

	bool
	FrictionCircleObject::init()
	{
		// draw outer circle
		cv::circle(outlineImg_,center_,radius_px_,borderColor_,8);

		cv::putText(
			outlineImg_, // target image
			"1.0g", // text
			cv::Point(center_.x+(radius_px_+20)*0.707,center_.y-(radius_px_+20)*0.707),// bottom-right (0.707 is sin(45deg))
			cv::FONT_HERSHEY_DUPLEX,// font face
			1.0,// font scale
			borderColor_, // font color
			2);// thickness

		return true;
	}

	void
	FrictionCircleObject::updateTail(
		const std::vector<gpt::CombinedSample> &samples,
		size_t currentIndex,
		size_t tailLength)
	{
		outlineImg_.copyTo(outImg_);

		// draw trail
		int startIdx = currentIndex - tailLength;
		if (startIdx < 0)
		{
			startIdx = 0;
		}
		for (size_t i=startIdx; i<=currentIndex && i<samples.size(); i++)
		{
			bool isLast = i == currentIndex;
			auto color = (isLast ? currentDotColor_ : RGBA_COLOR(247,162,2,255));
			int dotRadius = (isLast ? 20 : 6);
			const auto &accl = samples.at(i).accl;

			auto drawPoint = cv::Point(
				(accl.x / -9.8) * radius_px_ + center_.x,
				(accl.z / 9.8) * radius_px_ + center_.y);
			cv::circle(outImg_,drawPoint,dotRadius,color,cv::FILLED);

			if (isLast)
			{
				double netG = std::sqrt((accl.x*accl.x) + (accl.z*accl.z)) / 9.8;
				char tmpStr[1024];
				sprintf(tmpStr,"%.1fg",netG);
				cv::putText(
					outImg_, // target image
					tmpStr, // text
					cv::Point(center_.x+(radius_px_+20)*0.707,center_.y+(radius_px_+30)*0.707),// bottom-right (0.707 is sin(45deg))
					cv::FONT_HERSHEY_DUPLEX,// font face
					1.0,// font scale
					currentDotColor_, // font color
					2);// thickness
			}
		}
	}

}