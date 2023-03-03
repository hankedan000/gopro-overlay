#include "GoProOverlay/graphics/RenderedObject.h"

#include <spdlog/spdlog.h>

namespace gpo
{

	ModifiableDrawObject::ModifiableDrawObject(
		const std::string &className)
	 : ModifiableObject(className,true,false)
	 , needsRedraw_(false)
	 , observers_()
	{
	}

	ModifiableDrawObject::ModifiableDrawObject(
		const ModifiableDrawObject &other)
	 : ModifiableObject(other)
	 , needsRedraw_(other.needsRedraw_)
	 , observers_()// don't copy observers. they should remain bound only to 'other'.
	{
	}

	ModifiableDrawObject::~ModifiableDrawObject()
	{
	}

	void
	ModifiableDrawObject::markNeedsRedraw()
	{
		spdlog::trace("{} modified.", className());
		needsRedraw_ = true;
        for (auto &observer : observers_)
        {
            observer->onNeedsRedraw(this);
        }
	}

	bool
	ModifiableDrawObject::needsRedraw() const
	{
		return needsRedraw_;
	}

	void
	ModifiableDrawObject::addObserver(
		ModifiableDrawObjectObserver *observer)
	{
        if (observer == nullptr)
        {
            spdlog::warn("can't add a null ModifiableDrawObjectObserver. ignoring add.'");
            return;
        }
        observers_.insert(observer);
	}

	void
	ModifiableDrawObject::removeObserver(
		ModifiableDrawObjectObserver *observer)
	{
		observers_.erase(observer);
	}

	bool
	ModifiableDrawObject::subclassApplyModifications()
	{
		// nothing to apply
		return true;
	}

	bool
	ModifiableDrawObject::subclassSaveModifications()
	{
		// we don't support saving indivual RenderedObjects (should never get in here)
		return false;
	}

	void
	ModifiableDrawObject::clearNeedsRedraw()
	{
		needsRedraw_ = false;
	}

	RenderedObject::RenderedObject(
		const std::string &typeName,
		int width,
		int height)
	 : ModifiableDrawObject(typeName)
	 , typeName_(typeName)
	 , outImg_(height,width,CV_8UC4,cv::Scalar(0,0,0,0))
	 , visible_(true)
	 , boundingBoxVisible_(false)
	 , vSources_()
	 , tSources_()
	 , track_(nullptr)
	{
	}

	const std::string &
	RenderedObject::typeName() const
	{
		return typeName_;
	}

	const cv::UMat &
	RenderedObject::getImage() const
	{
		return outImg_;
	}

	void
	RenderedObject::render()
	{
		// call subclass's render method
		subRender();
		clearNeedsRedraw();
	}

	void
    RenderedObject::drawInto(
		cv::UMat &intoImg,
		int originX, int originY)
	{
        drawInto(intoImg,originX,originY,outImg_.size());
	}

	void
    RenderedObject::drawInto(
		cv::UMat &intoImg,
		int originX, int originY,
		cv::Size renderSize)
	{
		cv::UMat *imgToRender = &outImg_;
		if (renderSize.width != getNativeWidth() || renderSize.height != getNativeHeight())
		{
			ALPHA_SAFE_RESIZE(outImg_,scaledImg_,renderSize);
			imgToRender = &scaledImg_;
		}

		// clip the RenderedObject if it falls outside the destination image
		//
		// TODO we don't really need to perform this computation each frame.
		// we could just update the clipped region when the object is moved
		// and/or resized.
		cv::Rect srcROI(cv::Point(0,0), imgToRender->size());
		cv::Rect destROI(cv::Point(originX,originY), renderSize);
		if (originX < 0)
		{
			destROI.x = 0;
			destROI.width = renderSize.width - std::abs(originX);
			srcROI.x = std::abs(originX);
			srcROI.width = destROI.width;
		}
		if (originY < 0)
		{
			destROI.y = 0;
			destROI.height = renderSize.height - std::abs(originY);
			srcROI.y = std::abs(originY);
			srcROI.height = destROI.height;
		}
		const int overHangRight = (destROI.x + destROI.width) - intoImg.size().width;
		if (overHangRight > 0)
		{
			destROI.width -= overHangRight;
			srcROI.width = destROI.width;
		}
		const int overHangBottom = (destROI.y + destROI.height) - intoImg.size().height;
		if (overHangBottom > 0)
		{
			destROI.height -= overHangBottom;
			srcROI.height = destROI.height;
		}

		// check to see if object was clipped entirely (fully off screen)
		if (srcROI.width < 0 || srcROI.height < 0)
		{
			return;// don't bother rendering it
		}

		// draw final output to user image
		cv::Mat intoImgMat = intoImg.getMat(cv::AccessFlag::ACCESS_RW);
		cv::Mat imgToRenderMat = imgToRender->getMat(cv::AccessFlag::ACCESS_READ);
		cv::Mat4b srcMat = imgToRenderMat(srcROI);
		cv::Mat3b destMat = intoImgMat(destROI);
		const uint8_t alphaB = 255; // alpha in [0,255]
		for (int r = 0; r < destMat.rows; ++r)
		{
			for (int c = 0; c < destMat.cols; ++c)
			{
				auto vA = srcMat.at<cv::Vec4b>(r,c);
				// Blending
				const uint8_t alphaA = vA[3];
				cv::Vec3b &vB = destMat(r,c);
				vB[0] = (vA[0] * alphaA / 255) + (vB[0] * alphaB * (255 - alphaA) / (255*255));
				vB[1] = (vA[1] * alphaA / 255) + (vB[1] * alphaB * (255 - alphaA) / (255*255));
				vB[2] = (vA[2] * alphaA / 255) + (vB[2] * alphaB * (255 - alphaA) / (255*255));
			}
		}

		if (boundingBoxVisible_)
		{
			cv::rectangle(intoImg,destROI,CV_RGB(255,255,255),boundingBoxThickness_);
		}
	}

