#include "DataSourceTest.h"

#include "GoProOverlay/data/DataSource.h"
#include "test_data.h"

DataSourceTest::DataSourceTest()
{
	printf("TMP_ROOT = %s\n",test_data::TMP_ROOT);
}

void
DataSourceTest::setUp()
{
	// run before each test case
}

void
DataSourceTest::tearDown()
{
	// run after each test case
}

void
DataSourceTest::testLoadFromTelemetry()
{
	const double SAMP_RATE_HZ = 100.0;
	const size_t N_SAMPS = 10;
	gpo::TelemetrySamples tSamps;
	tSamps.resize(N_SAMPS);
	for (unsigned int i=0; i<N_SAMPS; i++)
	{
		auto &samp = tSamps.at(i);
		samp.t_offset = (1.0 / SAMP_RATE_HZ) * i;
		samp.gpSamp.gps.coord.lat = i;
		samp.gpSamp.gps.coord.lon = 0;
	}

	auto srcFromTelem = gpo::DataSource::makeDataFromTelemetry(
		tSamps);
	CPPUNIT_ASSERT(srcFromTelem != nullptr);

	CPPUNIT_ASSERT_EQUAL(
		std::string(""),
		srcFromTelem->getOrigin());

	CPPUNIT_ASSERT_DOUBLES_EQUAL(SAMP_RATE_HZ, srcFromTelem->getTelemetryRate_hz(), 0.1);

	// when importing from raw telemetry, we have no idea what's available.... (for now)
	CPPUNIT_ASSERT_EQUAL(false, srcFromTelem->dataAvailable().any());

	// check samples were parsed correctly (trivial range check)
	auto tSrc = srcFromTelem->telemSrc;
	CPPUNIT_ASSERT_EQUAL((size_t)(N_SAMPS), tSrc->size());
	for (size_t i=0; i<tSrc->size(); i++)
	{
		const auto &tSamp = tSrc->at(i);
		CPPUNIT_ASSERT_DOUBLES_EQUAL((1.0 / SAMP_RATE_HZ) * i,tSamp.t_offset,0.1);
		CPPUNIT_ASSERT_DOUBLES_EQUAL((double)i,tSamp.gpSamp.gps.coord.lat,0.1);
	}
}

void
DataSourceTest::testLoadFromMegaSquirtLog()
{
	auto srcFromMsq = gpo::DataSource::loadDataFromMegaSquirtLog(
		test_data::ecu::MS2E_AUTOCROSS);
	CPPUNIT_ASSERT(srcFromMsq != nullptr);

	CPPUNIT_ASSERT_EQUAL(
		std::string(test_data::ecu::MS2E_AUTOCROSS),
		srcFromMsq->getOrigin());

	CPPUNIT_ASSERT_DOUBLES_EQUAL(14.8, srcFromMsq->getTelemetryRate_hz(), 0.1);

	// should have ECU data available
	gpo::DataAvailableBitSet expectedAvail;
	expectedAvail.reset();
	expectedAvail.set(gpo::eDA_ECU_ENGINE_SPEED);
	expectedAvail.set(gpo::eDA_ECU_TPS);
	expectedAvail.set(gpo::eDA_ECU_BOOST);
	CPPUNIT_ASSERT((srcFromMsq->dataAvailable() == expectedAvail));

	// check samples were parsed correctly (trivial range check)
	auto tSrc = srcFromMsq->telemSrc;
	CPPUNIT_ASSERT_EQUAL((size_t)(826), tSrc->size());
	for (size_t i=0; i<tSrc->size(); i++)
	{
		const auto &tSamp = tSrc->at(i);
		CPPUNIT_ASSERT(tSamp.ecuSamp.engineSpeed_rpm > 600.0);
		CPPUNIT_ASSERT(tSamp.ecuSamp.engineSpeed_rpm < 7200.0);
		CPPUNIT_ASSERT(tSamp.ecuSamp.tps > -5.0);
		CPPUNIT_ASSERT(tSamp.ecuSamp.tps < 105.0);
		CPPUNIT_ASSERT(tSamp.ecuSamp.boost_psi > -30.0);
		CPPUNIT_ASSERT(tSamp.ecuSamp.boost_psi < 12.0);
	}
}

