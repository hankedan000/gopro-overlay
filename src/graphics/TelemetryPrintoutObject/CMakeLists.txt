add_library(TelemetryPrintoutObject STATIC
	TelemetryPrintoutObject.cpp)

target_include_directories(TelemetryPrintoutObject
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(TelemetryPrintoutObject
	TelemetryObject)