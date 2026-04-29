cmake_minimum_required(VERSION 4.1)
project(cppespresso)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

set(CMAKE_CXX_STANDARD 20)

add_executable(cppespresso main.cpp
        Server.cpp
        Controller.cpp
)

add_executable(UnitTests
        tests/ServerTests.cpp
        Server.cpp
)
target_link_libraries(UnitTests GTest::gtest_main)