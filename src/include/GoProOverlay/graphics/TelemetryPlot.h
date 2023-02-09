#pragma once

#include <vector>

#include "GoProOverlay/data/DataSource.h"
#include "qcustomplot.h"

class TelemetryPlot : public QCustomPlot
{
	Q_OBJECT

public:
	enum X_Component
	{
		eXC_Samples = 0,
		eXC_Time = 1
	};

	enum Y_Component
	{
		eYC_UNKNOWN = 0,

		eYC_TIME = 1,

		eYC_ACCL_X = 2,
		eYC_ACCL_Y = 3,
		eYC_ACCL_Z = 4,
		eYC_GYRO_X = 5,
		eYC_GYRO_Y = 6,
		eYC_GYRO_Z = 7,
		eYC_GRAV_X = 23,
		eYC_GRAV_Y = 24,
		eYC_GRAV_Z = 25,
		eYC_CORI_W = 26,
		eYC_CORI_X = 27,
		eYC_CORI_Y = 28,
		eYC_CORI_Z = 29,
		eYC_GPS_LAT = 30,
		eYC_GPS_LON = 31,
		eYC_GPS_SPEED2D = 8,
		eYC_GPS_SPEED3D = 9,

		eYC_ECU_ENGINE_SPEED = 20,
		eYC_ECU_TPS = 21,
		eYC_ECU_BOOST = 22
	};

	enum AxisSide
	{
		eAS_Side1,// xAxis & yAxis
		eAS_Side2 // xAxis2 & yAxis2
	};

	struct Y_ComponentEnumInfo
	{
		Y_Component yComp;
		const char *name;
		const char *plotTitle;
		const char *axisTitle;
		const char *unit;
	};
	
	//                                                           yComp                  name                    plotTitle               axisTitle                unit
	static constexpr Y_ComponentEnumInfo YCEI_UNKNOWN          = {eYC_UNKNOWN,          "eYC_UNKNOWN",          "",                     "unknown",               ""       };
	static constexpr Y_ComponentEnumInfo YCEI_TIME             = {eYC_TIME,             "eYC_TIME",             "Time",                 "time",                  "s"      };
	static constexpr Y_ComponentEnumInfo YCEI_ACCL_X           = {eYC_ACCL_X,           "eYC_ACCL_X",           "X Acceleration",       "acceleration",          "m/s^s"  };
	static constexpr Y_ComponentEnumInfo YCEI_ACCL_Y           = {eYC_ACCL_Y,           "eYC_ACCL_Y",           "Y Acceleration",       "acceleration",          "m/s^s"  };
	static constexpr Y_ComponentEnumInfo YCEI_ACCL_Z           = {eYC_ACCL_Z,           "eYC_ACCL_Z",           "Z Acceleration",       "acceleration",          "m/s^s"  };
	static constexpr Y_ComponentEnumInfo YCEI_GYRO_X           = {eYC_GYRO_X,           "eYC_GYRO_X",           "X Gyroscope",          "angular velocity",      "rad/s"  };
	static constexpr Y_ComponentEnumInfo YCEI_GYRO_Y           = {eYC_GYRO_Y,           "eYC_GYRO_Y",           "Y Gyroscope",          "angular velocity",      "rad/s"  };
	static constexpr Y_ComponentEnumInfo YCEI_GYRO_Z           = {eYC_GYRO_Z,           "eYC_GYRO_Z",           "Z Gyroscope",          "angular velocity",      "rad/s"  };
	static constexpr Y_ComponentEnumInfo YCEI_GRAV_X           = {eYC_GRAV_X,           "eYC_GRAV_X",           "X Gravity",            "gravity",               "G"      };
	static constexpr Y_ComponentEnumInfo YCEI_GRAV_Y           = {eYC_GRAV_Y,           "eYC_GRAV_Y",           "Y Gravity",            "gravity",               "G"      };
	static constexpr Y_ComponentEnumInfo YCEI_GRAV_Z           = {eYC_GRAV_Z,           "eYC_GRAV_Z",           "Z Gravity",            "gravity",               "G"      };
	static constexpr Y_ComponentEnumInfo YCEI_CORI_W           = {eYC_CORI_W,           "eYC_CORI_W",           "Camera Orientation W", "camera orientation",    "unit"   };
	static constexpr Y_ComponentEnumInfo YCEI_CORI_X           = {eYC_CORI_X,           "eYC_CORI_X",           "Camera Orientation X", "camera orientation",    "unit"   };
	static constexpr Y_ComponentEnumInfo YCEI_CORI_Y           = {eYC_CORI_Y,           "eYC_CORI_Y",           "Camera Orientation Y", "camera orientation",    "unit"   };
	static constexpr Y_ComponentEnumInfo YCEI_CORI_Z           = {eYC_CORI_Z,           "eYC_CORI_Z",           "Camera Orientation Z", "camera orientation",    "unit"   };
	static constexpr Y_ComponentEnumInfo YCEI_GPS_LAT          = {eYC_GPS_LAT,          "eYC_GPS_LAT",          "GPS Latitude",         "gps latitude",          "deg"    };
	static constexpr Y_ComponentEnumInfo YCEI_GPS_LON          = {eYC_GPS_LON,          "eYC_GPS_LON",          "GPS Longitude",        "gps longitude",         "deg"    };
	static constexpr Y_ComponentEnumInfo YCEI_GPS_SPEED2D      = {eYC_GPS_SPEED2D,      "eYC_GPS_SPEED2D",      "GPS Speed (2D)",       "gps speed (2D)",        "m/s"    };
	static constexpr Y_ComponentEnumInfo YCEI_GPS_SPEED3D      = {eYC_GPS_SPEED3D,      "eYC_GPS_SPEED3D",      "GPS Speed (3D)",       "gps speed (3D)",        "m/s"    };
	static constexpr Y_ComponentEnumInfo YCEI_ECU_ENGINE_SPEED = {eYC_ECU_ENGINE_SPEED, "eYC_ECU_ENGINE_SPEED", "Engine Speed",         "engine speed",          "rpm"    };
	static constexpr Y_ComponentEnumInfo YCEI_ECU_TPS          = {eYC_ECU_TPS,          "eYC_ECU_TPS",          "Throttle Position",    "throttle position",     "%"      };
	static constexpr Y_ComponentEnumInfo YCEI_ECU_BOOST        = {eYC_ECU_BOOST,        "eYC_ECU_BOOST",        "Boost",                "boost",                 "psi"    };

