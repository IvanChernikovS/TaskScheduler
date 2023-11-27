cmake_minimum_required(VERSION 3.26)
project(TaskScheduler)

set(CMAKE_CXX_STANDARD 17)

find_package (Threads REQUIRED)

add_executable(TaskScheduler main.cpp)

target_link_libraries(TaskScheduler
    ${CMAKE_THREAD_LIBS_INIT}
)