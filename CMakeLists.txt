cmake_minimum_required (VERSION 3.1)

project(GoProOverlay VERSION 1.0.0)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# add project's module directory to CMAKE_MODULE_PATH
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

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

# use cppunit for testing
find_package(cppunit)
# enable testing if root project is us
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND CPPUNIT_FOUND)
	include(CTest)
	set(BUILD_TESTING "ON")
endif()

add_subdirectory(extern)
add_subdirectory(src)