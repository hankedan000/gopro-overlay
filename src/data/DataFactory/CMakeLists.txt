add_library(DataFactory STATIC
	DataFactory.cpp)

target_include_directories(DataFactory
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(DataFactory
	PUBLIC
		TelemetrySeeker
		TelemetrySource
		VideoSource)