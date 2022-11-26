#pragma once

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class TrackDataObjectsTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(TrackDataObjectsTest);
	CPPUNIT_TEST(closestPoint);
	CPPUNIT_TEST(detectionGate);
	CPPUNIT_TEST(sortedPathObjects);
	CPPUNIT_TEST_SUITE_END();

public:
	TrackDataObjectsTest();
	void setUp();
	void tearDown();

protected:
	void closestPoint();
	void detectionGate();
	void sortedPathObjects();

private:

};
