add_executable(LineSegmentUtilsTest LineSegmentUtilsTest.cpp)
add_test(NAME LineSegmentUtilsTest COMMAND LineSegmentUtilsTest)
target_link_libraries(LineSegmentUtilsTest
	PRIVATE
		GoProOverlay
		${CPPUNIT_LIBRARIES})
