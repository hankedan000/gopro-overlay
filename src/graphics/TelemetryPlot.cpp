#include "GoProOverlay/graphics/TelemetryPlot.h"

#include <spdlog/spdlog.h>

TelemetryPlot::TelemetryPlot(
		QWidget *parent)
: QCustomPlot(parent)
, xComponent_(X_Component::eXC_Samples)
, yComponent_(Y_Component::eYC_UNKNOWN)
, plotTitle_(new QCPTextElement(this,"",15))
{
	setInteraction(QCP::Interaction::iRangeDrag,true);
	setInteraction(QCP::Interaction::iRangeZoom,true);
	QList<QCPAxis *> draggableAxes = {xAxis,yAxis,yAxis2};
	axisRect()->setRangeDragAxes(draggableAxes);
	QList<QCPAxis *> zoomableAxes = {xAxis,yAxis,yAxis2};
	axisRect()->setRangeZoomAxes(zoomableAxes);
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
	addSource_(telemSrc,telemSrc->getDataSourceName(),color,AxisSide::eAS_Side1,replot);
}
			
void
TelemetryPlot::addSource(
		gpo::TelemetrySourcePtr telemSrc,
		const std::string &label,
		QColor color,
		bool replot)
{
	addSource_(telemSrc,label,color,AxisSide::eAS_Side1,replot);
}

void
TelemetryPlot::addSource(
	gpo::TelemetrySourcePtr telemSrc,
	const std::string &label,
	QColor color,
	AxisSide yAxisSide,
	bool replot)
{
	addSource_(telemSrc,label,color,yAxisSide,replot);
}

void
TelemetryPlot::removeSource(
	gpo::TelemetrySourcePtr telemSrc,
	bool replot)
{
	for (size_t i=0; i<numSources();)
	{
		if (sources_.at(i).telemSrc == telemSrc)
		{
			removeSource(i,replot);
		}
		else
		{
			i++;
		}
	}
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
			// Just calling QPen::setColor() wasn't enough. Seems like the
			// QCustomPlot library doesn't let you change the pen color without
			// calling QCPGraph::setPen() all over again. To preserve other pen
			// settings, we acquire a copy of the original pen and set only the
			// new color to it.
			auto penCopy = sourceObjs.graph->pen();
			penCopy.setColor(color);
			sourceObjs.graph->setPen(penCopy);
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
		if (sourceObjs.yAxisSide == AxisSide::eAS_Side1)
		{
			setY_Data(sourceObjs,comp);
		}
	}
	auto yCompInfo = getY_ComponentInfo(comp);
	setPlotTitle(yCompInfo->plotTitle);
	char axisTitle[1024];
	sprintf(axisTitle,"%s (%s)",yCompInfo->axisTitle,yCompInfo->unit);
	yAxis->setLabel(axisTitle);
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
TelemetryPlot::setY_Component2(
	Y_Component comp,
	bool replot)
{
	if (yComponent2_ == comp)
	{
		// nothing to do
		return;
	}

	// perform updates
	yComponent2_ = comp;
	for (auto &sourceObjs : sources_)
	{
		if (sourceObjs.yAxisSide == AxisSide::eAS_Side2)
		{
			setY_Data(sourceObjs,comp);
		}
	}
	auto yCompInfo = getY_ComponentInfo(comp);
	setPlotTitle(yCompInfo->plotTitle);
	char axisTitle[1024];
	sprintf(axisTitle,"%s (%s)",yCompInfo->axisTitle,yCompInfo->unit);
	yAxis2->setLabel(axisTitle);
	rescaleAxes();

	if (replot)
	{
		this->replot();
	}
}

TelemetryPlot::Y_Component
TelemetryPlot::getY_Component2() const
{
	return yComponent2_;
}

void
TelemetryPlot::setPlotTitle(
	const std::string &title)
{
	plotTitle_->setText(title.c_str());
}

std::string
TelemetryPlot::getPlotTitle() const
{
	return plotTitle_->text().toStdString();
}

std::vector<const TelemetryPlot::Y_ComponentEnumInfo *>
TelemetryPlot::getAvailY_ComponentInfo(
	gpo::TelemetrySourcePtr tSrc)
{
	std::vector<const TelemetryPlot::Y_ComponentEnumInfo *> infos;
	infos.reserve(NUM_Y_COMP_ENUM_INFOS);

    const auto &gpAvail = tSrc->gpDataAvail();
    const auto &ecuAvail = tSrc->ecuDataAvail();
    if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_ACCL))
    {
		infos.push_back(&YCEI_ACCL_X);
		infos.push_back(&YCEI_ACCL_Y);
		infos.push_back(&YCEI_ACCL_Z);
    }
    if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_GYRO))
    {
		infos.push_back(&YCEI_GYRO_X);
		infos.push_back(&YCEI_GYRO_Y);
		infos.push_back(&YCEI_GYRO_Z);
    }
    if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_GRAV))
    {
		infos.push_back(&YCEI_GRAV_X);
		infos.push_back(&YCEI_GRAV_Y);
		infos.push_back(&YCEI_GRAV_Z);
    }
    if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_CORI))
    {
		infos.push_back(&YCEI_CORI_W);
		infos.push_back(&YCEI_CORI_X);
		infos.push_back(&YCEI_CORI_Y);
		infos.push_back(&YCEI_CORI_Z);
    }
    if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_GPS_LATLON))
    {
		infos.push_back(&YCEI_GPS_LAT);
		infos.push_back(&YCEI_GPS_LON);
    }
    if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_GPS_SPEED2D))
    {
		infos.push_back(&YCEI_GPS_SPEED2D);
    }
    if (bitset_is_set(gpAvail, gpo::GOPRO_AVAIL_GPS_SPEED3D))
    {
		infos.push_back(&YCEI_GPS_SPEED3D);
    }
    if (bitset_is_set(ecuAvail, gpo::ECU_AVAIL_ENGINE_SPEED))
    {
		infos.push_back(&YCEI_ECU_ENGINE_SPEED);
    }
    if (bitset_is_set(ecuAvail, gpo::ECU_AVAIL_TPS))
    {
		infos.push_back(&YCEI_ECU_TPS);
    }
    if (bitset_is_set(ecuAvail, gpo::ECU_AVAIL_BOOST))
    {
		infos.push_back(&YCEI_ECU_BOOST);
    }

	return infos;
}

