add_library(TelemetryObject STATIC
	TelemetryObject.cpp)

target_include_directories(TelemetryObject
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(TelemetryObject
	TelemetrySource
	RenderedObject)