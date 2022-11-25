add_executable(pretty_print
  pretty_print.cpp)

target_link_libraries(pretty_print
  PRIVATE
    Lorina
)

add_executable(ttf-sample ttf_sample.cpp)
target_link_libraries(ttf-sample
  PRIVATE
    SDL2::SDL2
    SDL2_ttf::SDL2_ttf
)

target_link_libraries(pretty_print
        PRIVATE
        SDL2::SDL2
        SDL2_ttf::SDL2_ttf
        )