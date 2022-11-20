add_library(GoProOverlay STATIC
	"${CMAKE_CURRENT_SOURCE_DIR}/data/DataFactory.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/data/TelemetrySeeker.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/data/TelemetrySource.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/data/TrackDataObjects.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/data/VideoSource.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/FrictionCircleObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/LapTimerObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/RenderedObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/SpeedometerObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/TelemetryObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/TelemetryPrintoutObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/TextObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/TrackMapObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/VideoObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/utils/LineSegmentUtils.cpp")

# add namespaced alias so that library target is referencable similar
# to how it would when included from exports. this enabled project
# users to do add_sub_directory() on this project as submodule
add_library(GoProOverlay::GoProOverlay ALIAS GoProOverlay)

target_include_directories(GoProOverlay
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(GoProOverlay
	PUBLIC
		GoProTelem::GoProTelem
		${OpenCV_LIBS})
