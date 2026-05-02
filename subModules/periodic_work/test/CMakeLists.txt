cmake_minimum_required(VERSION 3.10)

set(PROJECT_BINARY "periodic_work_gtest")
add_executable("${PROJECT_BINARY}" test_periodic_work_gtest.cpp)
target_link_libraries("${PROJECT_BINARY}" PRIVATE gtest_main Threads::Threads)
add_test(NAME "${PROJECT_BINARY}" COMMAND "${PROJECT_BINARY}")
