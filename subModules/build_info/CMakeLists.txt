cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "build_info")
set(PROJECT_BINARY "build_info")
project("${PROJECT_NAME}" VERSION 1.0)


#some marcon
add_definitions(-DCOMPILER_VERSION="${CMAKE_CXX_COMPILER_VERSION}")
add_definitions(-DCOMPILER_ID="${CMAKE_CXX_COMPILER_ID}")
add_definitions(-DBUILD_TYPE="${CMAKE_BUILD_TYPE}")
execute_process(
    COMMAND date  "+%Y-%m-%d %H:%M:%S %Z"
    RESULT_VARIABLE script_result
    OUTPUT_VARIABLE build_time_str
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions(-DBUILD_TIME="${build_time_str}")

message(STATUS "Platform: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")
add_definitions(-DSYSTEM_NAME="${CMAKE_SYSTEM_NAME}")
add_definitions(-DCMAKE_SYSTEM_VERSION="${CMAKE_SYSTEM_VERSION}")


execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions(-DGIT_COMMIT_HASH="${GIT_COMMIT_HASH}")

# Fetch Git branch
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions(-DGIT_BRANCH="${GIT_BRANCH}")


execute_process(
    COMMAND git status --porcelain
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_STATUS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Determine if the repository is dirty
if (GIT_STATUS STREQUAL "")
    add_definitions(-DGIT_DIRTY="DIRTY")
else()
    add_definitions(-DGIT_DIRTY="CLEAN")
endif()


set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE Release)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_library("${PROJECT_NAME}" STATIC build_info.cpp)

set_target_properties("${PROJECT_NAME}"  PROPERTIES VERSION 1.0.0 SOVERSION 1)
