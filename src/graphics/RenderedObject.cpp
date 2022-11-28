#include "GoProOverlay/graphics/RenderedObject.h"

namespace gpo
{
	RenderedObject::RenderedObject(
		int width,
		int height)
	 : outImg_(height,width,CV_8UC4,cv::Scalar(0,0,0,0))
	 , visible_(true)
	 , boundingBoxVisible_(false)
	 , vSources_()
	 , tSources_()
	 , track_(nullptr)
	{
	}

	const cv::Mat &
	RenderedObject::getImage() const
	{
		return outImg_;
	}

	void
	RenderedObject::render(
		cv::Mat &intoImg,
		int originX, int originY)
	{
		render(intoImg,originX,originY,outImg_.size());
	}

	void
	RenderedObject::render(
		cv::Mat &intoImg,
		int originX, int originY,
		cv::Size renderSize)
	{
		cv::Mat *imgToRender = &outImg_;
		if (renderSize.width != getNativeWidth() || renderSize.height != getNativeHeight())
		{
			ALPHA_SAFE_RESIZE(outImg_,scaledImg_,renderSize);
			imgToRender = &scaledImg_;
		}

		// draw final output to user image
		cv::Rect roi(cv::Point(originX,originY), imgToRender->size());
		cv::Mat3b destROI = intoImg(roi);
		double alpha = 1.0; // alpha in [0,1]
		for (int r = 0; r < destROI.rows; ++r)
		{
			for (int c = 0; c < destROI.cols; ++c)
			{
				auto vf = imgToRender->at<cv::Vec4b>(r,c);
				// Blending
				if (vf[3] > 0)
				{
					cv::Vec3b &vb = destROI(r,c);
					vb[0] = alpha * vf[0] + (1 - alpha) * vb[0];
					vb[1] = alpha * vf[1] + (1 - alpha) * vb[1];
					vb[2] = alpha * vf[2] + (1 - alpha) * vb[2];
				}
			}
		}

		if (boundingBoxVisible_)
		{
			cv::rectangle(intoImg,roi,CV_RGB(255,255,255));
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
		visible_ = visible;
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
		boundingBoxVisible_ = visible;
	}

	bool
	RenderedObject::isBoundingBoxVisible() const
	{
		return boundingBoxVisible_;
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
			vSources_.size() == reqs.numVideoSources)
		{
			return false;
		}

		vSources_.push_back(vSrc);
		checkAndNotifyRequirementsMet();
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
	}

	bool
	RenderedObject::addTelemetrySource(
		TelemetrySourcePtr tSrc)
	{
		const auto reqs = dataSourceRequirements();
		if (reqs.numTelemetrySources != DSR_ZERO_OR_MORE &&
			reqs.numTelemetrySources != DSR_ONE_OR_MORE &&
			tSources_.size() == reqs.numTelemetrySources)
		{
			return false;
		}

		tSources_.push_back(tSrc);
		checkAndNotifyRequirementsMet();
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
		return true;
	}

	const Track *
	RenderedObject::getTrack() const
	{
		return track_;
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
			met = met && vSources_.size() == reqs.numVideoSources;
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
			met = met && tSources_.size() == reqs.numTelemetrySources;
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