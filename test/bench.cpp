#include <lorina/bench.hpp>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include <sstream>

#include <stdio.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_ttf.h>
#include <cstdio>
#include <ctype.h>

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

// Define screen dimensions
#define SCREEN_WIDTH    1200
#define SCREEN_HEIGHT   600

// Define params of input squares
#define INPUT_WIDTH   60
#define INPUT_HEIGHT   45

#define FONT_PATH   "../assets/ttf/DejaVuSansMono.ttf"

using namespace lorina;

struct bench_statistics {
    uint32_t number_of_inputs = 0;
    uint32_t number_of_outputs = 0;
    uint32_t number_of_dffs = 0;

    std::vector<std::string> inputs;

    std::vector<std::vector<std::string>> el;// = std::vector(100000, std::vector<std::string>());

    /* lines without input and outputs */
    uint32_t number_of_lines = 0;
};

int getGateIndex(const std::string &gate, bench_statistics &stats) {
    for (int i = 0; i < stats.el.size(); ++i)
        if (gate == stats.el.at(i)[0]) return i;
}

class bench_statistics_reader : public bench_reader {
public:
    explicit bench_statistics_reader(bench_statistics &stats)
            : _stats(stats) {
    }

    virtual void on_input(const std::string &name) const override {
        (void) name;
        std::vector<std::string> local;
        std::string type = "INPUT";
        local.push_back(name);
        local.push_back(type);
        _stats.el.push_back(local);
        _stats.inputs.push_back(name);
        ++_stats.number_of_inputs;
    }

    virtual void on_output(const std::string &name) const override {
        (void) name;
        ++_stats.number_of_outputs;
    }

    virtual void on_dff_input(const std::string &input) const override {
        (void) input;
    }

    virtual void on_dff(const std::string &input, const std::string &output) const override {
        (void) input;
        (void) output;
//        int inputGate = getGate(input);
//        int outputGate = getGate(output);
        std::string type = "DFF";
        std::vector<std::string> local;
        local.push_back(output);
        local.push_back(type);
        local.push_back(input);
        _stats.el.push_back(local);
//        _stats.el.at(outputGate).push_back(type);
//        _stats.el.at(outputGate).push_back(std::to_string(inputGate));
        ++_stats.number_of_dffs;
    }

    virtual void
    on_gate(const std::vector<std::string> &inputs, const std::string &output, const std::string &type) const override {
        gate_lines.emplace_back(inputs, output, type);
//        int outputGate = getGate(output);
        std::vector<std::string> local;
        local.push_back(output);
        local.push_back(type);
//        _stats.el.at(outputGate).push_back(type);
        for (int i = 0; i < inputs.size(); ++i)
            local.push_back(inputs[i]);
//            _stats.el.at(outputGate).push_back(std::to_string(getGate(inputs[i])));
        _stats.el.push_back(local);
        _stats.number_of_lines++;
    }

    virtual void on_assign(const std::string &input, const std::string &output) const override {
        (void) input;
        (void) output;
        ++_stats.number_of_lines;
    }

    bench_statistics &_stats;
    mutable std::vector<std::tuple<std::vector<std::string>, std::string, std::string>> gate_lines;
}; /* bench_statistics_reader */

//void secondCheck(bench_statistics &stats, int index_number) {
//
//    int index_numberLength = stats.el.at(index_number).size();
//    int index_numberXCOORD = std::stoi(stats.el.at(index_number).at(index_numberLength - 3));
//    for (int i = 2; i < stats.el.at(index_number).size() - 3; ++i) {
//        int gate = getGateIndex(stats.el.at(index_number).at(i), stats);
//        int length = stats.el.at(gate).size();
//
//        int gateXCoord = std::stoi(stats.el.at(gate).at(length - 3));
//        if (gateXCoord > index_numberXCOORD) {
//            //            при изменении x убрать значение y для предыдущего x!!!
//            index_numberXCOORD = gateXCoord + 80;
//            stats.el.at(index_number).at(index_numberLength - 3) = std::to_string(index_numberXCOORD);
//        }
//    }
//    int y_coord = 200;
//    if (!stats.x_coord.at(index_numberXCOORD).empty()) {
//        int length = stats.x_coord.at(index_numberXCOORD).size();
//        y_coord = stats.x_coord.at(index_numberXCOORD).at(length - 1) + 80;
//        stats.x_coord.at(index_numberXCOORD).push_back(y_coord);
//    }
//    stats.x_coord.at(index_numberXCOORD).push_back(y_coord);
//    stats.el.at(index_number).at(index_numberLength - 2) = std::to_string(y_coord);
//}

