add_subdirectory(widgets)

set(CMAKE_AUTORCC ON)

add_executable(gpo
	gpo.cpp

	"${CMAKE_CURRENT_SOURCE_DIR}/res/resources.qrc"
)
target_include_directories(gpo PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gpo PUBLIC
	argparse::argparse
	GoProOverlay
	GoProOverlayWidgets
	tqdm
)