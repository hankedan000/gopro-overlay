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
			break;
		case X_Component::eXC_Time:
			dataItr->key = telemSrc->at(i).gpSamp.t_offset - alignmentSamp.gpSamp.t_offset;
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
			plotTitle_->setText("");
			break;
		case Y_Component::eYC_Time:
			dataItr->value = tSamp.gpSamp.t_offset;
			plotTitle_->setText("Time");
			break;
		case Y_Component::eYC_AcclX:
			dataItr->value = tSamp.gpSamp.accl.x;
			plotTitle_->setText("X Acceleration");
			break;
		case Y_Component::eYC_AcclY:
			dataItr->value = tSamp.gpSamp.accl.y;
			plotTitle_->setText("Y Acceleration");
			break;
		case Y_Component::eYC_AcclZ:
			dataItr->value = tSamp.gpSamp.accl.z;
			plotTitle_->setText("Z Acceleration");
			break;
		case Y_Component::eYC_GyroX:
			dataItr->value = tSamp.gpSamp.gyro.x;
			plotTitle_->setText("X Gyroscope");
			break;
		case Y_Component::eYC_GyroY:
			dataItr->value = tSamp.gpSamp.gyro.y;
			plotTitle_->setText("Y Gyroscope");
			break;
		case Y_Component::eYC_GyroZ:
			dataItr->value = tSamp.gpSamp.gyro.z;
			plotTitle_->setText("Z Gyroscope");
			break;
		case Y_Component::eYC_GPS_Speed2D:
			dataItr->value = tSamp.gpSamp.gps.speed2D;
			plotTitle_->setText("GPS 2D Speed");
			break;
		case Y_Component::eYC_GPS_Speed3D:
			dataItr->value = tSamp.gpSamp.gps.speed3D;
			plotTitle_->setText("GPS 3D Speed");
			break;
		default:
			printf("%s - unsupported Y_Component (%d)\n",__func__,(int)(comp));
			dataItr->value = 0;
			plotTitle_->setText("**INVALID Y COMPONENT**");
			break;
		}
	}
}
