#pragma once

#include "GoProOverlay/graphics/RenderedObject.h"
#include "GoProOverlay/data/VideoSource.h"

namespace gpo
{
	class VideoObject : public RenderedObject
	{
	public:
		VideoObject();

		virtual
		DataSourceRequirements
		dataSourceRequirements() const override;

		void
		drawInto(
			cv::UMat &intoImg,
			int originX, int originY,
			cv::Size renderSize) override;

	protected:
		virtual
		void
		subRender() override;

		// callback from RenderedObject class when all source requirements are met
		virtual
		void
		sourcesValid() override;

		virtual
		YAML::Node
		subEncode() const override;

		virtual
		bool
		subDecode(
			const YAML::Node& node) override;

	private:
		cv::UMat resizedFrame_;
		size_t prevRenderedFrameIdx_;

	};
}