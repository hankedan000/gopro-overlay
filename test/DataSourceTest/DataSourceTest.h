#pragma once

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class DataSourceTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(DataSourceTest);
	CPPUNIT_TEST(testLoadFromTelemetry);
	CPPUNIT_TEST(testLoadFromMegaSquirtLog);
	CPPUNIT_TEST(testLoadFromSoloStormCSV);
	CPPUNIT_TEST(testTelemetryMerge);
	CPPUNIT_TEST_SUITE_END();

public:
	DataSourceTest();
	void setUp();
	void tearDown();

protected:
	void testLoadFromTelemetry();
	void testLoadFromMegaSquirtLog();
	void testLoadFromSoloStormCSV();
	void testTelemetryMerge();

private:

};