	int
	RenderedObject::getNativeWidth() const
	{
		return outImg_.cols;
	}

	int
	RenderedObject::getNativeHeight() const
	{
		return outImg_.rows;
	}

	cv::Size
	RenderedObject::getNativeSize() const
	{
		return outImg_.size();
	}

	cv::Size
	RenderedObject::getScaledSizeFromTargetHeight(
		int targetHeight_px) const
	{
		double scale = (double)(targetHeight_px) / getNativeHeight();
		return cv::Size(getNativeWidth()*scale,getNativeHeight()*scale);
	}

	void
	RenderedObject::setVisible(
		bool visible)
	{
		bool modified = visible_ != visible;
		visible_ = visible;
		if (modified)
		{
			markNeedsRedraw();
			markObjectModified();
		}
	}

	bool
	RenderedObject::isVisible() const
	{
		return visible_;
	}

	void
	RenderedObject::setBoundingBoxVisible(
		bool visible)
	{
		bool modified = boundingBoxVisible_ != visible;
		boundingBoxVisible_ = visible;
		if (modified)
		{
			markNeedsRedraw();
			// no need to mark as modified since we don't save this
			// property of the object when encoding/decoding
		}
	}

	bool
	RenderedObject::isBoundingBoxVisible() const
	{
		return boundingBoxVisible_;
	}

	void
	RenderedObject::setBoundingBoxThickness(
		unsigned int thickness)
	{
		bool modified = boundingBoxThickness_ != thickness;
		boundingBoxThickness_ = thickness;
		if (modified)
		{
			markNeedsRedraw();
			// no need to mark as modified since we don't save this
			// property of the object when encoding/decoding
		}
	}

	unsigned int
	RenderedObject::getBoundingBoxThickness() const
	{
		return boundingBoxThickness_;
	}

	DataSourceRequirements
	RenderedObject::dataSourceRequirements() const
	{
		return DataSourceRequirements(0,0,0);
	}

	bool
	RenderedObject::requirementsMet() const
	{
		bool met = true;
		met = met && videoReqsMet();
		met = met && telemetryReqsMet();
		met = met && trackReqsMet();
		return met;
	}

	bool
	RenderedObject::addVideoSource(
		VideoSourcePtr vSrc)
	{
		const auto reqs = dataSourceRequirements();
		if (reqs.numVideoSources != DSR_ZERO_OR_MORE &&
			reqs.numVideoSources != DSR_ONE_OR_MORE &&
			vSources_.size() == (size_t)reqs.numVideoSources)
		{
			return false;
		}

		vSources_.push_back(vSrc);
		checkAndNotifyRequirementsMet();
		markNeedsRedraw();
		markObjectModified();
		return true;
	}

	size_t
	RenderedObject::numVideoSources() const
	{
		return vSources_.size();
	}

	void
	RenderedObject::setVideoSource(
		size_t idx,
		VideoSourcePtr vSrc)
	{
		vSources_.at(idx) = vSrc;
		markNeedsRedraw();
		markObjectModified();
	}

	VideoSourcePtr
	RenderedObject::getVideoSource(
		size_t idx)
	{
		return vSources_.at(idx);
	}

	void
	RenderedObject::removeVideoSource(
		size_t idx)
	{
		vSources_.erase(std::next(vSources_.begin(), idx));
		markNeedsRedraw();
		markObjectModified();
	}

	bool
	RenderedObject::addTelemetrySource(
		TelemetrySourcePtr tSrc)
	{
		const auto reqs = dataSourceRequirements();
		if (reqs.numTelemetrySources != DSR_ZERO_OR_MORE &&
			reqs.numTelemetrySources != DSR_ONE_OR_MORE &&
			tSources_.size() == (size_t)reqs.numTelemetrySources)
		{
			return false;
		}

		tSources_.push_back(tSrc);
		checkAndNotifyRequirementsMet();
		markNeedsRedraw();
		markObjectModified();
		return true;
	}

