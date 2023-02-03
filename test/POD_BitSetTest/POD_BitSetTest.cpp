#include "POD_BitSetTest.h"

#include "GoProOverlay/data/pod_bitset.hpp"

POD_BitSetTest::POD_BitSetTest()
{
}

void
POD_BitSetTest::setUp()
{
	// run before each test case
}

void
POD_BitSetTest::tearDown()
{
	// run after each test case
}

void
POD_BitSetTest::checkPOD()
{
	pod_bitset<uint64_t,2> bitset;
	CPPUNIT_ASSERT_EQUAL(sizeof(uint64_t) * 2, sizeof(bitset));
}

void
POD_BitSetTest::setAndGet()
{
	pod_bitset<uint64_t,2> bitset;

	bitset_clear(bitset);
	CPPUNIT_ASSERT_EQUAL((uint64_t)(0x0),bitset.bit_units[0]);
	CPPUNIT_ASSERT_EQUAL((uint64_t)(0x0),bitset.bit_units[1]);

	bitset_set_bit(bitset,3);
	CPPUNIT_ASSERT_EQUAL((uint64_t)(0x8),bitset.bit_units[0]);
	CPPUNIT_ASSERT_EQUAL((uint64_t)(0x0),bitset.bit_units[1]);

	bitset_set_bit(bitset,75);
	CPPUNIT_ASSERT_EQUAL((uint64_t)(0x8),bitset.bit_units[0]);
	CPPUNIT_ASSERT_EQUAL((uint64_t)(0x800),bitset.bit_units[1]);
}

int main()
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(POD_BitSetTest::suite());
	return runner.run() ? 0 : EXIT_FAILURE;
}
