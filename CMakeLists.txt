cmake_minimum_required(VERSION 3.13)
include(CMakePrintHelpers)
project(LSVis VERSION 0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

add_subdirectory(extern/pugixml-1.13)

add_library(Lorina INTERFACE)
target_include_directories(Lorina
    INTERFACE
        extern/lorina/include
        extern/lorina/lib/fmt
        )

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
add_subdirectory(src)
add_subdirectory(test)