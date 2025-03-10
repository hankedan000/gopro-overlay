#include "GoProOverlay/graphics/FrictionCircleObject.h"

#include <opencv2/imgproc.hpp>
#include <tracy/Tracy.hpp>

#include "GoProOverlay/utils/OpenCV_Utils.h"

namespace gpo
{

	const int F_CIRCLE_RENDER_WIDTH = 480;
	const int F_CIRCLE_RENDER_HEIGHT = 480;

	FrictionCircleObject::FrictionCircleObject()
	 : RenderedObject("FrictionCircleObject",F_CIRCLE_RENDER_WIDTH,F_CIRCLE_RENDER_HEIGHT)
	 , outlineImg_(F_CIRCLE_RENDER_HEIGHT,F_CIRCLE_RENDER_WIDTH,CV_8UC4,RGBA_COLOR(0,0,0,0))
	 , tailLength_(0)
	 , radius_px_(200)
	 , margin_px_(40)
	 , center_(radius_px_+margin_px_,radius_px_+margin_px_)
	 , borderColor_(RGBA_COLOR(2,155,250,255))
	 , tailColor_(RGBA_COLOR(2,155,250,255))
	 , currentDotColor_(RGBA_COLOR(255,255,255,255))
	{
		redrawOutline();
	}

	DataSourceRequirements
	FrictionCircleObject::dataSourceRequirements() const
	{
		return DataSourceRequirements(0,1,0);
	}

	void
	FrictionCircleObject::setTailLength(
		size_t tailLength)
	{
		tailLength_ = tailLength;
		markNeedsRedraw();
		markObjectModified(false,true);
	}

	size_t
	FrictionCircleObject::getTailLength() const
	{
		return tailLength_;
	}

	void
	FrictionCircleObject::setBorderColor(
		cv::Scalar rgbColor)
	{
		borderColor_ = rgbColor;
		redrawOutline();
		markNeedsRedraw();
		markObjectModified(false,true);
	}

	cv::Scalar
	FrictionCircleObject::getBorderColor() const
	{
		return borderColor_;
	}

	void
	FrictionCircleObject::setTailColor(
		cv::Scalar rgbColor)
	{
		tailColor_ = rgbColor;
		markNeedsRedraw();
		markObjectModified(false,true);
	}

	cv::Scalar
	FrictionCircleObject::getTailColor() const
	{
		return tailColor_;
	}

	void
	FrictionCircleObject::setCurrentDotColor(
		cv::Scalar rgbColor)
	{
		currentDotColor_ = rgbColor;
		markNeedsRedraw();
		markObjectModified(false,true);
	}

	cv::Scalar
	FrictionCircleObject::getCurrentDotColor() const
	{
		return currentDotColor_;
	}

	void
	FrictionCircleObject::subRender()
	{
		ZoneScopedN("FrictionCircleObject::subRender()");
		outlineImg_.copyTo(outImg_);
		if ( ! requirementsMet())
		{
			return;
		}

		auto telemSrc = tSources_.front();

		// draw tail
		int startIdx = telemSrc->seekedIdx() - tailLength_;
		if (startIdx < 0)
		{
			startIdx = 0;
		}
		for (size_t i=startIdx; i<=telemSrc->seekedIdx() && i<telemSrc->size(); i++)
		{
			bool isLast = i == telemSrc->seekedIdx();
			auto color = (isLast ? currentDotColor_ : tailColor_);
			int dotRadius = (isLast ? 20 : 6);
			const auto &vehiAccl = telemSrc->at(i).calcSamp.vehiAccl;

			auto drawPoint = cv::Point(
				vehiAccl.lat_g * radius_px_ + center_.x,
				vehiAccl.lon_g * radius_px_ + center_.y);
			cv::circle(outImg_,drawPoint,dotRadius,color,cv::FILLED);

			if (isLast)
			{
				double netG = std::sqrt((vehiAccl.lat_g*vehiAccl.lat_g) + (vehiAccl.lon_g*vehiAccl.lon_g));
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

	YAML::Node
	FrictionCircleObject::subEncode() const
	{
		YAML::Node node;

		node["tailLength"] = tailLength_;
		node["borderColor"] = borderColor_;
		node["tailColor"] = tailColor_;
		node["currentDotColor"] = currentDotColor_;

		return node;
	}

	bool
	FrictionCircleObject::subDecode(
		const YAML::Node& node)
	{
		tailLength_ = node["tailLength"].as<size_t>();
		borderColor_ = node["borderColor"].as<cv::Scalar>();
		tailColor_ = node["tailColor"].as<cv::Scalar>();
		currentDotColor_ = node["currentDotColor"].as<cv::Scalar>();

		return true;
	}

	void
	FrictionCircleObject::redrawOutline()
	{
		outlineImg_.setTo(RGBA_COLOR(0,0,0,0));

		// add a grey translucent background
		int bgWidth = outlineImg_.size().width;
		int bgHeight = outlineImg_.size().height;
		cv::rounded_rectangle(
			outlineImg_,
			cv::Point(0,0),
			cv::Point(bgWidth,bgHeight),
			BACKGROUND_COLOR,
			cv::FILLED,
			cv::LINE_AA,
			BACKGROUND_RADIUS);

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
	}

}