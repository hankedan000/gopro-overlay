#pragma once

#include "GoProOverlay/graphics/RenderedObject.h"

namespace gpo
{
	class TelemetryPrintoutObject : public RenderedObject
	{
	public:
		TelemetryPrintoutObject();

		virtual
		std::string
		typeName() const override;

		virtual
		DataSourceRequirements
		dataSourceRequirements() const override;

		void
		setFontFace(
			int face);

		void
		setFontColor(
			cv::Scalar color);

	protected:
		virtual
		void
		subRender() override;

		virtual
		YAML::Node
		subEncode() const override;

		virtual
		bool
		subDecode(
			const YAML::Node& node) override;

	private:
		int fontFace_;
		cv::Scalar fontColor_;

	};
}