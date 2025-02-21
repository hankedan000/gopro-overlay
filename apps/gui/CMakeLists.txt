add_subdirectory(widgets)

set(CMAKE_AUTORCC ON)

add_executable(gpo
	gpo.cpp

	cmds/AlignmentPlotCmd.cpp
	cmds/Command.cpp
	cmds/ListOpenCL_DevicesCmd.cpp
	cmds/SingleOverlayCmd.cpp
	cmds/TelemetryMergeCmd.cpp
	cmds/TopBottomOverlayCmd.cpp
	cmds/TrackEditorCmd.cpp

	"${CMAKE_CURRENT_SOURCE_DIR}/res/resources.qrc"
)
target_include_directories(gpo PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gpo PUBLIC
	AppWidgets
	argparse::argparse
	GoProOverlay
	tqdm
)