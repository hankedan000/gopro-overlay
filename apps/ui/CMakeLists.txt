set(QT_WIDGETS_SOURCES
        cvimageview.h
        cvimageview.cpp
        cvimageview.ui
        progressdialog.h
        progressdialog.cpp
        progressdialog.ui
        projectwindow.h
        projectwindow.cpp
        projectwindow.ui
        renderenginewizard_topbottom.h
        renderenginewizard_topbottom.cpp
        renderenginewizard_topbottom.ui
        renderentitypropertiestab.h
        renderentitypropertiestab.cpp
        renderentitypropertiestab.ui
        renderthread.h
        renderthread.cpp
        scrubbablevideo.h
        scrubbablevideo.cpp
        scrubbablevideo.ui
	trackeditor.h
	trackeditor.cpp
	trackeditor.ui
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
