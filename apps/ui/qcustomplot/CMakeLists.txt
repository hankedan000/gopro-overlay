set(QCUSTOMPLOT_SOURCES
		qcustomplot.cpp
		qcustomplot.h
)

add_library(QCustomPlot STATIC
	${QCUSTOMPLOT_SOURCES}
)

target_include_directories(QCustomPlot PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(QCustomPlot PUBLIC
	Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::PrintSupport
)
