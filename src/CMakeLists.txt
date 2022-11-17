add_subdirectory(data)
add_subdirectory(graphics)

add_executable(gopro_overlay
	gopro_overlay.cpp)

target_link_libraries(gopro_overlay
	${OpenCV_LIBS} 
	DataFactory
	FrictionCircleObject
	GoProTelem::GoProTelem
	LapTimerObject
	TrackMapObject
	tqdm)