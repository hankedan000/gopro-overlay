add_subdirectory(utils)
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
	SpeedometerObject
	TelemetryPrintoutObject
	TrackMapObject
	tqdm
	VideoObject)

add_executable(topbottom_overlay
	topbottom_overlay.cpp)

target_link_libraries(topbottom_overlay
	${OpenCV_LIBS} 
	DataFactory
	FrictionCircleObject
	GoProTelem::GoProTelem
	LapTimerObject
	SpeedometerObject
	TelemetryPrintoutObject
	TextObject
	TrackMapObject
	tqdm
	VideoObject)