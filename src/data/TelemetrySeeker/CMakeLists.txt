add_library(TelemetrySeeker STATIC
	TelemetrySeeker.cpp)

target_include_directories(TelemetrySeeker
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(TelemetrySeeker
	TelemetrySample)