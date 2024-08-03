cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "project_archieve")
set(PROJECT_BINARY "project_archieve")
project("${PROJECT_NAME}" VERSION 1.0)

message(STATUS "root project path: ${ROOT_PROJECT_PATH}")
# tar pack 
execute_process(
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/project_package.sh "${ROOT_PROJECT_PATH}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE script_output
    RESULT_VARIABLE script_result
    ERROR_VARIABLE script_error
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(" package process output: ${script_output}")
message(" package process error:  ${script_error}")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE Release)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_library("${PROJECT_NAME}" STATIC project_archieve.cpp project_archieve_data.cpp)

set_target_properties("${PROJECT_NAME}"  PROPERTIES VERSION 1.0.0 SOVERSION 1)
