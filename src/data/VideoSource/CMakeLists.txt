add_library(VideoSource STATIC
	VideoSource.cpp)

target_include_directories(VideoSource
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(VideoSource
	PUBLIC
		${OpenCV_LIBS}
		TelemetrySample
		TelemetrySeeker)