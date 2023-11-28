cmake_minimum_required(VERSION 3.26)
project(TaskScheduler)

set(CMAKE_CXX_STANDARD 17)

find_package (Threads REQUIRED)

add_executable(TaskScheduler
    main.cpp
    Randomizer.cpp
    EasyLogging/easylogging++.cc)

set(LoggerPath ${CMAKE_SOURCE_DIR}/EasyLogging)
include_directories(${LoggerPath})

target_link_libraries(TaskScheduler
    ${CMAKE_THREAD_LIBS_INIT}
)