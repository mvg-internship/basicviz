#include <lorina/bench.hpp>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_ttf.h>

// Define MAX and MIN macros
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

// Define screen dimensions
#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

#define FONT_PATH   "../assets/ttf/DejaVuSansMono.ttf"

using namespace lorina;

struct bench_statistics {
    uint32_t number_of_inputs = 0;
    uint32_t number_of_outputs = 0;
    uint32_t number_of_dffs = 0;

    /* lines without input and outputs */
    uint32_t number_of_lines = 0;
};

class bench_statistics_reader : public bench_reader {
public:
    explicit bench_statistics_reader(bench_statistics &stats)
            : _stats(stats) {
    }

    virtual void on_input(const std::string &name) const override {
        (void) name;
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
        ++_stats.number_of_dffs;
    }

    virtual void
    on_gate(const std::vector<std::string> &inputs, const std::string &output, const std::string &type) const override {
        gate_lines.emplace_back(inputs, output, type);
        ++_stats.number_of_lines;
    }

    virtual void on_assign(const std::string &input, const std::string &output) const override {
        (void) input;
        (void) output;
        ++_stats.number_of_lines;
    }

    bench_statistics &_stats;
    mutable std::vector<std::tuple<std::vector<std::string>, std::string, std::string>> gate_lines;
}; /* bench_statistics_reader */

static void
dump_statistics(FILE *f, const bench_statistics &st) {
    fprintf(f, "inputs: %u, outputs: %u, num ddfs: %u, num lines: %u\n",
            st.number_of_inputs,
            st.number_of_outputs,
            st.number_of_dffs,
            st.number_of_lines);
}

int
main(int argc, char *argv[]) {
#if defined linux && SDL_VERSION_ATLEAST(2, 0, 8)
    // Disable compositor bypass
    if(!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0"))
    {
        printf("SDL can not disable compositor bypass!\n");
        return 0;
    }
#endif
    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL2 could not be initialized!\n"
               "SDL2 Error: %s\n", SDL_GetError());
        return 0;
    }
    // Initialize SDL2_ttf
    TTF_Init();

    bench_statistics stats;
    for (int i = 1; i < argc; ++i) {
        std::ifstream ifs(argv[i]);

        bench_statistics_reader reader(stats);
        auto result = read_bench(ifs, reader);
        if (result == return_code::success) {
            dump_statistics(stdout, stats);
        }

    }
    SDL_Rect inputs[stats.number_of_inputs];
    SDL_Rect outputs[stats.number_of_inputs];

    // Create window
    SDL_Window *window = SDL_CreateWindow("SDL2_ttf sample",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created!\n"
               "SDL_Error: %s\n", SDL_GetError());
    } else {
        // Create renderer
        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            printf("Renderer could not be created!\n"
                   "SDL_Error: %s\n", SDL_GetError());
        } else {

            for (int i = 0; i < stats.number_of_inputs; ++i) {
                // Dimensions of the inputs
                inputs[i].w = 50;
                inputs[i].h = 50;
                // Location of the rect
                inputs[i].x = 10;
                inputs[i].y = SCREEN_HEIGHT/stats.number_of_inputs - inputs[i].h*stats.number_of_inputs/2 + (inputs[i].h + 10) * i;


                SDL_Color textColor = {0x00, 0x00, 0x00, 0xFF};
                SDL_Color textBackgroundColor = {0xFF, 0xFF, 0xFF, 0xFF};

            }

            for (int i = 0; i < stats.number_of_outputs; ++i) {
                // Dimensions of the outputs
                outputs[i].w = 100;
                outputs[i].h = 75;
                // Location of the rect
                outputs[i].x = SCREEN_WIDTH-outputs[i].w - 10;
                outputs[i].y = 50 + (outputs[i].h + 10) * i;


                SDL_Color textColor = {0x00, 0x00, 0x00, 0xFF};
                SDL_Color textBackgroundColor = {0xFF, 0xFF, 0xFF, 0xFF};

            }

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
                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);

                // Draw filled square
                for (int i = 0; i < stats.number_of_inputs; ++i) {
                    SDL_RenderFillRect(renderer, &inputs[i]);
                }
                for (int i = 0; i < stats.number_of_outputs; ++i) {
                    SDL_RenderFillRect(renderer, &outputs[i]);
                }

                // Update screen
                SDL_RenderPresent(renderer);
            }

            // Destroy renderer
            SDL_DestroyRenderer(renderer);

            // Destroy window
            SDL_DestroyWindow(window);
        }

        // Quit SDL2_ttf
        TTF_Quit();

        // Quit SDL
        SDL_Quit();

    }
    return 0;
}