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


target_link_libraries(benchstat
        PRIVATE
        SDL2::SDL2
        SDL2_ttf::SDL2_ttf
        )