	static constexpr const Y_ComponentEnumInfo *Y_COMP_ENUM_INFOS[] = {
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
	static constexpr size_t NUM_Y_COMP_ENUM_INFOS = sizeof(Y_COMP_ENUM_INFOS) / sizeof(Y_COMP_ENUM_INFOS[0]);

private:
	static constexpr Qt::GlobalColor DEFAULT_COLORS[] = {
		Qt::red,
		Qt::green,
		Qt::blue,
		Qt::magenta,
		Qt::cyan,
		Qt::yellow,
		Qt::black,
		Qt::gray,
	};
	static constexpr size_t N_DEFAULT_COLORS = sizeof(DEFAULT_COLORS) / sizeof(DEFAULT_COLORS[0]);

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

		AxisSide yAxisSide;
	};

public:
	explicit TelemetryPlot(
		QWidget *parent = nullptr);
	~TelemetryPlot();

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
		AxisSide yAxisSide,
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
		X_Component comp,
		bool replot = true);

	X_Component
	getX_Component() const;

	void
	setY_Component(
		Y_Component comp,
		bool replot = true);

	Y_Component
	getY_Component() const;

	void
	setY_Component2(
		Y_Component comp,
		bool replot = true);

	Y_Component
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
	std::vector<const TelemetryPlot::Y_ComponentEnumInfo *>
	getAvailY_ComponentInfo(
		gpo::TelemetrySourcePtr tSrc);

	static
	const Y_ComponentEnumInfo *
	getY_ComponentInfo(
		const Y_Component &comp);

private:
	void
	addSource_(
			gpo::TelemetrySourcePtr telemSrc,
			const std::string &label,
			QColor color,
			AxisSide yAxisSide,
			bool replot = true);
	
	void
	setX_Data(
			SourceObjects &sourceObjs,
			X_Component comp);

	void
	setY_Data(
			SourceObjects &sourceObjs,
			Y_Component comp);

private:
	X_Component xComponent_;
	Y_Component yComponent_; // for yAxis
	Y_Component yComponent2_;// for yAxis2
	std::vector<SourceObjects> sources_;

	QCPTextElement *plotTitle_;

};
