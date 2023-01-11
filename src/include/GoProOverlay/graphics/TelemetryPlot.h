#pragma once

#include "qcustomplot.h"
#include <vector>

#include "GoProOverlay/data/DataSource.h"

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
		eYC_GPS_Speed3D = 9
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
	};

public:
	explicit TelemetryPlot(
		QWidget *parent = nullptr);
	~TelemetryPlot();

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

private:
	void
	addSource_(
			gpo::TelemetrySourcePtr telemSrc,
			const std::string &label,
			QColor color,
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
	Y_Component yComponent_;
	std::vector<SourceObjects> sources_;

	QCPTextElement *plotTitle_;

};
