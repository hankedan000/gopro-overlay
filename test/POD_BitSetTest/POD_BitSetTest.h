#pragma once

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class POD_BitSetTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(POD_BitSetTest);
	CPPUNIT_TEST(checkPOD);
	CPPUNIT_TEST(setAndGet);
	CPPUNIT_TEST_SUITE_END();

public:
	POD_BitSetTest();
	void setUp();
	void tearDown();

protected:
	void checkPOD();
	void setAndGet();

private:

};
