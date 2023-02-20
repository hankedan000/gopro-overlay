add_library(test_data INTERFACE test_data.h)
target_include_directories(test_data
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
# make a directory for test to generate temporary data into
set(TEST_DATA_TMP_ROOT "${CMAKE_CURRENT_BINARY_DIR}/tmp")
file(MAKE_DIRECTORY ${TEST_DATA_TMP_ROOT})

target_compile_definitions(test_data
    INTERFACE
        -DTEST_DATA_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
        -DTEST_DATA_TMP_ROOT="${TEST_DATA_TMP_ROOT}"
)
target_compile_features(test_data
    INTERFACE
        cxx_std_17# using std::filesystem
)