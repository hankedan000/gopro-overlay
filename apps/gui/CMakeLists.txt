add_subdirectory(widgets)

set(CMAKE_AUTORCC ON)

add_executable(AlignmentPlotStandalone
	AlignmentPlotStandalone.cpp
)
target_link_libraries(AlignmentPlotStandalone PUBLIC
	GoProOverlay
	AppWidgets
)

add_executable(TelemetryMergerStandalone
	TelemetryMergerStandalone.cpp
)
target_link_libraries(TelemetryMergerStandalone PUBLIC
	AppWidgets
)

add_executable(gpo
	gpo.cpp

	cmds/TrackEditorCmd.cpp

	"${CMAKE_CURRENT_SOURCE_DIR}/res/resources.qrc"
)
target_include_directories(gpo PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gpo PUBLIC
	AppWidgets
	argparse::argparse
	GoProOverlay
)