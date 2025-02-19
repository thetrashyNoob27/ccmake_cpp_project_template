cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "spdlog")
set(PROJECT_BINARY "spdlog")
project("${PROJECT_NAME}" VERSION 1.0)


set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE Release)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")


file(GLOB_RECURSE SOURCE_FILES "*.h")
add_library("${PROJECT_NAME}" STATIC ${SOURCE_FILES})
target_compile_options("${PROJECT_BINARY}" PRIVATE 
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:-g -o2>
)


set_target_properties("${PROJECT_NAME}" PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties("${PROJECT_NAME}"  PROPERTIES VERSION 1.15.1 SOVERSION 1)

enable_testing()