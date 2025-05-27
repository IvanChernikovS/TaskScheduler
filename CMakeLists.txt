cmake_minimum_required(VERSION 3.26)
project(TaskScheduler)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

add_executable(TaskScheduler
    main.cpp
    TaskSchedulerImpl.cpp
    Task.cpp
    ThreadPool.cpp
    Randomizer.cpp
    EasyLogging/easylogging++.cc)

include_directories(${CMAKE_SOURCE_DIR}/EasyLogging
    ${CMAKE_SOURCE_DIR}/Interface)

target_link_libraries(TaskScheduler
    ${CMAKE_THREAD_LIBS_INIT}
)

target_compile_definitions(TaskScheduler PRIVATE ELPP_THREAD_SAFE)