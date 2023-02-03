#include "GoProOverlay/data/DataSource.h"
#include "GoProOverlay/utils/DataProcessingUtils.h"

#include "AlignmentPlot.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

    auto src1 = gpo::DataSource::loadDataFromVideo("/home/daniel/Downloads/Autocross/20230115_MSCC/GH010181.MP4");

    std::vector<gpo::VehicleTelemetry> vTelem;
    utils::readMegaSquirtLog(
        "/home/daniel/Downloads/Autocross/20230115_MSCC/msq_logs/2021-05-12_09.37.04.msl",
        vTelem);
    std::vector<gpo::VehicleTelemetry> vTelemResamp;
    utils::resample(vTelemResamp,vTelem,59.94);
    std::vector<gpo::TelemetrySample> tSamps;
    tSamps.resize(vTelemResamp.size());
    for (size_t i=0; i<tSamps.size(); i++)
    {
        tSamps[i].gpSamp.t_offset = vTelemResamp[i].t_offset;
        tSamps[i].vehSamp = vTelemResamp[i];
    }
    auto src2 = gpo::DataSource::makeDataFromTelemetry(tSamps);

    auto plot = new AlignmentPlot();
    // plot->setY_Component(TelemetryPlot::Y_Component::eYC_GPS_Speed2D);
    plot->setSourceA(src1->telemSrc);
    plot->setSourceB(src2->telemSrc);
    plot->show();

    printf("telem1.size_bytes() = %ld\n", src1->telemSrc->size_bytes());
    printf("telem2.size_bytes() = %ld\n", src2->telemSrc->size_bytes());

	return a.exec();
}
