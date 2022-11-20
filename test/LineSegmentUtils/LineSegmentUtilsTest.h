#pragma once

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class LineSegmentUtilsTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(LineSegmentUtilsTest);
	CPPUNIT_TEST(tests);
	CPPUNIT_TEST_SUITE_END();

public:
	LineSegmentUtilsTest();
	void setUp();
	void tearDown();

protected:
	void tests();

private:

};
