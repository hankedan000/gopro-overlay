add_executable(DataSourceTest DataSourceTest.cpp)
add_test(NAME DataSourceTest COMMAND DataSourceTest)
target_link_libraries(DataSourceTest
    PRIVATE
		${CPPUNIT_LIBRARIES}
		GoProOverlay
		test_data)
