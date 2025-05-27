
cmake_minimum_required(VERSION 3.14)
project(TaskSchedulerTests)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(googletest)
include_directories(googletest/googletest/include)

include_directories(${CMAKE_SOURCE_DIR})
file(GLOB TEST_SOURCES "tests/*.cpp" "tests/mocks/*.h")

add_executable(TaskSchedulerTests ${TEST_SOURCES})
target_link_libraries(TaskSchedulerTests gtest gtest_main pthread)
