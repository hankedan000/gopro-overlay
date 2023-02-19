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
	CPPUNIT_ASSERT_EQUAL(false, bitset_is_any_set(srcFromTelem->dataAvailable()));

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
	bitset_clear(expectedAvail);
	bitset_set_bit(expectedAvail, gpo::eDA_ECU_ENGINE_SPEED);
	bitset_set_bit(expectedAvail, gpo::eDA_ECU_TPS);
	bitset_set_bit(expectedAvail, gpo::eDA_ECU_BOOST);
	CPPUNIT_ASSERT(bitset_equal(srcFromMsq->dataAvailable(), expectedAvail));

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
	const auto expectedAvail = bitset_or(srcFromTelem->dataAvailable(), srcFromMsq->dataAvailable());
	
	// merge without growth
	auto mergedSamps = srcFromTelem->mergeTelemetryIn(
		srcFromMsq,
		0,
		0,
		false);// no growth
	CPPUNIT_ASSERT_EQUAL((size_t)(N_SAMPS),mergedSamps);

	CPPUNIT_ASSERT(bitset_equal(expectedAvail, srcFromTelem->dataAvailable()));
}

int main()
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(DataSourceTest::suite());
	return runner.run() ? 0 : EXIT_FAILURE;
}
