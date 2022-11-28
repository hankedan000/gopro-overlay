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
		std::string
		typeName() const override;

		virtual
		DataSourceRequirements
		dataSourceRequirements() const override;

		virtual
		void
		render(
			cv::Mat &intoImg,
			int originX, int originY,
			cv::Size renderSize);

	protected:
		virtual
		YAML::Node
		subEncode() const override;

		virtual
		bool
		subDecode(
			const YAML::Node& node) override;

	private:
		VideoSourcePtr source_;

		cv::Mat resizedFrame_;
		size_t prevRenderedFrameIdx_;

	};
}