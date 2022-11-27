add_executable(gopro_overlay
	gopro_overlay.cpp)

target_link_libraries(gopro_overlay
	GoProOverlay
	tqdm)

add_executable(topbottom_overlay
	topbottom_overlay.cpp)

target_link_libraries(topbottom_overlay
	GoProOverlay
	tqdm)





set(CMAKE_AUTOUIC ON)# Qt UI compiler (for *.ui files)
set(CMAKE_AUTOMOC ON)# Qt Meta Object compiler
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_subdirectory(ui)

add_executable(TrackEditorApp
	TrackEditorApp.cpp
)

target_link_libraries(TrackEditorApp PUBLIC
	GoProOverlay
	MyWidgets
)

add_executable(ProjectApp
	ProjectApp.cpp
)

target_link_libraries(ProjectApp PUBLIC
	GoProOverlay
	MyWidgets
)
