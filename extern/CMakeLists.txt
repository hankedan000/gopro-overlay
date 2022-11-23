add_subdirectory(gopro-telem)
add_subdirectory(yaml-cpp)

# check to see that tqdm was cloned successfully
find_path(TQDM_INCLUDE_DIR
	NAMES tqdm.h
	HINTS "${CMAKE_CURRENT_SOURCE_DIR}/cpptqdm")
if (NOT TQDM_INCLUDE_DIR)
	message(FATAL_ERROR "Could not find tqdm header-only library.")
else()
	add_library(tqdm INTERFACE)
	target_include_directories(tqdm INTERFACE ${TQDM_INCLUDE_DIR})
endif()
