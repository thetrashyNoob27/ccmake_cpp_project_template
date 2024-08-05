cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "sqlite3")
set(PROJECT_BINARY "sqlite3")
project("${PROJECT_NAME}" VERSION 1.0)


set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE Release)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_library("${PROJECT_NAME}" STATIC shell.c sqlite3.c)

set_target_properties("${PROJECT_NAME}"  PROPERTIES VERSION 1.0.0 SOVERSION 1)