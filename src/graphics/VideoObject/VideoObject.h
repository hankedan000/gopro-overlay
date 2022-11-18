#pragma once

#include "RenderedObject.h"
#include "VideoSource.h"

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

	};
}