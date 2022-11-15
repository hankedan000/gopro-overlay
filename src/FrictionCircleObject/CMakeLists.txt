add_library(FrictionCircleObject STATIC
	FrictionCircleObject.cpp)

target_include_directories(FrictionCircleObject
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(FrictionCircleObject
	${OpenCV_LIBS}
	GoProTelem::GoProTelem
	RenderedObject)