add_library(TextObject STATIC
	TextObject.cpp)

target_include_directories(TextObject
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(TextObject
	RenderedObject)