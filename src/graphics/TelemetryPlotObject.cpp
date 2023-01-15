#include "GoProOverlay/graphics/TelemetryPlotObject.h"

namespace gpo
{

    const int PLOT_RENDER_WIDTH = 853;
    const int PLOT_RENDER_HEIGHT = 480;

	TelemetryPlotObject::TelemetryPlotObject()
	 : RenderedObject(PLOT_RENDER_WIDTH,PLOT_RENDER_HEIGHT)
	 , fakeApp_()
	 , plot_(nullptr)
	 , plotWidthTime_sec_(6.0)
	 , calculatedFPS_(0)
	{
		fakeApp_.app = nullptr;
        if (QApplication::instance() == nullptr)
        {
			fakeApp_.argc = 1;
			fakeApp_.argv[0] = (char *)malloc(100);
			strcpy(fakeApp_.argv[0],"FakeQtApp");
            fakeApp_.app = new QApplication(fakeApp_.argc,fakeApp_.argv);
        }
		plot_ = new TelemetryPlot(nullptr);
		plot_->setMaximumSize(PLOT_RENDER_WIDTH, PLOT_RENDER_HEIGHT);
		plot_->setMinimumSize(PLOT_RENDER_WIDTH, PLOT_RENDER_HEIGHT);
		// default to plotting GPS speed
        plot_->setY_Component(TelemetryPlot::Y_Component::eYC_GPS_Speed2D,false);
		plot_->applyDarkTheme();
	}

	TelemetryPlotObject::~TelemetryPlotObject()
	{
		if (plot_)
		{
			delete(plot_);
		}
		if (fakeApp_.app)
		{
			delete(fakeApp_.app);
			delete(fakeApp_.argv[0]);
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
		if (tSources_.size() > 0)
		{
			auto telemSrc = tSources_.front();
			auto seeker = telemSrc->seeker();
			auto offsetFromAlignment = (long long)(seeker->seekedIdx()) - seeker->getAlignmentIdx();

			// compute x-range to have right side aligned with 1st dataset's current
			// seeked location, and the width sized to fit N amount of seconds worth of data.
			size_t windowWidthSamples = std::round(plotWidthTime_sec_ * calculatedFPS_);
			double xRangeUpper = offsetFromAlignment;
			double xRangeLower = xRangeUpper - windowWidthSamples;
			plot_->xAxis->setRange(xRangeLower, xRangeUpper);
			plot_->replot(QCustomPlot::RefreshPriority::rpImmediateRefresh);
		}

        auto pixmap = plot_->toPixmap(PLOT_RENDER_WIDTH,PLOT_RENDER_HEIGHT);
        auto image = pixmap.toImage();
        // make a temporary cv::Mat with the QImage's memory
        cv::Mat tmpMat(image.height(), image.width(), CV_8UC4, (cv::Scalar*)image.scanLine(0));
        tmpMat.copyTo(outImg_);
	}

	void
	TelemetryPlotObject::setTelemetryColor(
		gpo::TelemetrySourcePtr telemSrc,
		QColor color)
	{
		plot_->setTelemetryColor(telemSrc,color,false);// hold off replot until render()
	}

	std::pair<bool,QColor>
	TelemetryPlotObject::getTelemetryColor(
		gpo::TelemetrySourcePtr telemSrc) const
	{
		return plot_->getTelemetryColor(telemSrc);
	}

	void
	TelemetryPlotObject::setTelemetryLabel(
		gpo::TelemetrySourcePtr telemSrc,
		const std::string &label)
	{
		plot_->setTelemetryLabel(telemSrc,label,false);// hold off replot until render()
	}

	std::pair<bool,std::string>
	TelemetryPlotObject::getTelemetryLabel(
		gpo::TelemetrySourcePtr telemSrc) const
	{
		return plot_->getTelemetryLabel(telemSrc);
	}

	void
	TelemetryPlotObject::setX_Component(
			TelemetryPlot::X_Component comp)
	{
		plot_->setX_Component(comp,false);// hold off replot until render()
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
		plot_->setY_Component(comp,false);// hold off replot until render()

		// touch up plot titles so they are more understandable to the 'layperson'
		// when rendering videos "X,Y,Z Acceleration" doesn't mean must to people
		switch (comp)
		{
			case TelemetryPlot::Y_Component::eYC_AcclX:
				plot_->setPlotTitle("Left/Right Acceleration");
				break;
			case TelemetryPlot::Y_Component::eYC_AcclY:
				plot_->setPlotTitle("Front/Back Acceleration");
				break;
			default:
				// leave TelemetryPlot's default title
				break;
		}
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

		// FIXME add support to save/restore telemetry-specific properties.
		// assume we're restoring the top/bottom video style when there are 2 sources
		if (tSources_.size() == 2)
		{
			plot_->setTelemetryLabel(tSources_[0], "Run A",false);
			plot_->setTelemetryLabel(tSources_[1], "Run B",false);
			plot_->setTelemetryColor(tSources_[0], QColor(255,0,0,255),false);
			plot_->setTelemetryColor(tSources_[1], QColor(0,255,255,255),false);
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
