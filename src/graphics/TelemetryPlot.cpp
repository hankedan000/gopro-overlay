#include "GoProOverlay/graphics/TelemetryPlot.h"

TelemetryPlot::TelemetryPlot(
		QWidget *parent)
: QCustomPlot(parent)
, xComponent_(X_Component::eXC_Samples)
, yComponent_(Y_Component::eYC_Unknown)
, plotTitle_(new QCPTextElement(this,"",15))
{
	setInteraction(QCP::Interaction::iRangeDrag,true);
	setInteraction(QCP::Interaction::iRangeZoom,true);
	legend->setVisible(true);

	// add the title element to the plot
	plotLayout()->insertRow(0);
	plotLayout()->addElement(0,0, plotTitle_);

	// check and correct for data realignments before each replot()
	connect(this, &QCustomPlot::beforeReplot, this, [this]{
		for (auto &sourceObjs : sources_)
		{
			auto alignmentIdx = sourceObjs.telemSrc->seeker()->getAlignmentIdx();
			if (alignmentIdx != sourceObjs.alignmentIdxAtLastReplot)
			{
				setX_Data(sourceObjs, xComponent_);
				sourceObjs.alignmentIdxAtLastReplot = alignmentIdx;
			}
		}
	});
}

TelemetryPlot::~TelemetryPlot()
{}

void
TelemetryPlot::applyDarkTheme()
{
	const auto BORDER_COLOR = Qt::gray;
	const auto TICK_COLOR = Qt::gray;
	const auto TEXT_COLOR = Qt::gray;
	
	// style items within the chart
    setBackground(QBrush(QRgb(0x1b1e20)));
    xAxis->setBasePen(QPen(BORDER_COLOR, 1));
    yAxis->setBasePen(QPen(BORDER_COLOR, 1));
    xAxis->setTickPen(QPen(TICK_COLOR, 1));
    yAxis->setTickPen(QPen(TICK_COLOR, 1));
    xAxis->setSubTickPen(QPen(TICK_COLOR, 1));
    yAxis->setSubTickPen(QPen(TICK_COLOR, 1));

	// style text
	xAxis->setLabelColor(TEXT_COLOR);
	yAxis->setLabelColor(TEXT_COLOR);
    xAxis->setTickLabelColor(TEXT_COLOR);
    yAxis->setTickLabelColor(TEXT_COLOR);
	plotTitle_->setTextColor(TEXT_COLOR);

	// style the legend
	legend->setBrush(QBrush(QRgb(0x2A2E32)));
	legend->setTextColor(TEXT_COLOR);
	legend->setBorderPen(QPen(BORDER_COLOR, 1));
}

void
TelemetryPlot::addSource(
		gpo::TelemetrySourcePtr telemSrc,
		bool replot)
{
	QColor color = DEFAULT_COLORS[sources_.size() % N_DEFAULT_COLORS];
	addSource_(telemSrc,telemSrc->getDataSourceName(),color,replot);
}
			
void
TelemetryPlot::addSource(
		gpo::TelemetrySourcePtr telemSrc,
		const std::string &label,
		QColor color,
		bool replot)
{
	addSource_(telemSrc,label,color,replot);
}

void
TelemetryPlot::removeSource(
		size_t idx,
		bool replot)
{
	auto sourceItr = std::next(sources_.begin(),idx);
	if (sourceItr != sources_.end())
	{
		// remove the source's graph
		removeGraph(sourceItr->graph);

		sources_.erase(sourceItr);
	}

	if (replot)
	{
		this->replot();
	}
}

void
TelemetryPlot::clear(
		bool replot)
{
	while ( ! sources_.empty())
	{
		removeSource(0,false);// hold off replotting until the end
	}

	if (replot)
	{
		this->replot();
	}
}

gpo::TelemetrySourcePtr
TelemetryPlot::getSource(
		size_t idx)
{
	return sources_.at(idx).telemSrc;
}

size_t
TelemetryPlot::numSources() const
{
	return sources_.size();
}

