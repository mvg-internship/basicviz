#include <lorina/bench.hpp>
#include <string>
#include <vector>
#include <stdio.h>

#include <SDL.h>
#include <SDL_ttf.h>


enum {
    SCREEN_WIDTH = 800,
    SCREEN_HEIGHT = 600,
    indent = 10,
    fcoef_scale = 4,
    scoef_scale = 3
};

using namespace lorina;

struct parameters {
    std::vector<std::string> inputs;
    std::string type = "";
    std::string output = "";

    //coordinates of first and second points
    float fp_x= 0;
    float fp_y = 0;
    float sp_x = 0;
    float sp_y = 0;

    int coeff_x = 0;
    int coeff_y = 0;


    parameters(const std::vector<std::string> &inputs, const std::string &type, const std::string &output)
            : inputs(inputs), type(type), output(output) {}
};

struct bench_statistics {
    uint32_t number_of_inputs = 0;
    uint32_t number_of_outputs = 0;
    uint32_t number_of_dffs = 0;

    /* lines without input and outputs */
    uint32_t number_of_lines = 0;
    uint32_t number_of_edges = 0;

    std::vector<parameters> element;
};


void addEdge(bench_statistics &stats, const std::vector<std::string> &inputs, const std::string &type = "",
             const std::string &output = ""){
    stats.element.emplace_back(inputs, type, output);
}

void addingInputs( const bench_statistics &stats, std::vector<parameters> &temp,
                   std::vector<std::vector<parameters>> &matrix){
    for (const auto &iter : stats.element) {
        const std::string &type = iter.type;
        if (type == "input") {
            temp.emplace_back(iter);
        }
    }
    matrix.emplace_back(temp);
    temp.clear();
}

void addingElements( const bench_statistics &stats, std::vector<parameters> &temp){
    for (const auto &iter : stats.element) {
        const std::string &type = iter.type;
        if (type != "input" && type != "output") {
            temp.emplace_back(iter);
        }
    }
}

void addingOutputs ( const bench_statistics &stats, std::vector<parameters> &temp,
                     std::vector<std::vector<parameters>> &matrix){
    for (const auto &iter : stats.element) {
        if (iter.type == "output") {
            temp.emplace_back(iter);
        }
    }
    matrix.emplace_back(temp);
    temp.clear();
}

void sortElements(std::vector<parameters> &temp, std::vector<std::vector<parameters>> &matrix){
    int layer_num = 0;
    while (!temp.empty()) {
        matrix.emplace_back();
        for (const auto &matrix_it : matrix[layer_num]) {
            for (int j = 0; j < temp.size(); ++j) {
                const auto &temp_inputs = temp[j].inputs;
                auto temp_it = std::find(temp_inputs.begin(), temp_inputs.end(), matrix_it.output);
                if (temp_it != temp_inputs.end()) {
                    matrix.back().emplace_back(temp[j]);
                    temp.erase(temp.begin() + j);
                    j--;
                }
            }
        }
        layer_num++;
    }
    temp.clear();
}

void fillGraph(bench_statistics &stats, std::vector<std::vector<parameters>> &matrix) {
    std::vector<parameters> temp;

    addingInputs(stats, temp, matrix);
    addingElements(stats, temp);
    sortElements(temp, matrix);
    addingOutputs(stats, temp, matrix);
}

size_t maxStr(const std::vector<std::vector<parameters>> &matrix) {
    size_t max = 0;
    for (const auto &iter : matrix) {
        if (max < iter.size()) max = iter.size();
    }
    return max;
}

void calculate_fp_coordinates( const std::vector<parameters>::iterator &it_str,
                               const std::vector<parameters>::iterator &el_pred_str,
                               std::vector<std::vector<parameters>> &matrix) {
    it_str->fp_x = el_pred_str->sp_x + fcoef_scale * SCREEN_WIDTH / (2*scoef_scale*matrix.size());
    it_str->fp_y = el_pred_str->sp_y + SCREEN_WIDTH / (fcoef_scale*matrix.size());

}

void calculate_sp_coordinates( const std::vector<parameters>::iterator &it_str,
                               const std::vector<std::vector<parameters>>::iterator &it_matrix,
                               std::vector<std::vector<parameters>> &matrix) {
    it_str->sp_x = indent + (SCREEN_WIDTH/matrix.size()) * (it_matrix - matrix.begin());
    it_str->sp_y = indent + (SCREEN_HEIGHT / maxStr(matrix)) * (it_str - it_matrix->begin());

}

void setCoordinates(std::vector<std::vector<parameters>> &matrix) {
    for (auto it_matrix = matrix.begin(); it_matrix != matrix.end(); ++it_matrix) {
        for (auto it_str = it_matrix->begin(); it_str != it_matrix->end(); ++it_str) {
            it_str->coeff_x = it_matrix - matrix.begin();
            it_str->coeff_y = it_str - it_matrix->begin();

            if (it_str->type == "input"){
                calculate_sp_coordinates(it_str, it_matrix, matrix);
            }

            else{
                for (auto el_pred_str = (it_matrix-1)->begin(); el_pred_str != (it_matrix-1)->end(); ++el_pred_str){

                    auto result = std::find(it_str->inputs.begin(), it_str->inputs.end(), el_pred_str->output);
                    if (result != it_str->inputs.end()) {
                        calculate_fp_coordinates(it_str, el_pred_str, matrix);
                        calculate_sp_coordinates(it_str, it_matrix, matrix);
                    }
                }
            }
        }
    }
}