void
DataSourceTest::testLoadFromSoloStormCSV()
{
	auto srcFromMsq = gpo::DataSource::loadDataFromSoloStormCSV(
		test_data::solostorm::AUTOCROSS);
	CPPUNIT_ASSERT(srcFromMsq != nullptr);

	CPPUNIT_ASSERT_EQUAL(
		std::string(test_data::solostorm::AUTOCROSS),
		srcFromMsq->getOrigin());

	CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, srcFromMsq->getTelemetryRate_hz(), 0.1);

	// should have ECU data available
	gpo::DataAvailableBitSet expectedAvail;
	expectedAvail.reset();
	expectedAvail.set(gpo::eDA_GOPRO_ACCL);
	expectedAvail.set(gpo::eDA_GOPRO_GPS_ALTITUDE);
	expectedAvail.set(gpo::eDA_GOPRO_GPS_SPEED2D);
	expectedAvail.set(gpo::eDA_GOPRO_GPS_LATLON);
	expectedAvail.set(gpo::eDA_ECU_ENGINE_SPEED);
	expectedAvail.set(gpo::eDA_ECU_TPS);
	expectedAvail.set(gpo::eDA_CALC_ON_TRACK_LATLON);
	expectedAvail.set(gpo::eDA_CALC_LAP);
	expectedAvail.set(gpo::eDA_CALC_LAP_TIME_OFFSET);
	expectedAvail.set(gpo::eDA_CALC_SECTOR);
	expectedAvail.set(gpo::eDA_CALC_SECTOR_TIME_OFFSET);
	CPPUNIT_ASSERT((srcFromMsq->dataAvailable() == expectedAvail));

	// check samples were parsed correctly (trivial range check)
	auto tSrc = srcFromMsq->telemSrc;
	CPPUNIT_ASSERT_EQUAL((size_t)(349), tSrc->size());
	for (size_t i=0; i<tSrc->size(); i++)
	{
		const auto &tSamp = tSrc->at(i);
		CPPUNIT_ASSERT((-1.5 * 9.81) < tSamp.gpSamp.accl.x && tSamp.gpSamp.accl.x < (+1.5 * 9.81));
		CPPUNIT_ASSERT((-1.5 * 9.81) < tSamp.gpSamp.accl.y && tSamp.gpSamp.accl.y < (+1.5 * 9.81));
		CPPUNIT_ASSERT((+0.5 * 9.81) < tSamp.gpSamp.accl.z && tSamp.gpSamp.accl.z < (+1.5 * 9.81));
		CPPUNIT_ASSERT(+28.751000 < tSamp.gpSamp.gps.coord.lat && tSamp.gpSamp.gps.coord.lat < +28.754000);
		CPPUNIT_ASSERT(-81.743000 < tSamp.gpSamp.gps.coord.lon && tSamp.gpSamp.gps.coord.lon < -81.741000);
		CPPUNIT_ASSERT(tSamp.ecuSamp.engineSpeed_rpm > 600.0);
		CPPUNIT_ASSERT(tSamp.ecuSamp.engineSpeed_rpm < 7200.0);
		CPPUNIT_ASSERT(tSamp.ecuSamp.tps > -5.0);
		CPPUNIT_ASSERT(tSamp.ecuSamp.tps < 105.0);
	}
}

void
DataSourceTest::testTelemetryMerge()
{
	const double SAMP_RATE_HZ = 10.0;
	const size_t N_SAMPS = 100;
	gpo::TelemetrySamples tSamps;
	tSamps.resize(N_SAMPS);
	for (unsigned int i=0; i<N_SAMPS; i++)
	{
		auto &samp = tSamps.at(i);
		samp.t_offset = (1.0 / SAMP_RATE_HZ) * i;
		samp.gpSamp.gps.coord.lat = i;
		samp.gpSamp.gps.coord.lon = 0;
	}

	auto srcFromTelem = gpo::DataSource::makeDataFromTelemetry(
		tSamps);
	CPPUNIT_ASSERT(srcFromTelem != nullptr);
	
	auto srcFromMsq = gpo::DataSource::loadDataFromMegaSquirtLog(
		test_data::ecu::MS2E_AUTOCROSS);
	CPPUNIT_ASSERT(srcFromMsq != nullptr);

	// merge should not do anything due to out of bounds srcStartIdx
	CPPUNIT_ASSERT_EQUAL(
		(size_t)(0),
		srcFromTelem->mergeTelemetryIn(srcFromMsq,srcFromMsq->telemSrc->size(),0,false));

	// merge should not do anything due to out of bounds dstStartIdx
	CPPUNIT_ASSERT_EQUAL(
		(size_t)(0),
		srcFromTelem->mergeTelemetryIn(srcFromMsq,0,srcFromTelem->telemSrc->size(),false));

	// merge should not do anything due to incompatible data rates
	// srcFromTelem rate = 10Hz
	// srcFromMsq rate = ~14.8Hz
	CPPUNIT_ASSERT_EQUAL(
		(size_t)(0),
		srcFromTelem->mergeTelemetryIn(srcFromMsq,0,0,false));

	// now resample MSQ data to match destination
	srcFromMsq->resampleTelemetry(SAMP_RATE_HZ);

	// get expected data available
	// Note: needs to snapshot this before the merge or else srcFromTelem->*DataAvail() will
	// already have the merged bits set.
	const auto expectedAvail = (srcFromTelem->dataAvailable() | srcFromMsq->dataAvailable());
	
	// merge without growth
	auto mergedSamps = srcFromTelem->mergeTelemetryIn(
		srcFromMsq,
		0,
		0,
		false);// no growth
	CPPUNIT_ASSERT_EQUAL((size_t)(N_SAMPS),mergedSamps);

	CPPUNIT_ASSERT((expectedAvail == srcFromTelem->dataAvailable()));
}

int main()
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(DataSourceTest::suite());
	return runner.run() ? 0 : EXIT_FAILURE;
}
