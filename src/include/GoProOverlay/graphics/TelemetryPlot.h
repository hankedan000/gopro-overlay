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
		eYC_Unknown = 0,

		eYC_Time = 1,

		eYC_AcclX = 2,
		eYC_AcclY = 3,
		eYC_AcclZ = 4,

		eYC_GyroX = 5,
		eYC_GyroY = 6,
		eYC_GyroZ = 7,

		eYC_GPS_Speed2D = 8,
		eYC_GPS_Speed3D = 9,

		eYC_ECU_EngineSpeed = 20,
		eYC_ECU_TPS = 21,
		eYC_ECU_Boost = 22
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
	
	//                                                           yComp                  name                    plotTitle              axisTitle                unit
	static constexpr Y_ComponentEnumInfo YCEI_UNKNOWN          = {eYC_Unknown,          "eYC_Unknown",          "",                    "unknown",               ""       };
	static constexpr Y_ComponentEnumInfo YCEI_TIME             = {eYC_Time,             "eYC_Time",             "Time",                "time",                  "s"      };
	static constexpr Y_ComponentEnumInfo YCEI_GP_ACCL_X        = {eYC_AcclX,            "eYC_AcclX",            "X Acceleration",      "acceleration",          "m/s^s"  };
	static constexpr Y_ComponentEnumInfo YCEI_GP_ACCL_Y        = {eYC_AcclY,            "eYC_AcclY",            "Y Acceleration",      "acceleration",          "m/s^s"  };
	static constexpr Y_ComponentEnumInfo YCEI_GP_ACCL_Z        = {eYC_AcclZ,            "eYC_AcclZ",            "Z Acceleration",      "acceleration",          "m/s^s"  };
	static constexpr Y_ComponentEnumInfo YCEI_GP_GYRO_X        = {eYC_GyroX,            "eYC_GyroX",            "X Gyroscope",         "angular velocity",      "rad/s"  };
	static constexpr Y_ComponentEnumInfo YCEI_GP_GYRO_Y        = {eYC_GyroY,            "eYC_GyroY",            "Y Gyroscope",         "angular velocity",      "rad/s"  };
	static constexpr Y_ComponentEnumInfo YCEI_GP_GYRO_Z        = {eYC_GyroZ,            "eYC_GyroZ",            "Z Gyroscope",         "angular velocity",      "rad/s"  };
	static constexpr Y_ComponentEnumInfo YCEI_GP_GPS_SPEED2D   = {eYC_GPS_Speed2D,      "eYC_GPS_Speed2D",      "GPS Speed (2D)",      "gps speed (2D)",        "m/s"    };
	static constexpr Y_ComponentEnumInfo YCEI_GP_GPS_SPEED3D   = {eYC_GPS_Speed3D,      "eYC_GPS_Speed3D",      "GPS Speed (3D)",      "gps speed (3D)",        "m/s"    };
	static constexpr Y_ComponentEnumInfo YCEI_ECU_ENGINE_SPEED = {eYC_ECU_EngineSpeed,  "eYC_ECU_EngineSpeed",  "Engine Speed",        "engine speed",          "rpm"    };
	static constexpr Y_ComponentEnumInfo YCEI_ECU_TPS          = {eYC_ECU_TPS,          "eYC_ECU_TPS",          "Throttle Position",   "throttle position",     "%"      };
	static constexpr Y_ComponentEnumInfo YCEI_ECU_BOOST        = {eYC_ECU_Boost,        "eYC_ECU_Boost",        "Boost",               "boost",                 "psi"    };

	static constexpr const Y_ComponentEnumInfo *Y_COMP_ENUM_INFOS[] = {
		&YCEI_UNKNOWN,
		&YCEI_TIME,
		&YCEI_GP_ACCL_X,
		&YCEI_GP_ACCL_Y,
		&YCEI_GP_ACCL_Z,
		&YCEI_GP_GYRO_X,
		&YCEI_GP_GYRO_Y,
		&YCEI_GP_GYRO_Z,
		&YCEI_GP_GPS_SPEED2D,
		&YCEI_GP_GPS_SPEED3D,
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
