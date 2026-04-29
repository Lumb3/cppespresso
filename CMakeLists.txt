cmake_minimum_required(VERSION 4.1)
project(cppespresso)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Shared library
add_library(core_lib STATIC
        Server.cpp
        Controller.cpp
)

target_include_directories(core_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})  # if headers are here

target_link_libraries(core_lib PRIVATE GTest::gtest)

add_executable(cppespresso main.cpp)

target_link_libraries(cppespresso core_lib)

add_executable(UnitTests tests/ServerTests.cpp)

target_link_libraries(UnitTests core_lib GTest::gtest_main)