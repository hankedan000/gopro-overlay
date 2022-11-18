add_library(LineSegmentUtils STATIC
	LineSegmentUtils.cpp)

target_include_directories(LineSegmentUtils
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(LineSegmentUtils PUBLIC
	${OpenCV_LIBS})