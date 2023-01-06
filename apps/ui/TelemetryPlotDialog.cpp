#include "TelemetryPlotDialog.h"

TelemetryPlotDialog::TelemetryPlotDialog(
        QWidget *parent)
 : PlotDialog(parent)
{
    plot()->setInteraction(QCP::Interaction::iRangeDrag,true);
    plot()->setInteraction(QCP::Interaction::iRangeZoom,true);
    plot()->legend->setVisible(true);
}

TelemetryPlotDialog::~TelemetryPlotDialog()
{}

void
TelemetryPlotDialog::addSource(
        gpo::TelemetrySourcePtr telemSrc,
        bool replot)
{
    SourceObjects sourceObjs;
    sourceObjs.telemSrc = telemSrc;
    sourceObjs.xData.resize(telemSrc->size());
    sourceObjs.yData.resize(telemSrc->size());
    setX_Data(sourceObjs,false);
    setY_Data(sourceObjs,telemComponent_);
    plot()->addGraph();
    sourceObjs.graph = plot()->graph(plot()->graphCount() - 1);
    sourceObjs.graph->setName(telemSrc->getDataSourceName().c_str());
    sourceObjs.graph->setPen(QPen(DEFAULT_COLORS[sources_.size() % N_DEFAULT_COLORS]));
    sourceObjs.graph->setData(sourceObjs.xData,sourceObjs.yData,true);
    plot()->rescaleAxes();

    sources_.push_back(sourceObjs);

    if (replot)
    {
        plot()->replot();
    }
}

void
TelemetryPlotDialog::removeSource(
        size_t idx,
        bool replot)
{
    auto sourceItr = std::next(sources_.begin(),idx);
    if (sourceItr != sources_.end())
    {
        // remove the source's graph
        plot()->removeGraph(sourceItr->graph);

        sources_.erase(sourceItr);
    }

    if (replot)
    {
        plot()->replot();
    }
}

void
TelemetryPlotDialog::clear(
        bool replot)
{
    while ( ! sources_.empty())
    {
        removeSource(0,false);// hold off replotting until the end
    }

    if (replot)
    {
        plot()->replot();
    }
}

gpo::TelemetrySourcePtr
TelemetryPlotDialog::getSource(
        size_t idx)
{
    return sources_.at(idx).telemSrc;
}

size_t
TelemetryPlotDialog::numSources() const
{
    return sources_.size();
}

void
TelemetryPlotDialog::setTelemetryComponent(
        TelemetryComponent comp,
        bool replot)
{
    if (telemComponent_ == comp)
    {
        // nothing to do
        return;
    }

    // perform updates
    telemComponent_ = comp;
    for (auto &sourceObjs : sources_)
    {
        setY_Data(sourceObjs,telemComponent_);
    }
    plot()->rescaleAxes();

    if (replot)
    {
        plot()->replot();
    }
}

void
TelemetryPlotDialog::setX_Data(
        SourceObjects &sourceObjs,
        bool isTime)
{
    auto &xData = sourceObjs.xData;
    for (size_t i=0; i<sourceObjs.telemSrc->size() && i<sourceObjs.xData.size(); i++)
    {
        if (isTime)
        {
            xData[i] = sourceObjs.telemSrc->at(i).gpSamp.t_offset;
        }
        else
        {
            xData[i] = i;
        }
    }
}

void
TelemetryPlotDialog::setY_Data(
        SourceObjects &sourceObjs,
        TelemetryComponent comp)
{
    auto &yData = sourceObjs.yData;
    for (size_t i=0; i<sourceObjs.telemSrc->size() && i<sourceObjs.yData.size(); i++)
    {
        auto &tSamp = sourceObjs.telemSrc->at(i);
        switch (comp)
        {
            case TelemetryComponent::eTC_Unknown:
                yData[i] = 0;
                break;
            case TelemetryComponent::eTC_Time:
                yData[i] = tSamp.gpSamp.t_offset;
                break;
            case TelemetryComponent::eTC_AcclX:
                yData[i] = tSamp.gpSamp.accl.x;
                break;
            case TelemetryComponent::eTC_AcclY:
                yData[i] = tSamp.gpSamp.accl.y;
                break;
            case TelemetryComponent::eTC_AcclZ:
                yData[i] = tSamp.gpSamp.accl.z;
                break;
            case TelemetryComponent::eTC_GyroX:
                yData[i] = tSamp.gpSamp.gyro.x;
                break;
            case TelemetryComponent::eTC_GyroY:
                yData[i] = tSamp.gpSamp.gyro.y;
                break;
            case TelemetryComponent::eTC_GyroZ:
                yData[i] = tSamp.gpSamp.gyro.z;
                break;
            default:
                printf("%s - unsupported TelemetryComponent (%d)\n",__func__,(int)(comp));
                yData[i] = 0;
                break;
        }
    }
}
