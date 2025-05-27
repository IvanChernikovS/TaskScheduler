cmake_minimum_required(VERSION 3.26)
project(TaskScheduler)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

# ========== Include Directories ==========
include_directories(
        ${CMAKE_SOURCE_DIR}/EasyLogging
        ${CMAKE_SOURCE_DIR}/Interface
        ${CMAKE_SOURCE_DIR}/Tests/Mocks
)

# ========== Main Application ==========
add_executable(TaskScheduler
        main.cpp
        TaskSchedulerImpl.cpp
        Task.cpp
        ThreadPool.cpp
        Randomizer.cpp
        EasyLogging/easylogging++.cc
)

target_link_libraries(TaskScheduler
        ${CMAKE_THREAD_LIBS_INIT}
)

target_compile_definitions(TaskScheduler PRIVATE ELPP_THREAD_SAFE)

# ========== GoogleTest Setup ==========
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

# ========== Unit Tests ==========
file(GLOB TEST_SOURCES
        "Tests/*.cpp"
        "Tests/Mocks/*.h"
)

add_executable(TaskSchedulerTests
        ${TEST_SOURCES}
        TaskSchedulerImpl.cpp
        Task.cpp
        ThreadPool.cpp
        Randomizer.cpp
        EasyLogging/easylogging++.cc
)

target_link_libraries(TaskSchedulerTests
        gtest
        gtest_main
        gmock
        pthread
)

target_compile_definitions(TaskSchedulerTests PRIVATE ELPP_THREAD_SAFE)

include(GoogleTest)
gtest_discover_tests(TaskSchedulerTests)
