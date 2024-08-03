cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "cmake_cpp_project_template")
set(PROJECT_BINARY "cmake_cpp_project_template")
project("${PROJECT_NAME}" VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE Debug)



# arg prase feature
find_library(ARG_PRASE_LIB boost_program_options)
if (ARG_PRASE_LIB)
add_definitions(-DENABLE_ARG_PARSE)
message(STATUS "boost::program_options found: ${ARG_PRASE_LIB}")
else()
message(WARNING "not found, disable arg processing.")
endif()


# logingfeature 
find_library(BOOST_LOG_LIB boost_program_options)
find_library(BOOST_LOG_SETUP_LIB boost_log_setup)
find_library(BOOST_THREAD_LIB boost_thread)
find_library(BOOST_FILESYSTEM_LIB boost_filesystem)
if(BOOST_LOG_LIB AND BOOST_LOG_SETUP_LIB AND BOOST_THREAD_LIB AND BOOST_FILESYSTEM_LIB)
set(ENABLE_LOGGING TRUE)
add_definitions(-DENABLE_LOGGING)
message(STATUS "boost lib's logging requirement found: ${BOOST_LOG_LIB} ${BOOST_LOG_SETUP_LIB} ${BOOST_THREAD_LIB} ${BOOST_FILESYSTEM_LIB}")
else()
message(WARNING "not found, disable logging processing.")
endif()



# write marcon to header
configure_file(config.h.in config.h)

# main executable config
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${CMAKE_BINARY_DIR}")# for config.h.in
add_executable("${PROJECT_BINARY}" main.cpp argProcessing.cpp logging.cpp)
target_compile_options("${PROJECT_BINARY}" PRIVATE 
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:-O2>
)

if (ARG_PRASE_LIB)
target_link_libraries("${PROJECT_BINARY}" PRIVATE ${ARG_PRASE_LIB})
else()
endif()

if (ENABLE_LOGGING)
target_link_libraries("${PROJECT_BINARY}" PRIVATE ${BOOST_LOG_LIB} ${BOOST_LOG_SETUP_LIB} ${BOOST_THREAD_LIB} ${BOOST_FILESYSTEM_LIB})
else()
endif()

