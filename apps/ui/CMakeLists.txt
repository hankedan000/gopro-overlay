set(QT_WIDGETS_SOURCES
	mainwindow.h
	mainwindow.cpp
	mainwindow.ui
	trackview.h
	trackview.cpp
	trackview.ui
)

add_library(MyWidgets STATIC
	${QT_WIDGETS_SOURCES}
)

target_include_directories(MyWidgets PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(MyWidgets PUBLIC
	Qt${QT_VERSION_MAJOR}::Widgets
	GoProOverlay
)