void matrix_to_vector(std::vector<parameters> &vector, const std::vector<std::vector<parameters>> &matrix) {
    for (const auto &raw: matrix){
        for (const auto &element : raw){
            vector.emplace_back(element);
        }
    }
}

void debug(FILE *f, const bench_statistics &stats, const std::vector<parameters> &vector_of_elements ){
    fprintf(f, "All elements of the file:\n");
    for (const auto &element : stats.element){
        for (const auto &inputs : element.inputs){
            fprintf(f, "%s ", inputs.c_str());
        }
        fprintf(f, " || %s || %s\n",
                element.type.c_str(), element.output.c_str());
    }

    fprintf(f, "----------------------------------------\n");

    fprintf(f, "Vector of elements\n");
    for (const auto &element : vector_of_elements){
        for (const auto &inputs : element.inputs){
            fprintf(f, "%s ", inputs.c_str());
        }
        fprintf(f, "%s %s in(%.1f %.1f) out(%.1f %.1f) |%d %d|\n",
                element.type.c_str(), element.output.c_str(),
                element.fp_x, element.fp_y,
                element.sp_x, element.sp_y,
                element.coeff_x, element.coeff_y);
    }
}

class bench_statistics_reader : public bench_reader {
public:

    explicit bench_statistics_reader(bench_statistics &stats)
            : _stats(stats) {
    }

    virtual void on_input(const std::string &name) const override {
        std::vector<std::string> input;
        addEdge(_stats, input, "input", name);
        ++_stats.number_of_inputs;
        ++_stats.number_of_edges;
    }

    virtual void on_output(const std::string &name) const override {
        std::vector<std::string> input;
        input.emplace_back(name);
        addEdge(_stats, input, "output");
        ++_stats.number_of_outputs;
        ++_stats.number_of_edges;
    }

    virtual void on_dff(const std::string &input, const std::string &output) const override {
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
        std::vector<std::string> inp;
        inp.emplace_back(input);
        addEdge(_stats, inp, "dff", output);
        ++_stats.number_of_lines;
        ++_stats.number_of_edges;
    }

    bench_statistics &_stats;
    mutable std::vector<std::tuple<std::vector<std::string>, std::string, std::string>> gate_lines;
}; /* bench_statistics_reader */

static void dump_statistics(FILE *f, const bench_statistics &stats) {
    fprintf(f, "inputs: %u, outputs: %u, num ddfs: %u, num lines: %u\n",
            stats.number_of_inputs,
            stats.number_of_outputs,
            stats.number_of_dffs,
            stats.number_of_lines);
}

void eventloop(const bench_statistics &stats, SDL_Renderer *renderer, const std::vector<parameters> &vector_of_elements,
               const std::vector<std::vector<parameters>> &matrix){

    SDL_Rect elements[vector_of_elements.size()];

    for (int i = 0; i < vector_of_elements.size(); ++i) {
        // Dimensions of the inputs
        elements[i].w = fcoef_scale * SCREEN_WIDTH / (2*scoef_scale*matrix.size());
        elements[i].h = SCREEN_WIDTH / (2*matrix.size());


        // Location of the rects
        elements[i].x = indent + (SCREEN_WIDTH/matrix.size()) * vector_of_elements[i].coeff_x;
        elements[i].y = indent + (SCREEN_HEIGHT / maxStr(matrix)) * vector_of_elements[i].coeff_y;

    }

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
        for (size_t i = stats.number_of_inputs; i < stats.element.size(); ++i) {
            SDL_RenderDrawLine(renderer, vector_of_elements[i].fp_x,
                               vector_of_elements[i].fp_y,
                               vector_of_elements[i].sp_x,
                               vector_of_elements[i].sp_y);
        }


        // Update screen
        SDL_RenderPresent(renderer);
    }
}

void renderer(const bench_statistics &stats, SDL_Window *window, const std::vector<parameters> &vector_of_elements,
              const std::vector<std::vector<parameters>> &matrix){

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        printf("Renderer could not be created!\n"
               "SDL_Error: %s\n", SDL_GetError());
    } else {
        eventloop(stats, renderer, vector_of_elements, matrix);

        SDL_DestroyRenderer(renderer);
    }
}

void sdl_initialization(const bench_statistics &stats, const std::vector<parameters> &vector_of_elements,
                               const std::vector<std::vector<parameters>> &matrix){

#if defined linux && SDL_VERSION_ATLEAST(2, 0, 8)
    // Disable compositor bypass
    if(!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0"))
    {
        printf("SDL can not disable compositor bypass!\n");
        return 0;
    }
#endif
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL2 could not be initialized!\n"
               "SDL2 Error: %s\n", SDL_GetError());
    }

    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("basicviz",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created!\n"
               "SDL_Error: %s\n", SDL_GetError());
    }
    renderer(stats, window, vector_of_elements, matrix);

    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

int
main(int argc, char *argv[]) {

    bench_statistics stats;
    bench_statistics_reader reader(stats);

    std::vector<std::vector<parameters>> matrix;
    std::vector<parameters> vector_of_elements;

    for (int i = 1; i < argc; ++i) {
        std::ifstream ifs(argv[i]);

        auto result = read_bench(ifs, reader);
        if (result == return_code::success) {
            dump_statistics(stdout, stats);
        }

    }

    //Working with data
    fillGraph(stats, matrix);
    setCoordinates(matrix);
    matrix_to_vector(vector_of_elements, matrix);

    //Output of info for debugging
    debug(stdout, stats, vector_of_elements);

    //Visualization
    sdl_initialization(stats, vector_of_elements, matrix);

    return 0;
}