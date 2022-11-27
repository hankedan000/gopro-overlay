set(LIBNAME "GoProOverlay")
set(LIBNAME_ALL_UPPER "GOPRO_OVERLAY")

add_library("${LIBNAME}" STATIC
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
	"${CMAKE_CURRENT_SOURCE_DIR}/utils/DataProcessingUtils.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/utils/LineSegmentUtils.cpp")

# add namespaced alias so that library target is referencable similar
# to how it would when included from exports. this enabled project
# users to do add_sub_directory() on this project as submodule
add_library(GoProOverlay::GoProOverlay ALIAS "${LIBNAME}")

target_include_directories("${LIBNAME}"
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries("${LIBNAME}"
	PUBLIC
		GoProTelem::GoProTelem
		${OpenCV_LIBS}
		yaml-cpp)

# build a list of public header files to install
list(APPEND ${LIBNAME_ALL_UPPER}_PUBLIC_HEADERS
)
set_target_properties("${LIBNAME}" PROPERTIES
	PUBLIC_HEADER "${${LIBNAME_ALL_UPPER}_PUBLIC_HEADERS}"
)
install(
	TARGETS "${LIBNAME}"
	EXPORT "${LIBNAME}Targets"
	ARCHIVE
		DESTINATION "lib/${MULTI_ARCH_SUFFIX}/${LIBNAME}"
	LIBRARY
		DESTINATION "lib/${MULTI_ARCH_SUFFIX}/${LIBNAME}"
	PUBLIC_HEADER
		DESTINATION "include/${LIBNAME}"
)
