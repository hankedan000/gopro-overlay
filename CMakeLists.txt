cmake_minimum_required (VERSION 3.1)

project(GoProOverlay VERSION 1.0.0)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# add project's module directory to CMAKE_MODULE_PATH
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# build a multi-arch tuple for installing (ex. "x86_64-linux-gnu")
string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_LOWER)
string(TOLOWER ${CMAKE_CXX_COMPILER_ID} COMPILER_NAME_LOWER)
set(MULTI_ARCH_SUFFIX "${CMAKE_SYSTEM_PROCESSOR}-${SYSTEM_NAME_LOWER}-${COMPILER_NAME_LOWER}")

# update all submodules
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
	# Update submodules as needed
	option(GIT_SUBMODULE "Check submodules during build" ON)
	if(GIT_SUBMODULE)
		message(STATUS "Submodule update")
		execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
						WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
						RESULT_VARIABLE GIT_SUBMOD_RESULT)
		if(NOT GIT_SUBMOD_RESULT EQUAL "0")
			message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
		endif()
	endif()
endif()

# include Qt
set(CMAKE_AUTOUIC ON)# Qt UI compiler (for *.ui files)
set(CMAKE_AUTOMOC ON)# Qt Meta Object compiler
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR}
    REQUIRED COMPONENTS
        Widgets
        PrintSupport# needed by QCustomPlot
)

# use cppunit for testing
find_package(cppunit)
# enable testing if root project is us
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND CPPUNIT_FOUND)
	include(CTest)
endif()

add_subdirectory(extern)

# enable compiler warnings and treat them as errors for this library
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

add_subdirectory(src)
add_subdirectory(apps)
if (BUILD_TESTING)
	add_subdirectory(test)
endif ()