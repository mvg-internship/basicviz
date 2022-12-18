#include <lorina/bench.hpp>
#include <lorina/lorina.hpp>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdbool.h>
#include <list>

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

    parametrs(const std::vector<std::string> &inputs, const std::string &type , const std::string &output)
        : inputs(inputs), type(type), output(output){}
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

//int getGate(const std::string &gate) {
//    return std::stoi(gate.substr(1));
//}

void addEdge(bench_statistics &stats, const std::vector<std::string> &inputs, const std::string &type = "none", const std::string &output = "none"){
    stats.element.emplace_back(inputs, type, output);
}

void sortGraph(bench_statistics &stats, std::vector <std::vector<parametrs>> &array){
    std::vector<parametrs> temp;
//    std::vector <std::vector<parametrs>> array;

//  запись input'ов в временный массив
    for (int i = 0; i<stats.element.size(); ++i){
        if (stats.element[i].inputs[0] == "none"){
            temp.emplace_back(stats.element[i]);
        }
    }
    array.emplace_back(temp);
    temp.clear();

//  запись всех элементов в временный массив
    for (int i = 0; i<stats.element.size(); ++i){
        if (stats.element[i].inputs[0] != "none" && stats.element[i].output != "none"){
            temp.emplace_back(stats.element[i]);
        }
    }
//  сортировка элементов и запись в матрицу
////выделяется лишнаяя область памяти
    int k = 0, flag = 0;
    std::vector<parametrs> clear;
    while(temp.size() != 0){
        for (int i=0; i<array[k].size(); ++i){
            for (int j=0; j<temp.size(); ++j){
                if (temp[j].inputs[0] == array[k][i].output || temp[j].inputs[1] == array[k][i].output){
                    array.emplace_back();
                    array[k+1].emplace_back(temp[j]);

                    //вывод-проверка проходящих через условие элементов
//                    std::cout << (temp.begin()+j)->inputs[0] << " " << (temp.begin()+j)->inputs[1] << " " << (temp.begin()+j)->type
//                        << "  " << (temp.begin()+j)->output << std::endl;

                    temp.erase(temp.begin()+j); j--;
                }
            }
        }
        k++;
    }
    temp.clear();

////  перераспределение памяти матрицы -- не работает
//    for (int i = 0; i <array.size(); ++i)
//        if (array.empty() == true){ array[i].clear(); array[i].shrink_to_fit();}


//  запись output'ов в временный массив
    for (int i = 0; i<stats.element.size(); ++i){
        if (stats.element[i].output == "none"){
            temp.emplace_back(stats.element[i]);
        }
    }
    array.emplace_back(temp);
    temp.clear();

////  вывод полученной матрицы
//    for (int i = 0; i < array.size(); ++i ){
//        for (int j = 0; j < array[i].size(); ++j ){
//            for (int k = 0; k < array[i][j].inputs.size(); ++k)
//                std::cout << array[i][j].inputs[k] << "  ";
//            std::cout << array[i][j].type << "  " << array[i][j].output << " || ";
//        }
//        std::cout << std::endl;
//    }

//    std::vector<parametrs> array;

// 1 демо версия сортировки
//    std::string temp;
//    temp = stats.element[0].output;
//    for (int i=0; i < stats.element.size(); ++i){
//        if ( temp == stats.element[i].inputs[0] || temp == stats.element[i].inputs[1] ){
//            array.push_back(stats.element[i]);
//            temp = stats.element[i].output;
//        }
//    }

    //Вывод элементов после 1 версии сортировки
//    std::cout << "----------------------------------" << std::endl;
//    for(int i = 0; i<array.size(); ++i){
//        for (int j = 0; j<array[i].inputs.size(); ++j){
//            std::cout << array[i].inputs[j] << " ";
//        }
//        std::cout << " || " << array[i].type << " || ";
//        std::cout << array[i].output << std::endl;
//    }
//    std::cout << "----------------------------------" << std::endl;


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


//class bench_pretty_printer : public bench_reader
//{
//public:
//    /*! \brief Constructor of the BENCH pretty printer.
//     *
//     * \param os Output stream
//     */
//    bench_pretty_printer( std::ostream& os = std::cout )
//            : _os( os )
//    {
//    }
//
//    virtual void on_input( const std::string& name ) const override
//    {
//        _os << fmt::format( "INPUT({0})", name ) << std::endl;
//    }
//
//    virtual void on_output( const std::string& name ) const override
//    {
//        _os << fmt::format( "OUTPUT({0})", name ) << std::endl;
//    }
//
//    virtual void on_gate( const std::vector<std::string>& inputs, const std::string& output, const std::string& type ) const override
//    {
//        _os << fmt::format( "{0} = {1}({2})", output, type, detail::join( inputs, "," ) ) << std::endl;
//    }
//
//    virtual void on_assign( const std::string& input, const std::string& output ) const override
//    {
//        _os << fmt::format( "{0} = {1}", output, input ) << std::endl;
//    }
//
//    std::ostream& _os; /*!< Output stream */
//}; /* bench_pretty_printer */

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
//#if defined linux && SDL_VERSION_ATLEAST(2, 0, 8)
//    // Disable compositor bypass
//    if(!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0"))
//    {
//        printf("SDL can not disable compositor bypass!\n");
//        return 0;
//    }
//#endif
//    // Initialize SDL2
//    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
//        printf("SDL2 could not be initialized!\n"
//               "SDL2 Error: %s\n", SDL_GetError());
//        return 0;
//    }
//    // Initialize SDL2_ttf
//    TTF_Init();

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
    for(int i = 0; i<stats.element.size(); ++i){
        for (int j = 0; j<stats.element[i].inputs.size(); ++j){
            std::cout << stats.element[i].inputs[j] << " ";
        }
        std::cout << " || " << stats.element[i].type << " || ";
        std::cout << stats.element[i].output << std::endl;
    }


    std::cout << "----------------------------------------" << std::endl;

    std::vector <std::vector<parametrs>> matrix;

    sortGraph(stats, matrix);

    //  вывод полученной матрицы
    for (int i = 0; i < matrix.size(); ++i ){
        for (int j = 0; j < matrix[i].size(); ++j ){
            for (int k = 0; k < matrix[i][j].inputs.size(); ++k)
                std::cout << matrix[i][j].inputs[k] << "  ";
            std::cout << matrix[i][j].type << "  " << matrix[i][j].output << " || ";
        }
        std::cout << std::endl;
    }



//    const auto &gate_lines = reader.gate_lines;
//    for (int i=0; i<stats.number_of_lines; ++i){
//        std::cout << "------------------------line" << i << "------------------------" << std::endl;
//        std::cout << "left -- " << std::get<1>(gate_lines[i])[0] << std::get<1>(gate_lines[i])[1] << std::get<1>(gate_lines[i])[2]<< std::endl;
//        std::cout << "type -- " << std::get<2>(gate_lines[i])[0] << std::get<2>(gate_lines[i])[1] << std::get<2>(gate_lines[i])[2]<< std::endl;
//        std::cout << "right -- " << std::get<0>(gate_lines[i])[0]<< " " << std::get<0>(gate_lines[i])[1] << std::endl;
//    }




//    SDL_Rect inputs[stats.number_of_inputs];
//    SDL_Rect outputs[stats.number_of_inputs];
//    SDL_Rect ddfs[stats.number_of_dffs];
//    SDL_Rect lines[stats.number_of_lines];
//
//    // Create window
//    SDL_Window *window = SDL_CreateWindow("SDL2_ttf sample",
//                                          SDL_WINDOWPOS_UNDEFINED,
//                                          SDL_WINDOWPOS_UNDEFINED,
//                                          SCREEN_WIDTH, SCREEN_HEIGHT,
//                                          SDL_WINDOW_SHOWN);
//    if (!window) {
//        printf("Window could not be created!\n"
//               "SDL_Error: %s\n", SDL_GetError());
//    } else {
//        // Create renderer
//        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
//        if (!renderer) {
//            printf("Renderer could not be created!\n"
//                   "SDL_Error: %s\n", SDL_GetError());
//        } else {
//
//            for (int i = 0; i < stats.number_of_inputs; ++i) {
//                // Dimensions of the inputs
//                inputs[i].w = 60;
//                inputs[i].h = 40;
//                // Location of the rect
//                inputs[i].x = 10;
//                inputs[i].y = SCREEN_HEIGHT/stats.number_of_inputs - inputs[i].h*stats.number_of_inputs/2 + (inputs[i].h + 10) * i;
//
//
//                SDL_Color textColor = {0x00, 0x00, 0x00, 0xFF};
//                SDL_Color textBackgroundColor = {0xFF, 0xFF, 0xFF, 0xFF};
//
//            }
//
//            for (int i = 0; i < stats.number_of_outputs; ++i) {
//                // Dimensions of the outputs
//                outputs[i].w = 60;
//                outputs[i].h = 40;
//                // Location of the rect
//                outputs[i].x = SCREEN_WIDTH-outputs[i].w - 10;
////                outputs[i].y = 50 + (outputs[i].h + 10) * i;
//                outputs[i].y = SCREEN_HEIGHT/stats.number_of_inputs - outputs[i].h*stats.number_of_inputs/2 + (outputs[i].h + 10) * i;
//
//                SDL_Color textColor = {0x00, 0x00, 0x00, 0xFF};
//                SDL_Color textBackgroundColor = {0xFF, 0xFF, 0xFF, 0xFF};
//
//            }
//
//            for (int i = 0; i < stats.number_of_dffs; ++i) {
//                // Dimensions of the inputs
//                ddfs[i].w = 60;
//                ddfs[i].h = 40;
//                // Location of the rect
//                ddfs[i].x = SCREEN_WIDTH/2;
//                ddfs[i].y = SCREEN_HEIGHT/stats.number_of_dffs - ddfs[i].h*stats.number_of_dffs/2 + (ddfs[i].h + 10) * i;
//
//
//                SDL_Color textColor = {0x00, 0x00, 0x00, 0xFF};
//                SDL_Color textBackgroundColor = {0xFF, 0xFF, 0xFF, 0xFF};
//
//            }
//
//
//            // Event loop exit flag
//            bool quit = false;
//
//            // Event loop
//            while (!quit) {
//                SDL_Event e;
//
//                // Wait indefinitely for the next available event
//                SDL_WaitEvent(&e);
//
//                // User requests quit
//                if (e.type == SDL_QUIT) {
//                    quit = true;
//                }
//
//                // Initialize renderer color white for the background
//                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
//
//                // Clear screen
//                SDL_RenderClear(renderer);
//
//                // Set renderer color red to draw the square
//                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);
//
//                // Draw filled square
//                for (int i = 0; i < stats.number_of_inputs; ++i) {
//                    SDL_RenderFillRect(renderer, &inputs[i]);
//                }
//                for (int i = 0; i < stats.number_of_outputs; ++i) {
//                    SDL_RenderFillRect(renderer, &outputs[i]);
//                }
//                for (int i = 0; i < stats.number_of_dffs; ++i) {
//                    SDL_RenderFillRect(renderer, &ddfs[i]);
//                }
//                // Update screen
//                SDL_RenderPresent(renderer);
//            }
//
//            // Destroy renderer
//            SDL_DestroyRenderer(renderer);
//
//            // Destroy window
//            SDL_DestroyWindow(window);
//        }
//
//        // Quit SDL2_ttf
//        TTF_Quit();
//
//        // Quit SDL
//        SDL_Quit();
//
//    }
        return 0;
}