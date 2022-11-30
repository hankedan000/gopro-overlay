#include "DataProcessingUtilsTest.h"

#include "GoProOverlay/utils/DataProcessingUtils.h"

DataProcessingUtilsTest::DataProcessingUtilsTest()
{
}

void
DataProcessingUtilsTest::setUp()
{
	// run before each test case
}

void
DataProcessingUtilsTest::tearDown()
{
	// run after each test case
}

void
DataProcessingUtilsTest::trackTimes()
{
	const size_t PATH_LENGTH = 100;
	std::vector<cv::Vec2d> path;
	path.resize(PATH_LENGTH);
	auto tSamps = gpo::TelemetrySamplesPtr(new gpo::TelemetrySamples());
	tSamps->resize(PATH_LENGTH);
	for (unsigned int i=0; i<PATH_LENGTH; i++)
	{
		path.at(i) = cv::Vec2d(i, 0);

		auto &samp = tSamps->at(i);
		samp.gpSamp.t_offset = 0.010 * i;
		samp.gpSamp.gps.coord.lat = i;
		samp.gpSamp.gps.coord.lon = 0;
	}

	gpo::Track track(path);
	track.setStart(5);
	track.setFinish(95);
	track.addSector("Sector1",10,30);
	track.addSector("Sector2",30,50);
	track.addSector("Sector3",60,70);

	CPPUNIT_ASSERT_EQUAL(true, utils::computeTrackTimes(&track,tSamps));

	CPPUNIT_ASSERT_EQUAL(PATH_LENGTH, tSamps->size());
	for (size_t i=0; i<tSamps->size(); i++)
	{
		const auto &samp = tSamps->at(i);

		// printf("i: %3ld; lap: %+d (time: %3.3fs); sector: %+d (time: %3.3fs);\n",
		// 	i,
		// 	samp.lap, samp.lapTimeOffset,
		// 	samp.sector, samp.sectorTimeOffset);

		if (i < 5)// cross start
		{
			CPPUNIT_ASSERT_EQUAL(-1, samp.lap);
			CPPUNIT_ASSERT_EQUAL(-1, samp.sector);
			CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, samp.lapTimeOffset, 0.001);
			CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, samp.sectorTimeOffset, 0.001);
		}
		else if (i < 10)// entered sector1
		{
			CPPUNIT_ASSERT_EQUAL(1, samp.lap);
			CPPUNIT_ASSERT_EQUAL(-1, samp.sector);
		}
		else if (i < 30)// exited sector1 AND entered sector2
		{
			CPPUNIT_ASSERT_EQUAL(1, samp.lap);
			CPPUNIT_ASSERT_EQUAL(0, samp.sector);
		}
		else if (i < 50)// exited sector2
		{
			CPPUNIT_ASSERT_EQUAL(1, samp.lap);
			CPPUNIT_ASSERT_EQUAL(1, samp.sector);
		}
		else if (i < 60)// entered sector3
		{
			CPPUNIT_ASSERT_EQUAL(1, samp.lap);
			CPPUNIT_ASSERT_EQUAL(-1, samp.sector);
		}
		else if (i < 70)// exited sector3
		{
			CPPUNIT_ASSERT_EQUAL(1, samp.lap);
			CPPUNIT_ASSERT_EQUAL(2, samp.sector);
		}
		else if (i < 95)// cross finish
		{
			CPPUNIT_ASSERT_EQUAL(1, samp.lap);
			CPPUNIT_ASSERT_EQUAL(-1, samp.sector);
		}
		else// after finish
		{
			CPPUNIT_ASSERT_EQUAL(-1, samp.lap);
			CPPUNIT_ASSERT_EQUAL(-1, samp.sector);
		}
	}
}

int main()
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(DataProcessingUtilsTest::suite());
	return runner.run() ? 0 : EXIT_FAILURE;
}
