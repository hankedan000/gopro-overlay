#ifndef TELEMETRYPLOT_H
#define TELEMETRYPLOT_H

#include <array>
#include <qcustomplot.h>
#include <vector>

#include "GoProOverlay/data/TelemetrySource.h"
#include "GoProOverlay/graphics/TelemetryPlotTypes.h"

class QTelemetryPlot : public QCustomPlot
{
	Q_OBJECT

public:
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_UNKNOWN          = {gpo::TelemetryPlot::Y_Component::eYC_UNKNOWN,          "eYC_UNKNOWN",          "",                     "unknown",               ""       };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_TIME             = {gpo::TelemetryPlot::Y_Component::eYC_TIME,             "eYC_TIME",             "Time",                 "time",                  "s"      };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_ACCL_X           = {gpo::TelemetryPlot::Y_Component::eYC_ACCL_X,           "eYC_ACCL_X",           "X Acceleration",       "acceleration",          "m/s^s"  };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_ACCL_Y           = {gpo::TelemetryPlot::Y_Component::eYC_ACCL_Y,           "eYC_ACCL_Y",           "Y Acceleration",       "acceleration",          "m/s^s"  };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_ACCL_Z           = {gpo::TelemetryPlot::Y_Component::eYC_ACCL_Z,           "eYC_ACCL_Z",           "Z Acceleration",       "acceleration",          "m/s^s"  };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_GYRO_X           = {gpo::TelemetryPlot::Y_Component::eYC_GYRO_X,           "eYC_GYRO_X",           "X Gyroscope",          "angular velocity",      "rad/s"  };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_GYRO_Y           = {gpo::TelemetryPlot::Y_Component::eYC_GYRO_Y,           "eYC_GYRO_Y",           "Y Gyroscope",          "angular velocity",      "rad/s"  };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_GYRO_Z           = {gpo::TelemetryPlot::Y_Component::eYC_GYRO_Z,           "eYC_GYRO_Z",           "Z Gyroscope",          "angular velocity",      "rad/s"  };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_GRAV_X           = {gpo::TelemetryPlot::Y_Component::eYC_GRAV_X,           "eYC_GRAV_X",           "X Gravity",            "gravity",               "G"      };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_GRAV_Y           = {gpo::TelemetryPlot::Y_Component::eYC_GRAV_Y,           "eYC_GRAV_Y",           "Y Gravity",            "gravity",               "G"      };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_GRAV_Z           = {gpo::TelemetryPlot::Y_Component::eYC_GRAV_Z,           "eYC_GRAV_Z",           "Z Gravity",            "gravity",               "G"      };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_CORI_W           = {gpo::TelemetryPlot::Y_Component::eYC_CORI_W,           "eYC_CORI_W",           "Camera Orientation W", "camera orientation",    "unit"   };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_CORI_X           = {gpo::TelemetryPlot::Y_Component::eYC_CORI_X,           "eYC_CORI_X",           "Camera Orientation X", "camera orientation",    "unit"   };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_CORI_Y           = {gpo::TelemetryPlot::Y_Component::eYC_CORI_Y,           "eYC_CORI_Y",           "Camera Orientation Y", "camera orientation",    "unit"   };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_CORI_Z           = {gpo::TelemetryPlot::Y_Component::eYC_CORI_Z,           "eYC_CORI_Z",           "Camera Orientation Z", "camera orientation",    "unit"   };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_GPS_LAT          = {gpo::TelemetryPlot::Y_Component::eYC_GPS_LAT,          "eYC_GPS_LAT",          "GPS Latitude",         "gps latitude",          "deg"    };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_GPS_LON          = {gpo::TelemetryPlot::Y_Component::eYC_GPS_LON,          "eYC_GPS_LON",          "GPS Longitude",        "gps longitude",         "deg"    };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_GPS_SPEED2D      = {gpo::TelemetryPlot::Y_Component::eYC_GPS_SPEED2D,      "eYC_GPS_SPEED2D",      "GPS Speed (2D)",       "gps speed (2D)",        "m/s"    };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_GPS_SPEED3D      = {gpo::TelemetryPlot::Y_Component::eYC_GPS_SPEED3D,      "eYC_GPS_SPEED3D",      "GPS Speed (3D)",       "gps speed (3D)",        "m/s"    };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_ECU_ENGINE_SPEED = {gpo::TelemetryPlot::Y_Component::eYC_ECU_ENGINE_SPEED, "eYC_ECU_ENGINE_SPEED", "Engine Speed",         "engine speed",          "rpm"    };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_ECU_TPS          = {gpo::TelemetryPlot::Y_Component::eYC_ECU_TPS,          "eYC_ECU_TPS",          "Throttle Position",    "throttle position",     "%"      };
	static constexpr gpo::TelemetryPlot::Y_ComponentEnumInfo YCEI_ECU_BOOST        = {gpo::TelemetryPlot::Y_Component::eYC_ECU_BOOST,        "eYC_ECU_BOOST",        "Boost",                "boost",                 "psi"    };

