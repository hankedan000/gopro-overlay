#include "TrackDataObjectsTest.h"

#include "GoProOverlay/data/TrackDataObjects.h"

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

void
TrackDataObjectsTest::detectionGate()
{
	// horizontal path
	{
		/**
		 *   *(0,0)  *(2,0)  *(4,0)
		 */
		std::vector<cv::Vec2d> path;
		path.resize(3);
		path[0] = {0,0};
		path[1] = {2,0};
		path[2] = {4,0};

		gpo::Track track(path);
		CPPUNIT_ASSERT_EQUAL(3UL, track.pathCount());

		// -----------------------
		// test Track::getDetectionGate()

		const double WIDTH_METERS = 1.0;
		const double HALF_WIDTH_DECDEG = gpo::m2dd(WIDTH_METERS/2.0);
		auto gate = track.getDetectionGate(1,WIDTH_METERS);// around point (2,0)
		CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, gate.a()[0], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, gate.b()[0], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(+HALF_WIDTH_DECDEG, gate.a()[1], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(-HALF_WIDTH_DECDEG, gate.b()[1], 0.000001);

		// test case at beginning of path
		gate = track.getDetectionGate(0,WIDTH_METERS);// around point (0,0)
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, gate.a()[0], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, gate.b()[0], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(+HALF_WIDTH_DECDEG, gate.a()[1], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(-HALF_WIDTH_DECDEG, gate.b()[1], 0.000001);

		// test case at end of path
		gate = track.getDetectionGate(2,WIDTH_METERS);// around point (4,0)
		CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, gate.a()[0], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, gate.b()[0], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(+HALF_WIDTH_DECDEG, gate.a()[1], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(-HALF_WIDTH_DECDEG, gate.b()[1], 0.000001);
	}

	// vertical path
	{
		/**
		 *   *(0,4)
		 * 
		 *   *(0,2)
		 * 
		 *   *(0,0)
		 */
		std::vector<cv::Vec2d> path;
		path.resize(3);
		path[0] = {0,0};
		path[1] = {0,2};
		path[2] = {0,4};

		gpo::Track track(path);
		CPPUNIT_ASSERT_EQUAL(3UL, track.pathCount());

		// -----------------------
		// test Track::getDetectionGate()

		const double WIDTH_METERS = 1.0;
		const double HALF_WIDTH_DECDEG = gpo::m2dd(WIDTH_METERS/2.0);
		auto gate = track.getDetectionGate(1,WIDTH_METERS);// around point (0,2)
		CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, gate.a()[1], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, gate.b()[1], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(+HALF_WIDTH_DECDEG, gate.a()[0], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(-HALF_WIDTH_DECDEG, gate.b()[0], 0.000001);

		// test case at beginning of path
		gate = track.getDetectionGate(0,WIDTH_METERS);// around point (0,0)
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, gate.a()[1], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, gate.b()[1], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(+HALF_WIDTH_DECDEG, gate.a()[0], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(-HALF_WIDTH_DECDEG, gate.b()[0], 0.000001);

		// test case at end of path
		gate = track.getDetectionGate(2,WIDTH_METERS);// around point (0,4)
		CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, gate.a()[1], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, gate.b()[1], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(+HALF_WIDTH_DECDEG, gate.a()[0], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(-HALF_WIDTH_DECDEG, gate.b()[0], 0.000001);
	}

	// diagonal path
	{
		/**
		 *               *(5,4)
		 * 
		 *         *(3,2)
		 * 
		 *   *(0,0)
		 */
		std::vector<cv::Vec2d> path;
		path.resize(3);
		path[0] = {0,0};
		path[1] = {3,2};
		path[2] = {5,4};

		gpo::Track track(path);
		CPPUNIT_ASSERT_EQUAL(3UL, track.pathCount());

		// -----------------------
		// test Track::getDetectionGate()

		const double WIDTH_METERS = 1.0;
		auto gate = track.getDetectionGate(1,WIDTH_METERS);// around point (0,2)
		CPPUNIT_ASSERT_DOUBLES_EQUAL(2.000003, gate.a()[1], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(1.999997, gate.b()[1], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(2.999997, gate.a()[0], 0.000001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(3.000002, gate.b()[0], 0.000001);

		double pathSlope = (path[2][1]-path[0][1])/(path[2][0]-path[0][0]);
		double expectGateSlope = -1.0/pathSlope;
		double actualGateSlope = (gate.a()[1]-gate.b()[1])/(gate.a()[0]-gate.b()[0]);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(expectGateSlope,actualGateSlope,0.000001);
	}
}

void
TrackDataObjectsTest::sortedPathObjects()
{
	const size_t PATH_LENGTH = 100;
	std::vector<cv::Vec2d> path;
	for (unsigned int i=0; i<PATH_LENGTH; i++)
	{
		path.push_back(cv::Vec2d(i, i*2));
	}

	gpo::Track track(path);
	track.setStart(5);
	track.setFinish(95);
	track.addSector("Sector1",10,20);
	track.addSector("Sector3",60,70);
	track.addSector("Sector2",30,50);

	// make sure objects are sorted based on entry path index
	std::vector<const gpo::TrackPathObject *> pathObjs;
	CPPUNIT_ASSERT_EQUAL(true, track.getSortedPathObjects(pathObjs));
	CPPUNIT_ASSERT_EQUAL(5UL, pathObjs.size());

	CPPUNIT_ASSERT_EQUAL(std::string("startGate"), pathObjs.at(0)->getName());
	CPPUNIT_ASSERT_EQUAL(true, pathObjs.at(0)->isGate());
	CPPUNIT_ASSERT_EQUAL(false, pathObjs.at(0)->isSector());
	CPPUNIT_ASSERT_EQUAL(gpo::GateType_E::eGT_Start, pathObjs.at(0)->getGateType());

	CPPUNIT_ASSERT_EQUAL(std::string("Sector1"), pathObjs.at(1)->getName());
	CPPUNIT_ASSERT_EQUAL(false, pathObjs.at(1)->isGate());
	CPPUNIT_ASSERT_EQUAL(true, pathObjs.at(1)->isSector());
	CPPUNIT_ASSERT_EQUAL(gpo::GateType_E::eGT_NOT_A_GATE, pathObjs.at(1)->getGateType());

	CPPUNIT_ASSERT_EQUAL(std::string("Sector2"), pathObjs.at(2)->getName());
	CPPUNIT_ASSERT_EQUAL(false, pathObjs.at(2)->isGate());
	CPPUNIT_ASSERT_EQUAL(true, pathObjs.at(2)->isSector());
	CPPUNIT_ASSERT_EQUAL(gpo::GateType_E::eGT_NOT_A_GATE, pathObjs.at(2)->getGateType());

	CPPUNIT_ASSERT_EQUAL(std::string("Sector3"), pathObjs.at(3)->getName());
	CPPUNIT_ASSERT_EQUAL(false, pathObjs.at(3)->isGate());
	CPPUNIT_ASSERT_EQUAL(true, pathObjs.at(3)->isSector());
	CPPUNIT_ASSERT_EQUAL(gpo::GateType_E::eGT_NOT_A_GATE, pathObjs.at(3)->getGateType());

	CPPUNIT_ASSERT_EQUAL(std::string("finishGate"), pathObjs.at(4)->getName());
	CPPUNIT_ASSERT_EQUAL(true, pathObjs.at(4)->isGate());
	CPPUNIT_ASSERT_EQUAL(false, pathObjs.at(4)->isSector());
	CPPUNIT_ASSERT_EQUAL(gpo::GateType_E::eGT_Finish, pathObjs.at(4)->getGateType());
}

int main()
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(TrackDataObjectsTest::suite());
	return runner.run() ? 0 : EXIT_FAILURE;
}
