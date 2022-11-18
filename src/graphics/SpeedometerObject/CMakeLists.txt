add_library(SpeedometerObject STATIC
	SpeedometerObject.cpp)

target_include_directories(SpeedometerObject
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(SpeedometerObject
	TelemetryObject)