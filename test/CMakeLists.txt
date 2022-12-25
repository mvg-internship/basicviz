set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(benchstat
  bench.cpp)

target_link_libraries(benchstat
  PRIVATE
    Lorina
)

add_executable(ttf-sample ttf_sample.cpp)
target_link_libraries(ttf-sample
  PRIVATE
    SDL2::SDL2
    SDL2_ttf::SDL2_ttf
)

add_executable(test-viz test-viz.cpp)
target_link_libraries(test-viz
        PRIVATE
        SDL2::SDL2
        SDL2_ttf::SDL2_ttf
        Lorina
        )