add_executable(app
  app.cpp)

target_link_libraries(app
  PRIVATE
    Lorina
    SDL2::SDL2
    SDL2_ttf::SDL2_ttf
)