static void
dump_statistics(FILE *f, const bench_statistics &st) {
    fprintf(f, "inputs: %u, outputs: %u, num ddfs: %u, num lines: %u\n",
            st.number_of_inputs,
            st.number_of_outputs,
            st.number_of_dffs,
            st.number_of_lines);
}

void setElCoord(bench_statistics &stats, int index_number) {
//function that specifies coordinate for all elements
    int maxXCoord = -80;
    int index_numberLength = stats.el.at(index_number).size();
    if (stats.el.at(index_number).at(index_numberLength - 1) == "Coordinates set") return;
    stats.el.at(index_number).push_back("Setting Coordinates...");

    for (int i = 2; i < stats.el.at(index_number).size() - 1; ++i) {
        int index_numberLength = stats.el.at(index_number).size();
        if (stats.el.at(index_number).at(index_numberLength - 1) == "Coordinates set") break;
        std::string gate = stats.el.at(index_number).at(i);
        int j;
        for (j = 0; j < stats.el.size(); ++j)
            if (stats.el.at(j)[0] == gate) break;

        int length = stats.el.at(j).size();
        if (stats.el.at(j).at(length - 1) == "Coordinates set") {
            maxXCoord = fmax(maxXCoord, std::stoi(stats.el.at(j).at(length - 3)));
        } else {
            if (stats.el.at(j).at(length - 1) == "Setting Coordinates...") {
                stats.el.at(j).pop_back();
                continue;
            }
            setElCoord(stats, j);

            length = stats.el.at(j).size();

            maxXCoord = fmax(maxXCoord, std::stoi(stats.el.at(j)[length - 3]));
        }
    }
    index_numberLength = stats.el.at(index_number).size();
    if (stats.el.at(index_number).at(index_numberLength - 1) == "Setting Coordinates...")
        stats.el.at(index_number).pop_back();
    int y_coord = 0;
    for (int j = 0; j < stats.el.size(); ++j) {
        int length = stats.el.at(j).size();
        if (stats.el.at(j).at(length - 3) == std::to_string(maxXCoord + 80))
            y_coord = fmax(y_coord, std::stoi(stats.el.at(j).at(length - 2)) + 80);
    }

//    if (!stats.x_coord.at(maxXCoord + 80).empty()) {
//        int length = stats.x_coord.at(maxXCoord + 80).size();
//        y_coord = stats.x_coord.at(maxXCoord + 80).at(length - 1) + 80;
//        stats.x_coord.at(maxXCoord + 80).push_back(y_coord);
//    }
//    stats.x_coord.at(maxXCoord + 80).push_back(y_coord);

    stats.el.at(index_number).push_back(std::to_string(maxXCoord + 80));
    stats.el.at(index_number).push_back(std::to_string(y_coord));
    stats.el.at(index_number).push_back("Coordinates set");
}

void setInputsCoord(bench_statistics &stats, std::string name, int index_number) {
//    function that specifies coordinate for inputs
    const int pos_x = 80;
    const int pos_y = 80 + (INPUT_HEIGHT + 10) * (index_number + 1);
//    const int index = getGate(name);
    for (int i = 0; i < stats.el.size(); ++i)
        if (stats.el.at(i)[0] == name) {
            stats.el.at(i).push_back(std::to_string(pos_x));
            stats.el.at(i).push_back(std::to_string(pos_y));
            stats.el[i].push_back("Coordinates set");
        }
}

