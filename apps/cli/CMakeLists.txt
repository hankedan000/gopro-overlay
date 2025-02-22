add_executable(topbottom_overlay
	topbottom_overlay.cpp)

target_link_libraries(topbottom_overlay
	GoProOverlay
	tqdm)

add_executable(opencl_test
	opencl_test.cpp)

target_link_libraries(opencl_test
	${OpenCV_LIBS})
