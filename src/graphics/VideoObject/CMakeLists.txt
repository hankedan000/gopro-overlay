add_library(VideoObject STATIC
	VideoObject.cpp)

target_include_directories(VideoObject
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(VideoObject
	RenderedObject
	VideoSource)