SDL_Window *sdlInitialization() {
//    initialization
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL2 could not be initialized!\n"
               "SDL2 Error: %s\n", SDL_GetError());

        return 0;
    }
    TTF_Init();

#if defined linux && SDL_VERSION_ATLEAST(2, 0, 8)
    // Disable compositor bypass
      if(!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0"))
      {
          printf("SDL can not disable compositor bypass!\n");
          return 0;
      }
#endif

    // Create window
    SDL_Window *window = SDL_CreateWindow("SDL2_ttf sample",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created!\n"
               "SDL_Error: %s\n", SDL_GetError());
    }
    return window;
}

void eventLoop(bench_statistics &stats, SDL_Renderer *renderer, SDL_Rect *elements) {
// Event loop exit flag
    bool quit = false;

    // Event loop
    while (!quit) {
        SDL_Event e;

        // Wait indefinitely for the next available event
        SDL_WaitEvent(&e);

        // User requests quit
        if (e.type == SDL_QUIT) {
            quit = true;
        }

        // Initialize renderer color white for the background
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

        // Clear screen
        SDL_RenderClear(renderer);

        // Set renderer color red to draw the square
        SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);

        // Draw filled square
        for (int i = 0; i < stats.el.size(); ++i) {
            if (stats.el.at(i).size())
                SDL_RenderFillRect(renderer, &elements[i]);
        }

        for (int i = 0; i < stats.el.size(); ++i) {
            if (stats.el.at(i).size()) {
                for (int j = 2; j < stats.el.at(i).size() - 3; ++j) {
                    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
                    int el2 = getGateIndex(stats.el.at(i).at(j), stats);
//                    int el2 = std::stoi(stats.el.at(i).at(j));
                    SDL_RenderDrawLine(renderer, elements[i].x,
                                       elements[i].y, elements[el2].x + INPUT_WIDTH,
                                       elements[el2].y + INPUT_HEIGHT);
                }
            }
        }

        // Update screen
        SDL_RenderPresent(renderer);
    }
}

void renderer(bench_statistics &stats, SDL_Window *window) {
// Create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created!\n"
               "SDL_Error: %s\n", SDL_GetError());
    } else {
//            set inputs coordinates
        for (int i = 0; i < stats.inputs.size(); ++i) {
            setInputsCoord(stats, stats.inputs.at(i), i);
        }


//            set coordinates for other elements. Part 1
        for (int i = 0; i < stats.el.size(); ++i) {
            if (!stats.el.at(i).empty()) {
                setElCoord(stats, i);
            }
        }
//            set coordinates for other elements. Part 2
//        for (int i = 0; i < stats.el.size(); ++i) {
//            if (!stats.el.at(i).empty())
//                secondCheck(stats, i);
//        }

//          Declare rect of square
        SDL_Rect elements[stats.el.size()];


        for (int i = 0; i < stats.el.size(); ++i) {
            if (stats.el.at(i).size()) {
                int length = stats.el.at(i).size();
                elements[i].w = INPUT_WIDTH;
                elements[i].h = INPUT_HEIGHT;
                elements[i].x = std::stoi(stats.el.at(i).at(length - 3));
                elements[i].y = std::stoi(stats.el.at(i).at(length - 2));
            }
        }
        eventLoop(stats, renderer, elements);

        // Destroy renderer
        SDL_DestroyRenderer(renderer);

        // Destroy window
        SDL_DestroyWindow(window);
    }
}

int
main(int argc, char *argv[]) {
//    window initialization
    SDL_Window *window = sdlInitialization();


    for (int i = 1; i < argc; ++i) {
        std::ifstream ifs(argv[i]);

        bench_statistics stats;
        bench_statistics_reader reader(stats);
        auto result = read_bench(ifs, reader);

        if (result == return_code::success) {
            dump_statistics(stdout, stats);
        }
        const auto &gate_lines = reader.gate_lines;

        renderer(stats, window);

        // Quit SDL2_ttf
        TTF_Quit();

        // Quit SDL
        SDL_Quit();

    }
    return 0;
}
