add_library(TrackDataObjects STATIC
	TrackDataObjects.cpp)

target_include_directories(TrackDataObjects
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(TrackDataObjects PUBLIC
	${OpenCV_LIBS}
	LineSegmentUtils
	TelemetrySource)