add_executable(main main.cpp)
target_link_libraries(main
        PRIVATE
        SDL2::SDL2
        SDL2_ttf::SDL2_ttf
        pugixml::pugixml)