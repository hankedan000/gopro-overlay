add_executable(DataProcessingUtilsTest DataProcessingUtilsTest.cpp)
add_test(NAME DataProcessingUtilsTest COMMAND DataProcessingUtilsTest)
target_link_libraries(
	DataProcessingUtilsTest
		${CPPUNIT_LIBRARIES}
		GoProOverlay)
