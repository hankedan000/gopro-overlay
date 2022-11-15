add_subdirectory(RenderedObject)
add_subdirectory(TrackMapObject)
add_subdirectory(FrictionCircleObject)

add_executable(gopro_overlay
	gopro_overlay.cpp)

target_link_libraries(gopro_overlay
	${OpenCV_LIBS} 
	FrictionCircleObject
	GoProTelem::GoProTelem
	TrackMapObject)