add_library(TelemetrySource STATIC
	TelemetrySource.cpp)

target_include_directories(TelemetrySource
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(TelemetrySource
	TelemetrySample
	TelemetrySeeker)