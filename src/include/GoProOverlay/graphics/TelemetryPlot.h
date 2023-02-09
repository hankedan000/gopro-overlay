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
	static constexpr Y_ComponentEnumInfo Y_COMP_ENUM_INFO[] = {
	//  yComp                  name                    plotTitle              axisTitle                unit
		{eYC_Unknown,          "eYC_Unknown",          "",                    "unknown",               ""},
		{eYC_Time,             "eYC_Time",             "Time",                "time",                  "s"},
		{eYC_AcclX,            "eYC_AcclX",            "X Acceleration",      "acceleration",          "m/s^s"},
		{eYC_AcclY,            "eYC_AcclY",            "Y Acceleration",      "acceleration",          "m/s^s"},
		{eYC_AcclZ,            "eYC_AcclZ",            "Z Acceleration",      "acceleration",          "m/s^s"},
		{eYC_GyroX,            "eYC_GyroX",            "X Gyroscope",         "angular velocity",      "rad/s"},
		{eYC_GyroY,            "eYC_GyroY",            "Y Gyroscope",         "angular velocity",      "rad/s"},
		{eYC_GyroZ,            "eYC_GyroZ",            "Z Gyroscope",         "angular velocity",      "rad/s"},
		{eYC_GPS_Speed2D,      "eYC_GPS_Speed2D",      "GPS Speed (2D)",      "gps speed (2D)",        "m/s"},
		{eYC_GPS_Speed3D,      "eYC_GPS_Speed3D",      "GPS Speed (3D)",      "gps speed (3D)",        "m/s"},
		{eYC_ECU_EngineSpeed,  "eYC_ECU_EngineSpeed",  "Engine Speed",        "engine speed",          "rpm"},
		{eYC_ECU_TPS,          "eYC_ECU_TPS",          "Throttle Position",   "throttle position",     "%"},
		{eYC_ECU_Boost,        "eYC_ECU_Boost",        "Boost",               "boost",                 "psi"}
	};

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

	static
	const Y_ComponentEnumInfo &
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
