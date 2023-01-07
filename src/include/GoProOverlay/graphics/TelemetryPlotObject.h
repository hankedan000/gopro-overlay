#pragma once

#include "GoProOverlay/graphics/RenderedObject.h"
#include "GoProOverlay/graphics/TelemetryPlot.h"

namespace gpo
{
	class TelemetryPlotObject : public RenderedObject
	{
	public:
		TelemetryPlotObject();

		~TelemetryPlotObject();

		virtual
		std::string
		typeName() const override;

		virtual
		DataSourceRequirements
		dataSourceRequirements() const override;

		virtual
		void
		render() override;

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
    	QApplication *app_;
        TelemetryPlot *plot_;

	};
}