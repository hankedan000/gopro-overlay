#include "LineSegmentUtilsTest.h"

#include "LineSegmentUtils.h"

LineSegmentUtilsTest::LineSegmentUtilsTest()
{
}

void
LineSegmentUtilsTest::setUp()
{
	// run before each test case
}

void
LineSegmentUtilsTest::tearDown()
{
	// run after each test case
}

void
LineSegmentUtilsTest::tests()
{
	{
		/**
		 *            pA1(2,4)
		 *               *
		 *               |
		 *  pB1(0,5)*----|----*pB2(4,5)
		 *               |
		 *               |
		 *               *
		 *            pA2(2,6)
		 */
		cv::Vec2d pA1(2,4);
		cv::Vec2d pA2(2,6);
		cv::Vec2d pB1(0,5);
		cv::Vec2d pB2(4,5);

		CPPUNIT_ASSERT_EQUAL(true,utils::doIntersect(pA1,pA2,pB1,pB2));
		CPPUNIT_ASSERT_EQUAL(true,utils::doIntersect(pA2,pA1,pB1,pB2));// flip A's points
		CPPUNIT_ASSERT_EQUAL(true,utils::doIntersect(pA1,pA2,pB2,pB1));// flip B's points
		CPPUNIT_ASSERT_EQUAL(true,utils::doIntersect(pA2,pA1,pB2,pB1));// flip A's & B's points
	}

	{
		/**
		 *            pA1(2,4)
		 *               *
		 *               |
		 *           pB1(3,5)*--------*pB2(5,5)
		 *               |
		 *               |
		 *               *
		 *            pA2(2,6)
		 */
		cv::Vec2d pA1(2,4);
		cv::Vec2d pA2(2,6);
		cv::Vec2d pB1(3,5);
		cv::Vec2d pB2(5,5);

		CPPUNIT_ASSERT_EQUAL(false,utils::doIntersect(pA1,pA2,pB1,pB2));
		CPPUNIT_ASSERT_EQUAL(false,utils::doIntersect(pA2,pA1,pB1,pB2));// flip A's points
		CPPUNIT_ASSERT_EQUAL(false,utils::doIntersect(pA1,pA2,pB2,pB1));// flip B's points
		CPPUNIT_ASSERT_EQUAL(false,utils::doIntersect(pA2,pA1,pB2,pB1));// flip A's & B's points
	}

	{
		/**
		 *            pA1(2,4)
		 *               *
		 *               |
		 *       pB1(2,5)*--------*pB2(5,5)
		 *               |
		 *               |
		 *               *
		 *            pA2(2,6)
		 */
		cv::Vec2d pA1(2,4);
		cv::Vec2d pA2(2,6);
		cv::Vec2d pB1(2,5);
		cv::Vec2d pB2(5,5);

		CPPUNIT_ASSERT_EQUAL(true,utils::doIntersect(pA1,pA2,pB1,pB2));
		CPPUNIT_ASSERT_EQUAL(true,utils::doIntersect(pA2,pA1,pB1,pB2));// flip A's points
		CPPUNIT_ASSERT_EQUAL(true,utils::doIntersect(pA1,pA2,pB2,pB1));// flip B's points
		CPPUNIT_ASSERT_EQUAL(true,utils::doIntersect(pA2,pA1,pB2,pB1));// flip A's & B's points
	}

	{
		/**
		 *            pA1(2,4)
		 *               *  *pB2(4,4)
		                 | /
		 *               |/
		 *               +
		 *              /|
		 *             / |
		 *    pB1(0,5)*  |
		 *               *
		 *            pA2(2,6)
		 */
		cv::Vec2d pA1(2,4);
		cv::Vec2d pA2(2,6);
		cv::Vec2d pB1(0,5);
		cv::Vec2d pB2(4,4);

		CPPUNIT_ASSERT_EQUAL(true,utils::doIntersect(pA1,pA2,pB1,pB2));
		CPPUNIT_ASSERT_EQUAL(true,utils::doIntersect(pA2,pA1,pB1,pB2));// flip A's points
		CPPUNIT_ASSERT_EQUAL(true,utils::doIntersect(pA1,pA2,pB2,pB1));// flip B's points
		CPPUNIT_ASSERT_EQUAL(true,utils::doIntersect(pA2,pA1,pB2,pB1));// flip A's & B's points
	}
}

int main()
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(LineSegmentUtilsTest::suite());
	return runner.run() ? 0 : EXIT_FAILURE;
}
