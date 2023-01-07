#include "GoProOverlay/graphics/TelemetryPlotObject.h"

namespace gpo
{

    const int PLOT_RENDER_WIDTH = 853;
    const int PLOT_RENDER_HEIGHT = 480;

	TelemetryPlotObject::TelemetryPlotObject()
	 : RenderedObject(PLOT_RENDER_WIDTH,PLOT_RENDER_HEIGHT)
	 , app_(nullptr)
	 , plot_(nullptr)
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
	TelemetryPlotObject::sourcesValid()
	{
        for (auto &telemSrc : tSources_)
        {
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
        }
	}

	YAML::Node
	TelemetryPlotObject::subEncode() const
	{
		YAML::Node node;
		return node;
	}

	bool
	TelemetryPlotObject::subDecode(
		const YAML::Node& node)
	{
		return true;
	}

}