	size_t
	RenderedObject::numTelemetrySources() const
	{
		return tSources_.size();
	}

	void
	RenderedObject::setTelemetrySource(
		size_t idx,
		TelemetrySourcePtr tSrc)
	{
		tSources_.at(idx) = tSrc;
		markNeedsRedraw();
		markObjectModified();
	}

	TelemetrySourcePtr
	RenderedObject::getTelemetrySource(
		size_t idx)
	{
		return tSources_.at(idx);
	}

	void
	RenderedObject::removeTelemetrySource(
		size_t idx)
	{
		tSources_.erase(std::next(tSources_.begin(), idx));
		markNeedsRedraw();
		markObjectModified();
	}

	bool
	RenderedObject::setTrack(
		const Track *track)
	{
		if (trackReqsMet())
		{
			return false;
		}

		track_ = track;
		checkAndNotifyRequirementsMet();
		markNeedsRedraw();
		return true;
	}

	const Track *
	RenderedObject::getTrack() const
	{
		return track_;
	}

	YAML::Node
	RenderedObject::encode() const
	{
		YAML::Node node;
		node["typeName"] = typeName();
		node["visible"] = visible_;

		YAML::Node yVidSources = node["vSources"];
		for (const auto &vSource : vSources_)
		{
			yVidSources.push_back(vSource->getDataSourceName());
		}

		YAML::Node yTelemSources = node["tSources"];
		for (const auto &tSource : tSources_)
		{
			yTelemSources.push_back(tSource->getDataSourceName());
		}

		node["subclass"] = subEncode();

		return node;
	}

	bool
	RenderedObject::decode(
		const YAML::Node& node,
		const DataSourceManager &dsm)
	{
		bool okay = true;

		YAML_TO_FIELD(node, "visible", visible_);

		// load in all the video sources
		if (node["vSources"])
		{
			const YAML::Node yVidSources = node["vSources"];
			for (size_t i=0; i<yVidSources.size(); i++)
			{
				const auto &sourceName = yVidSources[i].as<std::string>();
				auto dSrc = dsm.getSourceByName(sourceName);
				if (dSrc != nullptr)
				{
					addVideoSource(dSrc->videoSrc);
				}
				else
				{
					spdlog::error("failed to lookup video DataSource for sourceName '{}'", sourceName.c_str());
					okay = false;
				}
			}
		}

		// load in all the telemetry sources
		if (node["tSources"])
		{
			const YAML::Node yTelemSources = node["tSources"];
			for (size_t i=0; i<yTelemSources.size(); i++)
			{
				const auto &sourceName = yTelemSources[i].as<std::string>();
				auto dSrc = dsm.getSourceByName(sourceName);
				if (dSrc != nullptr)
				{
					addTelemetrySource(dSrc->telemSrc);
				}
				else
				{
					spdlog::error("failed to lookup telemetry DataSource for sourceName '{}'", sourceName.c_str());
					okay = false;
				}
			}
		}

		// always call subclass's decode last because some of them are depending
		// on having their telemetry source lists populated already.
		okay = subDecode(node["subclass"]) && okay;

		return okay;
	}

	void
	RenderedObject::sourcesValid()
	{
		// do nothing impl - let subclass override if needed
	}

	bool
	RenderedObject::videoReqsMet() const
	{
		const auto reqs = dataSourceRequirements();
		bool met = true;

		if (reqs.numVideoSources == DSR_ZERO_OR_MORE)
		{
			// anything counts
		}
		else if (reqs.numVideoSources == DSR_ONE_OR_MORE)
		{
			met = met && vSources_.size() >= 1;
		}
		else
		{
			met = met && vSources_.size() == (size_t)reqs.numVideoSources;
		}

		return met;
	}

	bool
	RenderedObject::telemetryReqsMet() const
	{
		const auto reqs = dataSourceRequirements();
		bool met = true;

		if (reqs.numTelemetrySources == DSR_ZERO_OR_MORE)
		{
			// anything counts
		}
		else if (reqs.numTelemetrySources == DSR_ONE_OR_MORE)
		{
			met = met && tSources_.size() >= 1;
		}
		else
		{
			met = met && tSources_.size() == (size_t)reqs.numTelemetrySources;
		}

		return met;
	}

	bool
	RenderedObject::trackReqsMet() const
	{
		const auto reqs = dataSourceRequirements();
		bool met = true;

		if (reqs.numTracks == DSR_ZERO_OR_MORE)
		{
			// anything counts
		}
		else if (reqs.numTracks == DSR_ONE_OR_MORE)
		{
			met = met && track_ != nullptr;
		}
		else if (reqs.numTracks > 0)
		{
			met = met && track_ != nullptr;
		}

		return met;
	}

	void
	RenderedObject::checkAndNotifyRequirementsMet()
	{
		if (requirementsMet())
		{
			sourcesValid();
		}
	}

}
