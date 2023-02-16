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
		std::string
		typeName() const override;

		virtual
		DataSourceRequirements
		dataSourceRequirements() const override;

		virtual
		void
		render() override;

		void
		drawInto(
			cv::UMat &intoImg,
			int originX, int originY,
			cv::Size renderSize) override;

	protected:
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