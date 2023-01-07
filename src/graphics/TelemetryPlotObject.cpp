#include "GoProOverlay/graphics/TelemetryPlotObject.h"

namespace gpo
{

    const int PLOT_RENDER_WIDTH = 853;
    const int PLOT_RENDER_HEIGHT = 480;

	TelemetryPlotObject::TelemetryPlotObject()
	 : RenderedObject(PLOT_RENDER_WIDTH,PLOT_RENDER_HEIGHT)
	 , app_(nullptr)
	 , plot_(nullptr)
	 , plotWidthTime_sec_(0)
	 , calculatedFPS_(0)
	{
        if (QApplication::instance() == nullptr)
        {
            int argn = 0;
            app_ = new QApplication(argn,nullptr);
        }
		plot_ = new TelemetryPlot(nullptr);
        plot_->resize(PLOT_RENDER_WIDTH,PLOT_RENDER_HEIGHT);
        plot_->setY_Component(TelemetryPlot::Y_Component::eYC_AcclX,false);
	}

	TelemetryPlotObject::~TelemetryPlotObject()
	{
		if (plot_)
		{
			delete(plot_);
		}
		if (app_)
		{
			delete(app_);
		}
	}

	std::string
	TelemetryPlotObject::typeName() const
	{
		return "TelemetryPlotObject";
	}

	DataSourceRequirements
	TelemetryPlotObject::dataSourceRequirements() const
	{
        return DataSourceRequirements(0,gpo::DSR_ONE_OR_MORE,0);
	}

	void
	TelemetryPlotObject::render()
	{
        auto pixmap = plot_->toPixmap(PLOT_RENDER_WIDTH,PLOT_RENDER_HEIGHT);
        auto image = pixmap.toImage();
        // make a temporary cv::Mat with the QImage's memory
        cv::Mat tmpMat(image.height(), image.width(), CV_8UC4, (cv::Scalar*)image.scanLine(0));
        tmpMat.copyTo(outImg_);
	}

	void
	TelemetryPlotObject::setX_Component(
			TelemetryPlot::X_Component comp)
	{
		plot_->setX_Component(comp);
	}

	TelemetryPlot::X_Component
	TelemetryPlotObject::getX_Component() const
	{
		return plot_->getX_Component();
	}

	void
	TelemetryPlotObject::setY_Component(
			TelemetryPlot::Y_Component comp)
	{
		plot_->setY_Component(comp);
	}

	TelemetryPlot::Y_Component
	TelemetryPlotObject::getY_Component() const
	{
		return plot_->getY_Component();
	}

	void
	TelemetryPlotObject::setPlotWidthSeconds(
		double duration_sec)
	{
		plotWidthTime_sec_ = duration_sec;
	}

	double
	TelemetryPlotObject::getPlotWidthSeconds() const
	{
		return plotWidthTime_sec_;
	}

	void
	TelemetryPlotObject::sourcesValid()
	{
        for (auto &telemSrc : tSources_)
        {
			// derive FPS
			if (telemSrc->size() >= 2)
			{
				calculatedFPS_ = 1.0 /
					(telemSrc->at(1).gpSamp.t_offset - telemSrc->at(0).gpSamp.t_offset);
			}

			// only add the telemetry to the plot if it hasn't been added already
            bool alreadyAdded = false;
            for (size_t i=0; i<plot_->numSources(); i++)
            {
                if (plot_->getSource(i).get() == telemSrc.get())
                {
                    alreadyAdded = true;
                    break;
                }
            }
            if ( ! alreadyAdded)
            {
                plot_->addSource(telemSrc);
            }
			// FIXME add a way to remove sources too
        }
	}

	YAML::Node
	TelemetryPlotObject::subEncode() const
	{
		YAML::Node node;

		node["xComponent"] = (int)getX_Component();
		node["yComponent"] = (int)getY_Component();
		node["plotWidthTime_sec"] = plotWidthTime_sec_;

		return node;
	}

	bool
	TelemetryPlotObject::subDecode(
		const YAML::Node& node)
	{
		int compInt;
		TelemetryPlot::X_Component xComponent;
		YAML_TO_FIELD(node,"xComponent",compInt);
		xComponent = (TelemetryPlot::X_Component)(compInt);
		setX_Component(xComponent);

		TelemetryPlot::Y_Component yComponent;
		YAML_TO_FIELD(node,"yComponent",compInt);
		yComponent = (TelemetryPlot::Y_Component)(compInt);
		setY_Component(yComponent);

		YAML_TO_FIELD(node,"plotWidthTime_sec",plotWidthTime_sec_);

		return true;
	}

}
