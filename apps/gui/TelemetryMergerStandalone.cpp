#include "TelemetryMerger.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

    auto merger = new TelemetryMerger();
    merger->show();

    for (int argIdx=1; argIdx<argc; argIdx++)
    {
        merger->addSourceFromFile(argv[argIdx]);
    }

	return a.exec();
}
