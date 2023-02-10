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
		samp.t_offset = 0.010 * i;
		samp.gpSamp.gps.coord.lat = i;
		samp.gpSamp.gps.coord.lon = 0;
	}

	gpo::Track track(path);
	track.setStart(5);
	track.setFinish(95);
	track.addSector("Sector1",10,30);
	track.addSector("Sector2",30,50);
	track.addSector("Sector3",60,70);

	gpo::TrackDataAvailBitSet trackAvail;
	CPPUNIT_ASSERT_EQUAL(true, utils::computeTrackTimes(&track,tSamps,trackAvail));

	CPPUNIT_ASSERT_EQUAL(PATH_LENGTH, tSamps->size());
	for (size_t i=0; i<tSamps->size(); i++)
	{
		const auto &samp = tSamps->at(i);

		// printf("i: %3ld; lap: %+d (time: %3.3fs); sector: %+d (time: %3.3fs);\n",
		// 	i,
		// 	samp.trackData.lap, samp.trackData.lapTimeOffset,
		// 	samp.trackData.sector, samp.trackData.sectorTimeOffset);

		if (i < 5)// cross start
		{
			CPPUNIT_ASSERT_EQUAL(-1, samp.trackData.lap);
			CPPUNIT_ASSERT_EQUAL(-1, samp.trackData.sector);
			CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, samp.trackData.lapTimeOffset, 0.001);
			CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, samp.trackData.sectorTimeOffset, 0.001);
		}
		else if (i < 10)// entered sector1
		{
			CPPUNIT_ASSERT_EQUAL(1, samp.trackData.lap);
			CPPUNIT_ASSERT_EQUAL(-1, samp.trackData.sector);
		}
		else if (i < 30)// exited sector1 AND entered sector2
		{
			CPPUNIT_ASSERT_EQUAL(1, samp.trackData.lap);
			CPPUNIT_ASSERT_EQUAL(1, samp.trackData.sector);
		}
		else if (i < 50)// exited sector2
		{
			CPPUNIT_ASSERT_EQUAL(1, samp.trackData.lap);
			CPPUNIT_ASSERT_EQUAL(2, samp.trackData.sector);
		}
		else if (i < 60)// entered sector3
		{
			CPPUNIT_ASSERT_EQUAL(1, samp.trackData.lap);
			CPPUNIT_ASSERT_EQUAL(-1, samp.trackData.sector);
		}
		else if (i < 70)// exited sector3
		{
			CPPUNIT_ASSERT_EQUAL(1, samp.trackData.lap);
			CPPUNIT_ASSERT_EQUAL(3, samp.trackData.sector);
		}
		else if (i < 95)// cross finish
		{
			CPPUNIT_ASSERT_EQUAL(1, samp.trackData.lap);
			CPPUNIT_ASSERT_EQUAL(-1, samp.trackData.sector);
		}
		else// after finish
		{
			CPPUNIT_ASSERT_EQUAL(-1, samp.trackData.lap);
			CPPUNIT_ASSERT_EQUAL(-1, samp.trackData.sector);
		}
	}
}

