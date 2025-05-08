cmake_minimum_required (VERSION 3.1)

project(GoProOverlay VERSION 1.0.0)

# build for release if not set; otherwise, it's toolchain-dependant
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# ---------------------------------------------------------------------------------------
# Set GOPROOVERLAY_MASTER_PROJECT to ON if we are building GoProOverlay
# ---------------------------------------------------------------------------------------
# Check if GoProOverlay is being used directly or via add_subdirectory, but allow overriding
if(NOT DEFINED GOPROOVERLAY_MASTER_PROJECT)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(GOPROOVERLAY_MASTER_PROJECT ON)
    else()
        set(GOPROOVERLAY_MASTER_PROJECT OFF)
    endif()
endif()

option(GOPROOVERLAY_BUILD_ALL "Build all artifacts" OFF)

# application options
option(GOPROOVERLAY_BUILD_APPS "Build apps" ${GOPROOVERLAY_MASTER_PROJECT})

# testing options
option(GOPROOVERLAY_BUILD_TESTS "Build tests" ${GOPROOVERLAY_MASTER_PROJECT})

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # used by SonarQube in IDE

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
	option(GOPROOVERLAY_GIT_SUBMODULE "Check submodules during build" ON)
	if(GOPROOVERLAY_GIT_SUBMODULE)
		message(STATUS "${PROJECT_NAME} - Submodule update")
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

find_package(QT NAMES Qt5)
find_package(Qt${QT_VERSION_MAJOR}
    REQUIRED COMPONENTS
        Widgets
        PrintSupport# needed by QCustomPlot
)

add_subdirectory(extern)

# enable compiler warnings as errors
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")

add_subdirectory(src)

if (GOPROOVERLAY_BUILD_TESTS OR GOPROOVERLAY_BUILD_ALL)
	message(STATUS "${PROJECT_NAME} - Generating tests")
	include(CTest)
	add_subdirectory(test)
endif()