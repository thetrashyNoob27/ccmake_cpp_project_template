cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "pipeline")
set(PROJECT_BINARY "pipeline")
project("${PROJECT_NAME}" VERSION 1.0)


set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE Release)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_library("${PROJECT_NAME}" STATIC pipeline.hpp)
target_compile_options("${PROJECT_BINARY}" PRIVATE 
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:-g -o2>
)


set_target_properties("${PROJECT_NAME}" PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties("${PROJECT_NAME}"  PROPERTIES VERSION 1.0.0 SOVERSION 1)

enable_testing()
add_subdirectory(test)