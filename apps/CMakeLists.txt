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