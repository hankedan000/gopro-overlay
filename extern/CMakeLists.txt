add_subdirectory(gopro-telem)
add_subdirectory(yaml-cpp)
add_subdirectory(qcustomplot)
add_subdirectory(spdlog)
add_subdirectory(concrt-cpp)

set(TRACY_ENABLE          OFF CACHE BOOL "" FORCE)
set(TRACY_DELAYED_INIT    ON  CACHE BOOL "" FORCE)
set(TRACY_MANUAL_LIFETIME ON  CACHE BOOL "" FORCE)
set(TRACY_ON_DEMAND       ON  CACHE BOOL "" FORCE)
add_subdirectory(tracy)

# create header-only target for tqdm (if it was cloned correctly)
find_path(TQDM_INCLUDE_DIR
	NAMES tqdm.h
	HINTS "${CMAKE_CURRENT_SOURCE_DIR}/cpptqdm")
if (NOT TQDM_INCLUDE_DIR)
	message(FATAL_ERROR "Could not find tqdm header-only library.")
else()
	add_library(tqdm INTERFACE)
	target_include_directories(tqdm INTERFACE ${TQDM_INCLUDE_DIR})
endif()

# create header-only target for csv (if it was cloned correctly)
find_path(CSV_INCLUDE_DIR
	NAMES csv.hpp
	HINTS "${CMAKE_CURRENT_SOURCE_DIR}/csv-parser/single_include")
if (NOT CSV_INCLUDE_DIR)
	message(FATAL_ERROR "Could not find csv header-only library.")
else()
	add_library(csv INTERFACE)
	target_include_directories(csv INTERFACE ${CSV_INCLUDE_DIR})
endif()