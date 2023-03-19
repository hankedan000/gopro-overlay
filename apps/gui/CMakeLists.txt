add_subdirectory(widgets)

set(CMAKE_AUTORCC ON)

add_executable(TrackEditorApp
	TrackEditorApp.cpp
)
target_link_libraries(TrackEditorApp PUBLIC
	GoProOverlay
	AppWidgets
)

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

add_executable(ProjectApp
	ProjectApp.cpp
	"${CMAKE_CURRENT_SOURCE_DIR}/res/resources.qrc"
)
target_link_libraries(ProjectApp PUBLIC
	GoProOverlay
	AppWidgets
)