add_executable(gopro_overlay
	gopro_overlay.cpp)

target_link_libraries(gopro_overlay
	GoProOverlay
	tqdm)

add_executable(topbottom_overlay
	topbottom_overlay.cpp)

target_link_libraries(topbottom_overlay
	GoProOverlay
	tqdm)

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
