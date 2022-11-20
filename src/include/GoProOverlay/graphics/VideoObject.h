#pragma once

#include "GoProOverlay/graphics/RenderedObject.h"
#include "GoProOverlay/data/VideoSource.h"

namespace gpo
{
	class VideoObject : public RenderedObject
	{
	public:
		VideoObject(
			const VideoSourcePtr &vSrc);

		virtual
		void
		render(
			cv::Mat &intoImg,
			int originX, int originY,
			cv::Size renderSize);

	private:
		VideoSourcePtr source_;

		cv::Mat resizedFrame_;
		size_t prevRenderedFrameIdx_;

	};
}