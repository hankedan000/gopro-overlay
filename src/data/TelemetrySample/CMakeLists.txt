add_library(TelemetrySample INTERFACE)

target_include_directories(TelemetrySample
	INTERFACE
		"${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(TelemetrySample
	INTERFACE
		GoProTelem::GoProTelem)