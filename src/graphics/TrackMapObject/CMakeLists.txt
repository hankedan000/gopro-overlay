add_library(TrackMapObject STATIC
	TrackMapObject.cpp)

target_include_directories(TrackMapObject
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(TrackMapObject
	TelemetryObject)