void
TelemetryPlot::realignData(
		bool replot)
{
	for (auto &sourceObjs : sources_)
	{
		setX_Data(sourceObjs, xComponent_);
	}

	if (replot)
	{
		this->replot();
	}
}

void
TelemetryPlot::setTelemetryColor(
	gpo::TelemetrySourcePtr telemSrc,
	QColor color,
	bool replot)
{
	for (auto &sourceObjs : sources_)
	{
		if (sourceObjs.telemSrc.get() == telemSrc.get())
		{
			sourceObjs.graph->pen().setColor(color);
		}
	}

	if (replot)
	{
		this->replot();
	}
}

std::pair<bool,QColor>
TelemetryPlot::getTelemetryColor(
	gpo::TelemetrySourcePtr telemSrc) const
{
	for (const auto &sourceObjs : sources_)
	{
		if (sourceObjs.telemSrc.get() == telemSrc.get())
		{
			return {true,sourceObjs.graph->pen().color()};
		}
	}
	return {false,Qt::black};
}

void
TelemetryPlot::setTelemetryLabel(
	gpo::TelemetrySourcePtr telemSrc,
	const std::string &label,
	bool replot)
{
	for (auto &sourceObjs : sources_)
	{
		if (sourceObjs.telemSrc.get() == telemSrc.get())
		{
			sourceObjs.graph->setName(label.c_str());
		}
	}

	if (replot)
	{
		this->replot();
	}
}

std::pair<bool,std::string>
TelemetryPlot::getTelemetryLabel(
	gpo::TelemetrySourcePtr telemSrc) const
{
	for (const auto &sourceObjs : sources_)
	{
		if (sourceObjs.telemSrc.get() == telemSrc.get())
		{
			return {true,sourceObjs.graph->name().toStdString()};
		}
	}
	return {false,""};
}

void
TelemetryPlot::setX_Component(
		X_Component comp,
		bool replot)
{
	if (xComponent_ == comp)
	{
		// nothing to do
		return;
	}

	// perform updates
	xComponent_ = comp;
	for (auto &sourceObjs : sources_)
	{
		setX_Data(sourceObjs,xComponent_);
	}
	rescaleAxes();

	if (replot)
	{
		this->replot();
	}
}

TelemetryPlot::X_Component
TelemetryPlot::getX_Component() const
{
	return xComponent_;
}

void
TelemetryPlot::setY_Component(
		Y_Component comp,
		bool replot)
{
	if (yComponent_ == comp)
	{
		// nothing to do
		return;
	}

	// perform updates
	yComponent_ = comp;
	for (auto &sourceObjs : sources_)
	{
		setY_Data(sourceObjs,yComponent_);
	}
	rescaleAxes();

	if (replot)
	{
		this->replot();
	}
}

TelemetryPlot::Y_Component
TelemetryPlot::getY_Component() const
{
	return yComponent_;
}

void
TelemetryPlot::setPlotTitle(
	const std::string &title)
{
	plotTitle_->setText(title.c_str());
}

void
TelemetryPlot::addSource_(
		gpo::TelemetrySourcePtr telemSrc,
		const std::string &label,
		QColor color,
		bool replot)
{
	// prevent telemSrc from being added again
	for (const auto &sourceObjs : sources_)
	{
		if (sourceObjs.telemSrc.get() == telemSrc.get())
		{
			return;
		}
	}

	SourceObjects sourceObjs;
	sourceObjs.telemSrc = telemSrc;
	QVector<double> xData(telemSrc->size()), yData(telemSrc->size());
	addGraph();
	sourceObjs.graph = graph(graphCount() - 1);
	sourceObjs.graph->setData(xData,yData,true);
	sourceObjs.graph->setName(label.c_str());
	sourceObjs.graph->setPen(QPen(color));
	sourceObjs.alignmentIdxAtLastReplot = telemSrc->seeker()->getAlignmentIdx();
	setX_Data(sourceObjs,xComponent_);
	setY_Data(sourceObjs,yComponent_);
	rescaleAxes();

	sources_.push_back(sourceObjs);

	if (replot)
	{
		this->replot();
	}
}