const TelemetryPlot::Y_ComponentEnumInfo *
TelemetryPlot::getY_ComponentInfo(
	const TelemetryPlot::Y_Component &comp)
{
	for (const auto &info : Y_COMP_ENUM_INFOS)
	{
		if (info->yComp == comp)
		{
			return info;
		}
	}
	throw std::runtime_error("unable to find Y_ComponentEnumInfo for " + std::to_string((int)comp));
}

void
TelemetryPlot::addSource_(
		gpo::TelemetrySourcePtr telemSrc,
		const std::string &label,
		QColor color,
		AxisSide yAxisSide,
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

	auto yCompSelected = yComponent_;
	auto yAxisSelected = yAxis;
	if (yAxisSide == AxisSide::eAS_Side2)
	{
		yCompSelected = yComponent2_;
		yAxisSelected = yAxis2;
		yAxis2->setVisible(true);
	}

	SourceObjects sourceObjs;
	sourceObjs.telemSrc = telemSrc;
	QVector<double> xData(telemSrc->size()), yData(telemSrc->size());
	addGraph(xAxis,yAxisSelected);
	sourceObjs.graph = graph(graphCount() - 1);
	sourceObjs.graph->setData(xData,yData,true);
	sourceObjs.graph->setName(label.c_str());
	sourceObjs.graph->setPen(QPen(color));
	sourceObjs.alignmentIdxAtLastReplot = telemSrc->seeker()->getAlignmentIdx();
	sourceObjs.yAxisSide = yAxisSide;
	setX_Data(sourceObjs,xComponent_);
	setY_Data(sourceObjs,yCompSelected);
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
			dataItr->key = telemSrc->at(i).t_offset - alignmentSamp.t_offset;
			xAxis->setLabel("time (s)");
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
		case Y_Component::eYC_UNKNOWN:
			dataItr->value = 0;
			break;
		case Y_Component::eYC_TIME:
			dataItr->value = tSamp.t_offset;
			break;
		case Y_Component::eYC_ACCL_X:
			dataItr->value = tSamp.gpSamp.accl.x;
			break;
		case Y_Component::eYC_ACCL_Y:
			dataItr->value = tSamp.gpSamp.accl.y;
			break;
		case Y_Component::eYC_ACCL_Z:
			dataItr->value = tSamp.gpSamp.accl.z;
			break;
		case Y_Component::eYC_GYRO_X:
			dataItr->value = tSamp.gpSamp.gyro.x;
			break;
		case Y_Component::eYC_GYRO_Y:
			dataItr->value = tSamp.gpSamp.gyro.y;
			break;
		case Y_Component::eYC_GYRO_Z:
			dataItr->value = tSamp.gpSamp.gyro.z;
			break;
		case Y_Component::eYC_GRAV_X:
			dataItr->value = tSamp.gpSamp.grav.x;
			break;
		case Y_Component::eYC_GRAV_Y:
			dataItr->value = tSamp.gpSamp.grav.y;
			break;
		case Y_Component::eYC_GRAV_Z:
			dataItr->value = tSamp.gpSamp.grav.z;
			break;
		case Y_Component::eYC_CORI_W:
			dataItr->value = tSamp.gpSamp.cori.w;
			break;
		case Y_Component::eYC_CORI_X:
			dataItr->value = tSamp.gpSamp.cori.x;
			break;
		case Y_Component::eYC_CORI_Y:
			dataItr->value = tSamp.gpSamp.cori.y;
			break;
		case Y_Component::eYC_CORI_Z:
			dataItr->value = tSamp.gpSamp.cori.z;
			break;
		case Y_Component::eYC_GPS_LAT:
			dataItr->value = tSamp.gpSamp.gps.coord.lat;
			break;
		case Y_Component::eYC_GPS_LON:
			dataItr->value = tSamp.gpSamp.gps.coord.lon;
			break;
		case Y_Component::eYC_GPS_SPEED2D:
			dataItr->value = tSamp.gpSamp.gps.speed2D;
			break;
		case Y_Component::eYC_GPS_SPEED3D:
			dataItr->value = tSamp.gpSamp.gps.speed3D;
			break;
		case Y_Component::eYC_ECU_ENGINE_SPEED:
			dataItr->value = tSamp.ecuSamp.engineSpeed_rpm;
			break;
		case Y_Component::eYC_ECU_TPS:
			dataItr->value = tSamp.ecuSamp.tps;
			break;
		case Y_Component::eYC_ECU_BOOST:
			dataItr->value = tSamp.ecuSamp.boost_psi;
			break;
		}
	}
}
