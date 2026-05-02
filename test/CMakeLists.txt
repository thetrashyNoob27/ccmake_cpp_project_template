# Tests are only built when BUILD_TESTS=ON
cmake_minimum_required(VERSION 3.10)

# Prevent gtest from overriding our compiler/linker options
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(INSTALL_GMOCK OFF CACHE BOOL "" FORCE)

# Build gtest from vendored source
add_subdirectory(lib/googletest)

# Include project headers for tests
include_directories("${CMAKE_SOURCE_DIR}")
include_directories("${CMAKE_BINARY_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/spdlog_sqlite3_sink")

# Test executables
add_executable(test_runner
    core/test_sample.cpp
)

target_link_libraries(test_runner PRIVATE
    gtest_main
    Threads::Threads
)

# Discover tests for ctest
include(GoogleTest)
gtest_discover_tests(test_runner)
