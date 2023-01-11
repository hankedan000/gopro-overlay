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

		void
		setTelemetryColor(
			gpo::TelemetrySourcePtr telemSrc,
			QColor color);

		std::pair<bool,QColor>
		getTelemetryColor(
			gpo::TelemetrySourcePtr telemSrc) const;

		void
		setTelemetryLabel(
			gpo::TelemetrySourcePtr telemSrc,
			const std::string &label);

		std::pair<bool,std::string>
		getTelemetryLabel(
			gpo::TelemetrySourcePtr telemSrc) const;

		void
		setX_Component(
				TelemetryPlot::X_Component comp);

		TelemetryPlot::X_Component
		getX_Component() const;

		void
		setY_Component(
				TelemetryPlot::Y_Component comp);

		TelemetryPlot::Y_Component
		getY_Component() const;

		void
		setPlotWidthSeconds(
			double duration_sec);

		double
		getPlotWidthSeconds() const;

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

		// the number of visible samples to display horizontally (in seconds)
		double plotWidthTime_sec_;

		// calculated based on the time difference between telemetry samples.
		// this is used to determine how many samples to display along the x-axis
		// when scrolling.
		double calculatedFPS_;

	};
}