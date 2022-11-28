#pragma once

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class DataProcessingUtilsTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(DataProcessingUtilsTest);
	CPPUNIT_TEST(trackTimes);
	CPPUNIT_TEST_SUITE_END();

public:
	DataProcessingUtilsTest();
	void setUp();
	void tearDown();

protected:
	void trackTimes();

private:

};