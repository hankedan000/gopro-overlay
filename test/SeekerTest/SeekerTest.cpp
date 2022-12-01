#include "SeekerTest.h"

#include "GoProOverlay/data/TelemetrySeeker.h"

SeekerTest::SeekerTest()
{
}

void
SeekerTest::setUp()
{
	// run before each test case
}

void
SeekerTest::tearDown()
{
	// run after each test case
}

void
SeekerTest::lapIndexLookup()
{
	const size_t PATH_LENGTH = 10;
	auto tSamps = gpo::TelemetrySamplesPtr(new gpo::TelemetrySamples());
	tSamps->resize(PATH_LENGTH);
	for (unsigned int i=0; i<PATH_LENGTH; i++)
	{
		auto &samp = tSamps->at(i);
		samp.gpSamp.t_offset = 0.010 * i;
		samp.gpSamp.gps.coord.lat = i;
		samp.gpSamp.gps.coord.lon = 0;
	}

	// ----------------------------------------------
	// an "autocross" style data set where there's 1 lap and junk at begin/end

	tSamps->at(0).lap = -1;
	tSamps->at(1).lap = 1;// start gate
	tSamps->at(2).lap = 1;
	tSamps->at(3).lap = 1;
	tSamps->at(4).lap = 1;
	tSamps->at(5).lap = 1;
	tSamps->at(6).lap = 1;
	tSamps->at(7).lap = 1;// finish gate
	tSamps->at(8).lap = -1;
	tSamps->at(9).lap = -1;

	gpo::TelemetrySeeker seeker(tSamps);
	seeker.analyze();

	CPPUNIT_ASSERT_EQUAL(1U, seeker.lapCount());

	// lap 0 should not be found
	CPPUNIT_ASSERT_THROW(seeker.getLapEntryExit(0), std::out_of_range);

	// check lap 1
	std::pair<size_t,size_t> entryExit;
	CPPUNIT_ASSERT_NO_THROW(entryExit = seeker.getLapEntryExit(1));
	CPPUNIT_ASSERT_EQUAL(1UL, entryExit.first);
	CPPUNIT_ASSERT_EQUAL(7UL, entryExit.second);

	// ----------------------------------------------
	// a "circuit" track scenario where we enter somewhere random then do a much
	// of back-to-back start/finish gate entries

	tSamps->at(0).lap = -1;
	tSamps->at(1).lap = 1;
	tSamps->at(2).lap = 1;
	tSamps->at(3).lap = 1;
	tSamps->at(4).lap = 1;
	tSamps->at(5).lap = 2;
	tSamps->at(6).lap = 2;
	tSamps->at(7).lap = 2;
	tSamps->at(8).lap = 2;
	tSamps->at(9).lap = 3;// could be a corner case where we never saw an exit gate (pitted in early)

	seeker.analyze();

	CPPUNIT_ASSERT_EQUAL(3U, seeker.lapCount());

	// lap 0 should not be found
	CPPUNIT_ASSERT_THROW(seeker.getLapEntryExit(0), std::out_of_range);

	// check lap 1
	CPPUNIT_ASSERT_NO_THROW(entryExit = seeker.getLapEntryExit(1));
	CPPUNIT_ASSERT_EQUAL(1UL, entryExit.first);
	CPPUNIT_ASSERT_EQUAL(4UL, entryExit.second);

	// check lap 2
	CPPUNIT_ASSERT_NO_THROW(entryExit = seeker.getLapEntryExit(2));
	CPPUNIT_ASSERT_EQUAL(5UL, entryExit.first);
	CPPUNIT_ASSERT_EQUAL(8UL, entryExit.second);

	// check lap 3
	CPPUNIT_ASSERT_NO_THROW(entryExit = seeker.getLapEntryExit(3));
	CPPUNIT_ASSERT_EQUAL(9UL, entryExit.first);
	CPPUNIT_ASSERT_EQUAL(9UL, entryExit.second);

	// ----------------------------------------------
	// never enter any laps

	tSamps->at(0).lap = -1;
	tSamps->at(1).lap = -1;
	tSamps->at(2).lap = -1;
	tSamps->at(3).lap = -1;
	tSamps->at(4).lap = -1;
	tSamps->at(5).lap = -1;
	tSamps->at(6).lap = -1;
	tSamps->at(7).lap = -1;
	tSamps->at(8).lap = -1;
	tSamps->at(9).lap = -1;

	seeker.analyze();

	CPPUNIT_ASSERT_EQUAL(0U, seeker.lapCount());

	// ----------------------------------------------
	// some crazy thing that probably doesn't have any physical meaning

	tSamps->at(0).lap = 1;// only case that hasn't started with -1 so far!
	tSamps->at(1).lap = 1;
	tSamps->at(2).lap = -1;
	tSamps->at(3).lap = -1;
	tSamps->at(4).lap = 2;
	tSamps->at(5).lap = 2;
	tSamps->at(6).lap = -1;
	tSamps->at(7).lap = 3;
	tSamps->at(8).lap = 3;
	tSamps->at(9).lap = 3;

	seeker.analyze();

	CPPUNIT_ASSERT_EQUAL(3U, seeker.lapCount());

	// check lap 1
	CPPUNIT_ASSERT_NO_THROW(entryExit = seeker.getLapEntryExit(1));
	CPPUNIT_ASSERT_EQUAL(0UL, entryExit.first);
	CPPUNIT_ASSERT_EQUAL(1UL, entryExit.second);

	// check lap 2
	CPPUNIT_ASSERT_NO_THROW(entryExit = seeker.getLapEntryExit(2));
	CPPUNIT_ASSERT_EQUAL(4UL, entryExit.first);
	CPPUNIT_ASSERT_EQUAL(5UL, entryExit.second);

	// check lap 3
	CPPUNIT_ASSERT_NO_THROW(entryExit = seeker.getLapEntryExit(3));
	CPPUNIT_ASSERT_EQUAL(7UL, entryExit.first);
	CPPUNIT_ASSERT_EQUAL(9UL, entryExit.second);
}

int main()
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(SeekerTest::suite());
	return runner.run() ? 0 : EXIT_FAILURE;
}