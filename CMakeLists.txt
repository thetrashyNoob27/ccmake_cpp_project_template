cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "cmake_cpp_project_template")
set(PROJECT_BINARY "cmake_cpp_project_template")
project("${PROJECT_NAME}" VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE Debug)
cmake_policy(SET CMP0167 NEW)


#boost lib settings
set(Boost_USE_STATIC_LIBS ON)
#arg processing feature & logging
find_package(Boost 1.80 REQUIRED COMPONENTS program_options log_setup log thread filesystem)
find_package(Threads)

# write marcon to header
configure_file("config.h.in" "config.h")


# main executable config
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${CMAKE_BINARY_DIR}")# for config.h.in
add_executable("${PROJECT_BINARY}" main.cpp argProcessing.cpp logging.cpp)
target_compile_options("${PROJECT_BINARY}" PRIVATE 
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:-g -o2>
)

target_link_libraries("${PROJECT_BINARY}" ${Boost_LIBRARIES} Threads::Threads)

#sub modules
function(include_sub_project subproj_name)
message(STATUS "add sub-project: ${subproj_name}")
set(subproj_dir "${CMAKE_CURRENT_SOURCE_DIR}/subModules/${subproj_name}")
message(STATUS "sub-projectpath: ${subproj_dir}")

add_subdirectory("${subproj_dir}")
target_link_libraries("${PROJECT_BINARY}" "${subproj_name}")
endfunction()

include_sub_project("sqlite3")