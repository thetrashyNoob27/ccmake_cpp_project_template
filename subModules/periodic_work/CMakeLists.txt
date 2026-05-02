cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "periodic_work")
set(PROJECT_BINARY "periodic_work")
project("${PROJECT_NAME}" VERSION 1.0)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE Release)

add_library("${PROJECT_NAME}" STATIC periodicWork.h)
target_include_directories("${PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties("${PROJECT_NAME}" PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties("${PROJECT_NAME}"  PROPERTIES VERSION 1.0.0 SOVERSION 1)

enable_testing()
add_subdirectory(test)
