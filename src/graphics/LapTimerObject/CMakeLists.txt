add_library(LapTimerObject STATIC
	LapTimerObject.cpp)

target_include_directories(LapTimerObject
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(LapTimerObject
	${OpenCV_LIBS}
	GoProTelem::GoProTelem
	RenderedObject)