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
		// TelemetryPlot is a Qt widget, and all QWidgets require an instance to a QApplication.
		// Since not all applications using this class are Qt GUIs, we will detect if a QApplication
		// is already running within the process, and if not we'll construct our own.
		// It's a pretty bad hack, but I can't see a way around it.
		// There's also a requirement that QApplications need a valid set of argc & argv to exist
		// for the lifetime of the app. See the below post about weird seg faults that can occur if
		// this isn't the case.
		// https://stackoverflow.com/questions/35566459/segfault-when-accessing-qapplicationarguments
		struct FakeQtApp
		{
			int argc;
			char *argv[1];
    		QApplication *app;
		} fakeApp_;

        TelemetryPlot *plot_;

		// the number of visible samples to display horizontally (in seconds)
		double plotWidthTime_sec_;

		// calculated based on the time difference between telemetry samples.
		// this is used to determine how many samples to display along the x-axis
		// when scrolling.
		double calculatedFPS_;

	};
}