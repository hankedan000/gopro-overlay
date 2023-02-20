add_executable(POD_BitSetTest POD_BitSetTest.cpp)
add_test(NAME POD_BitSetTest COMMAND POD_BitSetTest)
target_link_libraries(POD_BitSetTest
	PRIVATE
		${CPPUNIT_LIBRARIES}
		GoProOverlay)