void
DataProcessingUtilsTest::smoothMovingAvg()
{
	const size_t N = 10;
	double dataA[N];
	double dataB[N];

	// window of 1 should do no averaging
	dataA[0] = 0.0;
	dataA[1] = 1.0;
	dataA[2] = 2.0;
	dataA[3] = 3.0;
	dataA[4] = 4.0;
	dataA[5] = 5.0;
	dataA[6] = 6.0;
	dataA[7] = 7.0;
	dataA[8] = 8.0;
	dataA[9] = 9.0;
	utils::smoothMovingAvg<double>(dataA,dataB,N,1);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.000, dataB[0], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.000, dataB[1], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(2.000, dataB[2], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(3.000, dataB[3], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(4.000, dataB[4], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(5.000, dataB[5], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(6.000, dataB[6], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(7.000, dataB[7], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(8.000, dataB[8], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(9.000, dataB[9], 0.001);

	// ----------------

	// window of 3
	dataA[0] = 0.0;// windowSum = 1.0,  windowSize = 2, mean = 0.500
	dataA[1] = 1.0;// windowSum = 3.0,  windowSize = 3, mean = 1.000
	dataA[2] = 2.0;// windowSum = 6.0,  windowSize = 3, mean = 2.000
	dataA[3] = 3.0;// windowSum = 9.0,  windowSize = 3, mean = 3.000
	dataA[4] = 4.0;// windowSum = 12.0, windowSize = 3, mean = 4.000
	dataA[5] = 5.0;// windowSum = 15.0, windowSize = 3, mean = 5.000
	dataA[6] = 6.0;// windowSum = 18.0, windowSize = 3, mean = 6.000
	dataA[7] = 7.0;// windowSum = 21.0, windowSize = 3, mean = 7.000
	dataA[8] = 8.0;// windowSum = 24.0, windowSize = 3, mean = 8.000
	dataA[9] = 9.0;// windowSum = 17.0, windowSize = 2, mean = 8.500
	utils::smoothMovingAvg<double>(dataA,dataB,N,3);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.500, dataB[0], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.000, dataB[1], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(2.000, dataB[2], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(3.000, dataB[3], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(4.000, dataB[4], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(5.000, dataB[5], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(6.000, dataB[6], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(7.000, dataB[7], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(8.000, dataB[8], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(8.500, dataB[9], 0.001);

	// ----------------

	// window of 5
	dataA[0] = 0.3;// windowSum = 1.4,  windowSize = 3, mean = 0.466
	dataA[1] = 0.5;// windowSum = 2.3,  windowSize = 4, mean = 0.575
	dataA[2] = 0.6;// windowSum = 3.5,  windowSize = 5, mean = 0.700
	dataA[3] = 0.9;// windowSum = 5.2,  windowSize = 5, mean = 1.040
	dataA[4] = 1.2;// windowSum = 7.7,  windowSize = 5, mean = 1.540
	dataA[5] = 2.0;// windowSum = 10.6, windowSize = 5, mean = 2.120
	dataA[6] = 3.0;// windowSum = 15.7, windowSize = 5, mean = 3.140
	dataA[7] = 3.5;// windowSum = 19.0, windowSize = 5, mean = 3.900
	dataA[8] = 6.0;// windowSum = 17.5, windowSize = 4, mean = 4.375
	dataA[9] = 5.0;// windowSum = 14.5, windowSize = 3, mean = 4.833
	utils::smoothMovingAvg<double>(dataA,dataB,N,5);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.466, dataB[0], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.575, dataB[1], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.700, dataB[2], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.040, dataB[3], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.540, dataB[4], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(2.120, dataB[5], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(3.140, dataB[6], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(3.900, dataB[7], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(4.375, dataB[8], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(4.833, dataB[9], 0.001);

	// ----------------

	// window of 5 in-place
	dataA[0] = 0.3;// windowSum = 1.4,  windowSize = 3, mean = 0.466
	dataA[1] = 0.5;// windowSum = 2.3,  windowSize = 4, mean = 0.575
	dataA[2] = 0.6;// windowSum = 3.5,  windowSize = 5, mean = 0.700
	dataA[3] = 0.9;// windowSum = 5.2,  windowSize = 5, mean = 1.040
	dataA[4] = 1.2;// windowSum = 7.7,  windowSize = 5, mean = 1.540
	dataA[5] = 2.0;// windowSum = 10.6, windowSize = 5, mean = 2.120
	dataA[6] = 3.0;// windowSum = 15.7, windowSize = 5, mean = 3.140
	dataA[7] = 3.5;// windowSum = 19.0, windowSize = 5, mean = 3.900
	dataA[8] = 6.0;// windowSum = 17.5, windowSize = 4, mean = 4.375
	dataA[9] = 5.0;// windowSum = 14.5, windowSize = 3, mean = 4.833
	utils::smoothMovingAvg<double>(dataA,dataA,N,5);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.466, dataA[0], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.575, dataA[1], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.700, dataA[2], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.040, dataA[3], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.540, dataA[4], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(2.120, dataA[5], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(3.140, dataA[6], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(3.900, dataA[7], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(4.375, dataA[8], 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(4.833, dataA[9], 0.001);
}

void
DataProcessingUtilsTest::smoothMovingAvgStructured()
{
	struct TestStruct
	{
		double a;
		int b;
		double c;
		long long d;
		char e[10];
		double f;
	};
	const size_t N = 10;
	TestStruct dataA[N];
	TestStruct dataB[N];
	for (size_t i=0; i<N; i++)
	{
		dataA[i].a = i;
		dataA[i].c = i*2;
		dataA[i].f = i*3;
	}

	// window of 1 should do no averaging
	size_t fieldOffsets[] = {
		offsetof(TestStruct, a),
		offsetof(TestStruct, c),
		offsetof(TestStruct, f)
	};
	utils::smoothMovingAvgStructured<TestStruct,double>(dataA,dataB,fieldOffsets,3,N,3);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.500, dataB[0].a, 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.000, dataB[1].a, 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(2.000, dataB[2].a, 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(3.000, dataB[3].a, 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(4.000, dataB[4].a, 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(5.000, dataB[5].a, 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(6.000, dataB[6].a, 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(7.000, dataB[7].a, 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(8.000, dataB[8].a, 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(8.500, dataB[9].a, 0.001);
	// TODO check 'c' and 'f' fields too
}

int main()
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(DataProcessingUtilsTest::suite());
	return runner.run() ? 0 : EXIT_FAILURE;
}
