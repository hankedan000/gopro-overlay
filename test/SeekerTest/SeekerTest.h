#pragma once

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class SeekerTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(SeekerTest);
	CPPUNIT_TEST(lapIndexLookup);
	CPPUNIT_TEST(seekRelativeTime);
	CPPUNIT_TEST_SUITE_END();

public:
	SeekerTest();
	void setUp();
	void tearDown();

protected:
	void lapIndexLookup();
	void seekRelativeTime();

private:

};
