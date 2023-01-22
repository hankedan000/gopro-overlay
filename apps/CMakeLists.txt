add_executable(single_overlay
	single_overlay.cpp)

target_link_libraries(single_overlay
	GoProOverlay
	tqdm)

add_executable(topbottom_overlay
	topbottom_overlay.cpp)

target_link_libraries(topbottom_overlay
	GoProOverlay
	tqdm)

add_executable(msq-parse
	msq-parse.cpp)

target_link_libraries(msq-parse
	csv
	GoProOverlay
	spdlog::spdlog)

# Qt Apps
add_subdirectory(ui)

add_executable(TrackEditorApp
	TrackEditorApp.cpp
)

target_link_libraries(TrackEditorApp PUBLIC
	GoProOverlay
	AppWidgets
)

add_executable(ProjectApp
	ProjectApp.cpp
)

target_link_libraries(ProjectApp PUBLIC
	GoProOverlay
	AppWidgets
)
