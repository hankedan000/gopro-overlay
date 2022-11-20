add_executable(TrackDataObjectsTest TrackDataObjectsTest.cpp)
add_test(NAME TrackDataObjectsTest COMMAND TrackDataObjectsTest)
target_link_libraries(
	TrackDataObjectsTest
		${CPPUNIT_LIBRARIES}
		GoProOverlay)