void
TelemetryPlot::setX_Data(
		SourceObjects &sourceObjs,
		X_Component comp)
{
	auto telemSrc = sourceObjs.telemSrc;
	auto seeker = telemSrc->seeker();
	size_t alignmentIdx = seeker->getAlignmentIdx();
	auto &alignmentSamp = telemSrc->at(alignmentIdx);
	auto dataPtr = sourceObjs.graph->data();
	auto dataItr = dataPtr->begin();
	for (size_t i=0; i<telemSrc->size() && dataItr!=dataPtr->end(); i++, dataItr++)
	{
		switch (comp)
		{
		case X_Component::eXC_Samples:
			dataItr->key = (double)(i) - alignmentIdx;
			xAxis->setLabel("samples");
			break;
		case X_Component::eXC_Time:
			dataItr->key = telemSrc->at(i).gpSamp.t_offset - alignmentSamp.gpSamp.t_offset;
			xAxis->setLabel("time (s)");
			break;
		default:
			printf("%s - unsupported X_Component (%d)\n",__func__,(int)(comp));
			dataItr->key = 0;
			break;
		}
	}
}

void
TelemetryPlot::setY_Data(
		SourceObjects &sourceObjs,
		Y_Component comp)
{
	auto dataPtr = sourceObjs.graph->data();
	auto dataItr = dataPtr->begin();
	for (size_t i=0; i<sourceObjs.telemSrc->size() && dataItr!=dataPtr->end(); i++, dataItr++)
	{
		auto &tSamp = sourceObjs.telemSrc->at(i);
		switch (comp)
		{
		case Y_Component::eYC_Unknown:
			dataItr->value = 0;
			setPlotTitle("");
			break;
		case Y_Component::eYC_Time:
			dataItr->value = tSamp.gpSamp.t_offset;
			setPlotTitle("Time");
			yAxis->setLabel("time offset (s)");
			break;
		case Y_Component::eYC_AcclX:
			dataItr->value = tSamp.gpSamp.accl.x;
			setPlotTitle("X Acceleration");
			yAxis->setLabel("acceleration (m/s^2)");
			break;
		case Y_Component::eYC_AcclY:
			dataItr->value = tSamp.gpSamp.accl.y;
			setPlotTitle("Y Acceleration");
			yAxis->setLabel("acceleration (m/s^2)");
			break;
		case Y_Component::eYC_AcclZ:
			dataItr->value = tSamp.gpSamp.accl.z;
			setPlotTitle("Z Acceleration");
			yAxis->setLabel("acceleration (m/s^2)");
			break;
		case Y_Component::eYC_GyroX:
			dataItr->value = tSamp.gpSamp.gyro.x;
			setPlotTitle("X Gyroscope");
			yAxis->setLabel("angular velocity (rad/s)");
			break;
		case Y_Component::eYC_GyroY:
			dataItr->value = tSamp.gpSamp.gyro.y;
			setPlotTitle("Y Gyroscope");
			yAxis->setLabel("angular velocity (rad/s)");
			break;
		case Y_Component::eYC_GyroZ:
			dataItr->value = tSamp.gpSamp.gyro.z;
			setPlotTitle("Z Gyroscope");
			yAxis->setLabel("angular velocity (rad/s)");
			break;
		case Y_Component::eYC_GPS_Speed2D:
			dataItr->value = tSamp.gpSamp.gps.speed2D;
			setPlotTitle("GPS 2D Speed");
			yAxis->setLabel("velocity (m/s)");
			break;
		case Y_Component::eYC_GPS_Speed3D:
			dataItr->value = tSamp.gpSamp.gps.speed3D;
			setPlotTitle("GPS 3D Speed");
			yAxis->setLabel("velocity (m/s)");
			break;
		default:
			printf("%s - unsupported Y_Component (%d)\n",__func__,(int)(comp));
			dataItr->value = 0;
			setPlotTitle("**INVALID Y COMPONENT**");
			break;
		}
	}
}
