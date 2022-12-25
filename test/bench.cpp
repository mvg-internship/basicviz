#include <lorina/bench.hpp>
#include <lorina/lorina.hpp>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdbool.h>
#include <list>
#include <algorithm>

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

struct parametrs {
    std::vector<std::string> inputs;
    std::string type = "none";
    std::string output = "none";

//    int width = 0;
//    int length = 0;

    float fp_x= 0;
    float fp_y = 0;
    float sp_x = 0;
    float sp_y = 0;

    int koef_x = 0;
    int koef_y = 0;


    parametrs(const std::vector<std::string> &inputs, const std::string &type, const std::string &output)
            : inputs(inputs), type(type), output(output) {}
};

struct bench_statistics {
    uint32_t number_of_inputs = 0;
    uint32_t number_of_outputs = 0;
    uint32_t number_of_dffs = 0;

    /* lines without input and outputs */
    uint32_t number_of_lines = 0;
    uint32_t number_of_edges = 0;

    std::vector<parametrs> element;
};


void addEdge(bench_statistics &stats, const std::vector<std::string> &inputs, const std::string &type = "none",
             const std::string &output = "none") {
    stats.element.emplace_back(inputs, type, output);
}

void sortGraph(bench_statistics &stats, std::vector<std::vector<parametrs>> &matrix) {
    std::vector<parametrs> temp;

//  запись input'ов в временный массив
    for (auto iter = stats.element.begin(); iter != stats.element.end(); ++iter) {
        std::string type = iter->type;
        if (type == "input") {
            temp.emplace_back(*iter);
        }
    }
    matrix.emplace_back(temp);
    temp.clear();

//  запись всех элементов в временный массив
    for (auto iter = stats.element.begin(); iter != stats.element.end(); ++iter) {
        std::string type = iter->type;
        if (type != "input" && type != "output") {
            temp.emplace_back(*iter);
        }
    }

//  сортировка элементов и запись в матрицу
 int layer_num = 0;
    while (temp.size() != 0) {
        matrix.emplace_back();
        for (auto matrix_it = matrix[layer_num].begin(); matrix_it != matrix[layer_num].end(); ++matrix_it) {
            for (int j = 0; j < temp.size(); ++j) {
                auto temp_it = std::find(temp[j].inputs.begin(), temp[j].inputs.end(), matrix_it->output);
                if (temp_it != temp[j].inputs.end()) {
                    matrix.back().emplace_back(temp[j]);
                    temp.erase(temp.begin() + j);
                    j--;
                }
            }
        }
        layer_num++;
    }
    temp.clear();

//  запись output'ов в временный массив
    for (auto iter = stats.element.begin(); iter != stats.element.end(); ++iter) {
        if (iter->output == "none") {
            temp.emplace_back(*iter);
        }
    }
    matrix.emplace_back(temp);
    temp.clear();
}

int maxStr(std::vector<std::vector<parametrs>> &matrix) {
    int max = 0;
    for (auto iter = matrix.begin(); iter != matrix.end(); ++iter) {
        if (max < iter->size()) max = iter->size();
    }
    return max;
}

void setKoef(std::vector<std::vector<parametrs>> &matrix) {

    for (int i =0; i<matrix.size(); ++i) {
        for (int j =0; j<matrix[i].size(); ++j) {
            matrix[i][j].koef_x = i;
            matrix[i][j].koef_y = j;

            if (matrix[i][j].type == "input"){

                matrix[i][j].sp_x = 10 + (SCREEN_WIDTH/matrix.size()) * i;
                matrix[i][j].sp_y = 10 + (SCREEN_HEIGHT / maxStr(matrix)) * j;

            }

            if (matrix[i][j].type != "input"){
                for (auto el_pred_str = matrix[i-1].begin(); el_pred_str != matrix[i-1].end(); ++el_pred_str){

                    auto iter = std::find(matrix[i][j].inputs.begin(), matrix[i][j].inputs.end(), el_pred_str->output);
                    if (iter != matrix[i][j].inputs.end()) {

                        matrix[i][j].fp_x = el_pred_str->sp_x + 4 * SCREEN_WIDTH / (6*matrix.size());
                        matrix[i][j].fp_y = el_pred_str->sp_y + SCREEN_WIDTH / (4*matrix.size());

                        matrix[i][j].sp_x = 10 + (SCREEN_WIDTH/matrix.size()) * i;
                        matrix[i][j].sp_y = 10 + (SCREEN_HEIGHT / maxStr(matrix)) * j;
                    }
                }
            }
        }
    }
}

void matrix_to_vector(std::vector<parametrs> &vector, std::vector<std::vector<parametrs>> &matrix) {
    for (auto i = matrix.begin(); i != matrix.end(); ++i) {
        for (auto j = i->begin(); j != i->end(); ++j) {
            vector.emplace_back(*j);
        }
    }
}

class bench_statistics_reader : public bench_reader {
public:

    explicit bench_statistics_reader(bench_statistics &stats)
            : _stats(stats) {
    }

