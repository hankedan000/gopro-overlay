add_executable(gopro_overlay
	gopro_overlay.cpp)

target_link_libraries(gopro_overlay
	${OpenCV_LIBS} 
	GoProTelem::GoProTelem)