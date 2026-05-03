cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "messageDistributor")
set(PROJECT_BINARY "messageDistributor")
project("${PROJECT_NAME}" VERSION 1.0)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE Release)

add_library("${PROJECT_NAME}" INTERFACE)
target_include_directories("${PROJECT_NAME}" INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

enable_testing()
add_subdirectory(test)
