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
	ScrubbableVideo.h
	ScrubbableVideo.cpp
	ScrubbableVideo.ui
	TelemetryMerger.h
	TelemetryMerger.cpp
	TelemetryMerger.ui
	TelemetryPlotDialog.h
	TelemetryPlotDialog.cpp
	TelemetryPlotDialog.ui
	trackeditor.h
	trackeditor.cpp
	trackeditor.ui
	trackview.h
	trackview.cpp
	trackview.ui

	utils/QModifiableObjectObserver.h
	utils/QModifiableObjectObserver.cpp
)

add_library(GoProOverlayWidgets STATIC
	${QT_WIDGETS_SOURCES}
)

target_include_directories(GoProOverlayWidgets PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(GoProOverlayWidgets PUBLIC
	Qt${QT_VERSION_MAJOR}::Widgets
	GoProOverlay
	QCustomPlot
	spdlog::spdlog
	ConcrtCpp::concrt
)
