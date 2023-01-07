set(LIBNAME "GoProOverlay")
set(LIBNAME_ALL_UPPER "GOPRO_OVERLAY")

add_library("${LIBNAME}" STATIC
	"${CMAKE_CURRENT_SOURCE_DIR}/data/DataSource.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/data/GroupedSeeker.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/data/RenderProject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/data/TelemetrySeeker.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/data/TelemetrySource.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/data/TrackDataObjects.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/data/VideoSource.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/LapTimerObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/FrictionCircleObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/LapTimerObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/RenderedObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/RenderEngine.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/SpeedometerObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/TelemetryPlot.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/TelemetryPlotObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/TelemetryPrintoutObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/TextObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/TrackMapObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/VideoObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/utils/DataProcessingUtils.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/utils/LineSegmentUtils.cpp"
	
	# MOC needs to know where this... grumble grumble
	# see https://stackoverflow.com/questions/14010922/qt-undefined-reference-to-vtable
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GoProOverlay/graphics/TelemetryPlot.h"
)

target_compile_features(${LIBNAME} PRIVATE cxx_std_17)

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
		QCustomPlot
		yaml-cpp)

# optionally use OpenMP for parallel processing
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
	target_link_libraries("${LIBNAME}" PUBLIC OpenMP::OpenMP_CXX)
endif()

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