    virtual void on_input(const std::string &name) const override {
        (void) name;
        std::vector<std::string> input;
        input.emplace_back("none");
        addEdge(_stats, input, "input", name);
        ++_stats.number_of_inputs;
        ++_stats.number_of_edges;
    }

    virtual void on_output(const std::string &name) const override {
        (void) name;
        std::vector<std::string> input;
        input.emplace_back(name);
        addEdge(_stats, input, "output", "none");
        ++_stats.number_of_outputs;
        ++_stats.number_of_edges;
    }

    virtual void on_dff_input(const std::string &input) const override {
        (void) input;
    }

    virtual void on_dff(const std::string &input, const std::string &output) const override {
        (void) input;
        (void) output;
        std::vector<std::string> inp;
        inp.emplace_back(input);
        addEdge(_stats, inp, "dff", output);
        ++_stats.number_of_dffs;
        ++_stats.number_of_edges;
    }

    virtual void
    on_gate(const std::vector<std::string> &inputs, const std::string &output, const std::string &type) const override {
        gate_lines.emplace_back(inputs, output, type);
        addEdge(_stats, inputs, type, output);
        ++_stats.number_of_lines;
        ++_stats.number_of_edges;
    }

    virtual void on_assign(const std::string &input, const std::string &output) const override {
        (void) input;
        (void) output;
        std::vector<std::string> inp;
        inp.emplace_back(input);
        addEdge(_stats, inp, "dff", output);
        ++_stats.number_of_lines;
        ++_stats.number_of_edges;
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
    bench_statistics_reader reader(stats);

    for (int i = 1; i < argc; ++i) {
        std::ifstream ifs(argv[i]);

        auto result = read_bench(ifs, reader);
        if (result == return_code::success) {
            std::cout << "Parsing: success" << std::endl;
//            dump_statistics(stdout, stats);
        }

    }

//  вывод содержащихся в структуре элементов
    std::cout << "All elements of the file" << std::endl;
    for (int i = 0; i < stats.element.size(); ++i) {
        for (int j = 0; j < stats.element[i].inputs.size(); ++j) {
            std::cout << stats.element[i].inputs[j] << " ";
        }
        std::cout << " || " << stats.element[i].type << " || ";
        std::cout << stats.element[i].output << std::endl;
    }

    std::cout << "----------------------------------------" << std::endl;

    std::vector<std::vector<parametrs>> matrix;
    std::vector<parametrs> vector_of_elements;

    sortGraph(stats, matrix);
    setKoef(matrix);
    matrix_to_vector(vector_of_elements, matrix);

    //  вывод полученной matrix
//    std::cout  << "Sorted matrix" << std::endl;
//    for (int i = 0; i < matrix.size(); ++i) {
//        for (int j = 0; j < matrix[i].size(); ++j) {
//            for (int k = 0; k < matrix[i][j].inputs.size(); ++k)
//                std::cout << matrix[i][j].inputs[k] << "  ";
//            std::cout << matrix[i][j].type << "  " << matrix[i][j].output << " | " << matrix[i][j].koef_x <<
//                      "  " << matrix[i][j].koef_y << " | ";
//        }
//        std::cout << std::endl;
//    }

    std::cout  << "vector of elements" << std::endl;
    for (auto iter = vector_of_elements.begin(); iter != vector_of_elements.end(); ++iter){
        for (auto inp = iter->inputs.begin(); inp != iter->inputs.end(); ++inp)
            std::cout << *inp << " ";
        std::cout << iter->type << " " << iter->output << "  in(" << iter->fp_x << " " << iter->fp_y <<")  out(" <<
        iter->sp_x << " " << iter->sp_y << ")" << "  |"  << iter->koef_x << " " << iter->koef_y << "|" << std::endl;
    }


    SDL_Rect elements[vector_of_elements.size()];


    // Create window
    SDL_Window *window = SDL_CreateWindow("SDL2_ttf sample",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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

            for (int i = 0; i < vector_of_elements.size(); ++i) {

                // Dimensions of the inputs
                elements[i].w = 4 * SCREEN_WIDTH / (6*matrix.size());
                elements[i].h = SCREEN_WIDTH / (2*matrix.size());


                // Location of the rects
                elements[i].x = 10 + (SCREEN_WIDTH/matrix.size()) * vector_of_elements[i].koef_x;
                elements[i].y = 10 + (SCREEN_HEIGHT / maxStr(matrix)) * vector_of_elements[i].koef_y;



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
                SDL_SetRenderDrawColor(renderer, 0x00, 0xAA, 0xFF, 0xFF);

                // Draw filled square
                for (int i = 0; i < vector_of_elements.size(); ++i) {
                    SDL_RenderFillRect(renderer, &elements[i]);
                }

                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
                // Draw lines
                for (int i = stats.number_of_inputs; i < stats.element.size(); ++i) {
                    SDL_RenderDrawLine(renderer, vector_of_elements[i].fp_x,
                                                    vector_of_elements[i].fp_y,
                                                        vector_of_elements[i].sp_x,
                                                            vector_of_elements[i].sp_y);
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