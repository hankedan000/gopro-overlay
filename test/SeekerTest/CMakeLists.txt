add_executable(SeekerTest SeekerTest.cpp)
add_test(NAME SeekerTest COMMAND SeekerTest)
target_link_libraries(
	SeekerTest
		${CPPUNIT_LIBRARIES}
		GoProOverlay)
