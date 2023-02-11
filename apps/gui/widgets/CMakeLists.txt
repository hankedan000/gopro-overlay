set(QT_WIDGETS_SOURCES
        AlignmentPlot.h
        AlignmentPlot.cpp
        AlignmentPlot.ui
	ColorPicker.h
	ColorPicker.cpp
	ColorPicker.ui
	cvimageview.h
	cvimageview.cpp
	cvimageview.ui
	progressdialog.h
	progressdialog.cpp
	progressdialog.ui
	ProgressTimer.h
	projectwindow.h
	projectwindow.cpp
	projectwindow.ui
	RenderEngineWizardSingleVideo.h
	RenderEngineWizardSingleVideo.cpp
	RenderEngineWizardSingleVideo.ui
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
	TelemetryPlotDialog.h
	TelemetryPlotDialog.cpp
	TelemetryPlotDialog.ui
	trackeditor.h
	trackeditor.cpp
	trackeditor.ui
	trackview.h
	trackview.cpp
	trackview.ui

	# MOC needs to know where this... grumble grumble
	# see https://stackoverflow.com/questions/14010922/qt-undefined-reference-to-vtable
	${CMAKE_CURRENT_SOURCE_DIR}/../../../src/include/GoProOverlay/graphics/TelemetryPlot.h
)

add_library(AppWidgets STATIC
	${QT_WIDGETS_SOURCES}
)

target_include_directories(AppWidgets PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(AppWidgets PUBLIC
	Qt${QT_VERSION_MAJOR}::Widgets
	GoProOverlay
	QCustomPlot
	spdlog::spdlog
)