	static constexpr std::array<const gpo::TelemetryPlot::Y_ComponentEnumInfo *, 22> Y_COMP_ENUM_INFOS = {
		&YCEI_UNKNOWN,
		&YCEI_TIME,
		&YCEI_ACCL_X,
		&YCEI_ACCL_Y,
		&YCEI_ACCL_Z,
		&YCEI_GYRO_X,
		&YCEI_GYRO_Y,
		&YCEI_GYRO_Z,
		&YCEI_GRAV_X,
		&YCEI_GRAV_Y,
		&YCEI_GRAV_Z,
		&YCEI_CORI_W,
		&YCEI_CORI_X,
		&YCEI_CORI_Y,
		&YCEI_CORI_Z,
		&YCEI_GPS_LAT,
		&YCEI_GPS_LON,
		&YCEI_GPS_SPEED2D,
		&YCEI_GPS_SPEED3D,
		&YCEI_ECU_ENGINE_SPEED,
		&YCEI_ECU_TPS,
		&YCEI_ECU_BOOST
	};

private:
	static constexpr std::array<Qt::GlobalColor, 8> DEFAULT_COLORS = {
		Qt::red,
		Qt::green,
		Qt::blue,
		Qt::magenta,
		Qt::cyan,
		Qt::yellow,
		Qt::black,
		Qt::gray,
	};

	struct SourceObjects
	{
		gpo::TelemetrySourcePtr telemSrc;

		// QCustomPlot graph for this telemetry data source
		QCPGraph *graph;

		// the telemetry's seeker alignment index at the last time replot() was called.
		// this is checked prior to a replot operation. if the value doesn't match the
		// the telemetry's current alignment index, then the data's key values will be
		// computed such that they reflect the new alignment index.
		size_t alignmentIdxAtLastReplot;

		gpo::TelemetryPlot::AxisSide yAxisSide;
	};

public:
	explicit QTelemetryPlot(
		QWidget *parent = nullptr);
	
	~QTelemetryPlot() override;

    void
    applyDarkTheme();

	void
	addSource(
		gpo::TelemetrySourcePtr telemSrc,
		bool replot = true);

	void
	addSource(
		gpo::TelemetrySourcePtr telemSrc,
		const std::string &label,
		QColor color,
		bool replot = true);

	void
	addSource(
		gpo::TelemetrySourcePtr telemSrc,
		const std::string &label,
		QColor color,
		gpo::TelemetryPlot::AxisSide yAxisSide,
		bool replot = true);

	void
	removeSource(
		gpo::TelemetrySourcePtr telemSrc,
		bool replot = true);

	void
	removeSource(
		size_t idx,
		bool replot = true);

	void
	clear(
		bool replot = true);

	gpo::TelemetrySourcePtr
	getSource(
		size_t idx);

	/**
	 * @return
	 * the TelemetrySource that pertains to the graph.
	 * nullptr is the graph doesn't reference a source.
	 */
	gpo::TelemetrySourcePtr
	getSource(
		QCPGraph *graph);

	size_t
	numSources() const;

	void
	realignData(
		bool replot = true);

	void
	setTelemetryColor(
		gpo::TelemetrySourcePtr telemSrc,
		QColor color,
		bool replot = true);

	std::pair<bool,QColor>
	getTelemetryColor(
		gpo::TelemetrySourcePtr telemSrc) const;

	void
	setTelemetryLabel(
		gpo::TelemetrySourcePtr telemSrc,
		const std::string &label,
		bool replot = true);

	std::pair<bool,std::string>
	getTelemetryLabel(
		gpo::TelemetrySourcePtr telemSrc) const;

	void
	setX_Component(
		gpo::TelemetryPlot::X_Component comp,
		bool replot = true);

	gpo::TelemetryPlot::X_Component
	getX_Component() const;

	void
	setY_Component(
		gpo::TelemetryPlot::Y_Component comp,
		bool replot = true);

	gpo::TelemetryPlot::Y_Component
	getY_Component() const;

	void
	setY_Component2(
		gpo::TelemetryPlot::Y_Component comp,
		bool replot = true);

	gpo::TelemetryPlot::Y_Component
	getY_Component2() const;

	void
	setPlotTitle(
		const std::string &title);

	std::string
	getPlotTitle() const;

	/**
	 * @param[in] tSrc
	 * a telemetry source
	 * 
	 * @return
	 * a list of Y_ComponentEnumInfo for each data field that's available
	 * within the telemetry source
	 */
	static
	std::vector<const gpo::TelemetryPlot::Y_ComponentEnumInfo *>
	getAvailY_ComponentInfo(
		gpo::TelemetrySourcePtr tSrc);

	static
	const gpo::TelemetryPlot::Y_ComponentEnumInfo *
	getY_ComponentInfo(
		const gpo::TelemetryPlot::Y_Component &comp);

private:
	void
	addSource_(
			gpo::TelemetrySourcePtr telemSrc,
			const std::string &label,
			QColor color,
			gpo::TelemetryPlot::AxisSide yAxisSide,
			bool replot = true);
	
	void
	setX_Data(
			SourceObjects &sourceObjs,
			gpo::TelemetryPlot::X_Component comp);

	void
	setY_Data(
			SourceObjects &sourceObjs,
			gpo::TelemetryPlot::Y_Component comp);

private:
	gpo::TelemetryPlot::X_Component xComponent_;
	gpo::TelemetryPlot::Y_Component yComponent_; // for yAxis
	gpo::TelemetryPlot::Y_Component yComponent2_;// for yAxis2
	std::vector<SourceObjects> sources_;

	QCPTextElement *plotTitle_;

};

#endif
