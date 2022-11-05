cmake_minimum_required(VERSION 3.13)

project(LSVis VERSION 0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(SDL2 REQUIRED)

add_library(Lorina INTERFACE)
target_include_directories(Lorina
    INTERFACE
        extern/lorina/include
        extern/lorina/lib/fmt
)

add_subdirectory(test)
