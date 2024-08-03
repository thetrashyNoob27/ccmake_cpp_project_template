cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "build_info")
set(PROJECT_BINARY "build_info")
project("${PROJECT_NAME}" VERSION 1.0)


#some marcon
add_definitions(-DCOMPILER_VERSION="${CMAKE_CXX_COMPILER_VERSION}")
add_definitions(-DCOMPILER_ID="${CMAKE_CXX_COMPILER_ID}")
execute_process(
    COMMAND date  "+%Y-%m-%d %H:%M:%S %Z"
    RESULT_VARIABLE script_result
    OUTPUT_VARIABLE build_time_str
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions(-DBUILD_TIME="${build_time_str}")


set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE Release)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_library("${PROJECT_NAME}" STATIC build_info.cpp)

set_target_properties("${PROJECT_NAME}"  PROPERTIES VERSION 1.0.0 SOVERSION 1)
