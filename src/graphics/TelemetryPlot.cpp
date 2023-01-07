#include "GoProOverlay/graphics/TelemetryPlot.h"

TelemetryPlot::TelemetryPlot(
		QWidget *parent)
: QCustomPlot(parent)
, xComponent_(X_Component::eXC_Samples)
, yComponent_(Y_Component::eYC_Unknown)
{
	setInteraction(QCP::Interaction::iRangeDrag,true);
	setInteraction(QCP::Interaction::iRangeZoom,true);
	legend->setVisible(true);
}

TelemetryPlot::~TelemetryPlot()
{}

void
TelemetryPlot::addSource(
		gpo::TelemetrySourcePtr telemSrc,
		bool replot)
{
	SourceObjects sourceObjs;
	sourceObjs.telemSrc = telemSrc;
	QVector<double> xData(telemSrc->size()), yData(telemSrc->size());
	addGraph();
	sourceObjs.graph = graph(graphCount() - 1);
	sourceObjs.graph->setData(xData,yData,true);
	sourceObjs.graph->setName(telemSrc->getDataSourceName().c_str());
	sourceObjs.graph->setPen(QPen(DEFAULT_COLORS[sources_.size() % N_DEFAULT_COLORS]));
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

void
TelemetryPlot::setX_Data(
		SourceObjects &sourceObjs,
		X_Component comp)
{
	auto &telemSrc = sourceObjs.telemSrc;
	size_t seekedIdx = telemSrc->seekedIdx();
	auto &seekedSamp = telemSrc->at(seekedIdx);
	auto dataPtr = sourceObjs.graph->data();
	auto dataItr = dataPtr->begin();
	for (size_t i=0; i<telemSrc->size() && dataItr!=dataPtr->end(); i++, dataItr++)
	{
		switch (comp)
		{
		case X_Component::eXC_Samples:
			dataItr->key = (double)(i) - seekedIdx;
			break;
		case X_Component::eXC_Time:
			dataItr->key = telemSrc->at(i).gpSamp.t_offset - seekedSamp.gpSamp.t_offset;
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
			break;
		case Y_Component::eYC_Time:
			dataItr->value = tSamp.gpSamp.t_offset;
			break;
		case Y_Component::eYC_AcclX:
			dataItr->value = tSamp.gpSamp.accl.x;
			break;
		case Y_Component::eYC_AcclY:
			dataItr->value = tSamp.gpSamp.accl.y;
			break;
		case Y_Component::eYC_AcclZ:
			dataItr->value = tSamp.gpSamp.accl.z;
			break;
		case Y_Component::eYC_GyroX:
			dataItr->value = tSamp.gpSamp.gyro.x;
			break;
		case Y_Component::eYC_GyroY:
			dataItr->value = tSamp.gpSamp.gyro.y;
			break;
		case Y_Component::eYC_GyroZ:
			dataItr->value = tSamp.gpSamp.gyro.z;
			break;
		default:
			printf("%s - unsupported Y_Component (%d)\n",__func__,(int)(comp));
			dataItr->value = 0;
			break;
		}
	}
}
