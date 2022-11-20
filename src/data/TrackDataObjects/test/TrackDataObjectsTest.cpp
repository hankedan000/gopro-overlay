#include "TrackDataObjectsTest.h"

#include "TrackDataObjects.h"

TrackDataObjectsTest::TrackDataObjectsTest()
{
}

void
TrackDataObjectsTest::setUp()
{
	// run before each test case
}

void
TrackDataObjectsTest::tearDown()
{
	// run after each test case
}

void
TrackDataObjectsTest::closestPoint()
{
	/**
	 *   *(0,4)                
	 *                         
	 *       *(1,3)            
	 *                         
	 *           *(2,2)        
	 *                         
	 *              *(3,1)     
	 *                         
	 *   *(0,0)  *(2,0)        
	 */
	std::vector<cv::Vec2d> path;
	path.resize(6);
	path[0] = {0,0};
	path[1] = {2,0};
	path[2] = {3,1};
	path[3] = {2,2};
	path[4] = {1,3};
	path[5] = {0,4};

	gpo::Track track(path);
	CPPUNIT_ASSERT_EQUAL(6UL, track.pathCount());

	// -----------------------
	// test Track::findClosestPoint()

	auto findRes = track.findClosestPoint({0.0,0.0});// should find (0,0)
	CPPUNIT_ASSERT_EQUAL(true, findRes.first);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, findRes.second[0], 0.000001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, findRes.second[1], 0.000001);

	findRes = track.findClosestPoint({2.9,1.0});// should find (3,1)
	CPPUNIT_ASSERT_EQUAL(true, findRes.first);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, findRes.second[0], 0.000001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, findRes.second[1], 0.000001);

	findRes = track.findClosestPoint({50,100});// should find (0,4)
	CPPUNIT_ASSERT_EQUAL(true, findRes.first);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, findRes.second[0], 0.000001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, findRes.second[1], 0.000001);

	// -----------------------
	// test Track::findClosestPointWithIdx()

	auto findWithIdxRes = track.findClosestPointWithIdx({0.0,0.0});// should find (0,0)
	CPPUNIT_ASSERT_EQUAL(true, std::get<0>(findWithIdxRes));
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, std::get<1>(findWithIdxRes)[0], 0.000001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, std::get<1>(findWithIdxRes)[1], 0.000001);
	CPPUNIT_ASSERT_EQUAL(0UL, std::get<2>(findWithIdxRes));

	findWithIdxRes = track.findClosestPointWithIdx({2.9,1.0});// should find (3,1)
	CPPUNIT_ASSERT_EQUAL(true, std::get<0>(findWithIdxRes));
	CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, std::get<1>(findWithIdxRes)[0], 0.000001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, std::get<1>(findWithIdxRes)[1], 0.000001);
	CPPUNIT_ASSERT_EQUAL(2UL, std::get<2>(findWithIdxRes));

	findWithIdxRes = track.findClosestPointWithIdx({50,100});// should find (0,4)
	CPPUNIT_ASSERT_EQUAL(true, std::get<0>(findWithIdxRes));
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, std::get<1>(findWithIdxRes)[0], 0.000001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, std::get<1>(findWithIdxRes)[1], 0.000001);
	CPPUNIT_ASSERT_EQUAL(5UL, std::get<2>(findWithIdxRes));
}

int main()
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(TrackDataObjectsTest::suite());
	return runner.run() ? 0 : EXIT_FAILURE;
}
