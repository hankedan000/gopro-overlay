add_library(RenderedObject STATIC
	RenderedObject.cpp)

target_include_directories(RenderedObject
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(RenderedObject PUBLIC
	${OpenCV_LIBS})