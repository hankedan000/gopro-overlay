#pragma once

#include "GoProOverlay/graphics/RenderedObject.h"

namespace gpo
{
	class SpeedometerObject : public RenderedObject
	{
	public:
		SpeedometerObject();

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
			cv::Size renderSize) override;

	protected:
		virtual
		YAML::Node
		subEncode() const override;

		virtual
		bool
		subDecode(
			const YAML::Node& node) override;

	private:

	};
}