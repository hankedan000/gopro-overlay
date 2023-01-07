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
		eXC_Samples,
		eXC_Time
	};

	enum Y_Component
	{
		eYC_Unknown,
		eYC_Time,
		eYC_AcclX,eYC_AcclY,eYC_AcclZ,
		eYC_GyroX,eYC_GyroY,eYC_GyroZ
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
	setX_Component(
			X_Component comp,
			bool replot = true);

	void
	setY_Component(
			Y_Component comp,
			bool replot = true);

private:
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

};
