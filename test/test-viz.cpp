#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <variant>

#include <lorina/bench.hpp>

#define SDL_MAIN_HANDLED

#include <SDL.h>

enum {
    HSPACING = 2,
    VSPACING = 2
};

enum ErrorCode {
    NO_FILENAME_PROVIDED = 1,
    PARSER_FAILURE,
    SDL_INIT_FAILURE
};

struct vertex {
    std::string type;
    std::string name;
    int layer;
    std::vector<int> ins;
    SDL_FRect rectangle;
};

class line {
public:
    static line rightToLeftLine(const SDL_FRect &src, const SDL_FRect &dst) {
        double x1, y1, x2, y2;
        getRight(src, x1, y1);
        getLeft(dst, x2, y2);
        return { x1, y1, x2, y2 };
    }

    static void getRight(const SDL_FRect &rect, double &x, double &y) {
        x = rect.x + rect.w;
        y = rect.y + rect.h / 2;
    }

    static void getLeft(const SDL_FRect &rect, double &x, double &y) {
        x = rect.x;
        y = rect.y + rect.h / 2;
    }

    double x1;
    double y1;
    double x2;
    double y2;
};

class logic_scheme {
public:

    logic_scheme() {
        outputNum = 0;
    }

    void add_input(const std::string& name) {
        vertex input;
        input.type = "IN";
        input.name = name;
        input.layer = 0;

        scheme.push_back(input);
    }

    void add_output(const std::string& input) {
        vertex output;
        output.type = "OUT";
        output.name = std::string("output" + std::to_string(outputNum));

        outputNum++;

        queue.emplace_back(output, input);

        scheme.push_back(output);
    }

    void add_dff(const std::string& input, const std::string& output) {
        vertex dff;

        dff.type = "DFF";
        dff.name = output;

        if(!connect(dff, input)) queue.emplace_back(dff, input);

        scheme.push_back(dff);
    }

    void add_gate(std::vector<std::string> inputs, const std::string& output, const std::string& type) {
        vertex gate;
        gate.type = type;
        gate.name = output;

        for(int i = 0; i < inputs.size(); i++) {
            if(connect(gate, inputs[i])) inputs.erase(inputs.begin() + i);
        }

        for(std::string input : inputs) {
            queue.emplace_back(gate, input);
        }

        scheme.push_back(gate);
    }

    //Method to check whether demanded input exists or not and connect it if possible
    bool connect(vertex& element, const std::string& input) {
        for(int j = 0; j < scheme.size(); j++) {
            if(scheme[j].name == input) {
                element.ins.push_back(j);
                element.layer = scheme[j].layer + 1;
                return true;
            }
        }
        return false;
    }

    //Method to search where scheme overlaps queue and to search for connection in those elements
    void find_and_connect_overlaps() {
        while(!queue.empty()) {
            for(int i = 0; i < scheme.size(); i++) {
                for(int j = 0; j < queue.size(); j++) {
                    if(scheme[i].name == std::get<0>(queue[j]).name) {
                        if(connect(scheme[i], std::get<1>(queue[j]))) {
                            queue.erase(queue.begin() + j);
                            j--;
                            continue;
                        }
                    }
                }
            }
        }
    }

    void print(std::ostream& out) {
        for(vertex& vert : scheme) {
            out << vert.type << ' ' << vert.name << ' ' << vert.layer << " INS: ";
            for(auto ins : vert.ins) {
                out << scheme[ins].name << ' ';
            }
            out << std::endl;
        }
        out << "\n====QUEUE===\n";
        for(auto &requests : queue) {
            out << std::get<0>(requests).type;
        }
    }

    std::vector<vertex> scheme;
    std::vector<std::tuple<vertex, std::string>> queue;
    int outputNum;
};

class bench_parser : public lorina::bench_reader {
public:
    explicit bench_parser(logic_scheme& logicScheme) : scheme(logicScheme) {}

    virtual void on_input(const std::string& name) const override {
        scheme.add_input(name);
    }

    virtual void on_output(const std::string& name) const override {
        scheme.add_output(name);
    }

    virtual void on_dff(const std::string &input, const std::string &output) const override {
        scheme.add_dff(input, output);
    }

    virtual void on_gate(const std::vector<std::string>& inputs, const std::string& output, const std::string& type) const override {
        scheme.add_gate(inputs, output, type);
    }

    logic_scheme& scheme;
};

void setRectangle(std::vector<vertex*>& elements, const int max_elements, const float height, const float width, const int screen_height) {
    int count = 0;
    int freeSpaces = elements.size() + 1;
    float step = (screen_height - height * elements.size()) / freeSpaces;
    for(vertex* elem : elements) {
        elem->rectangle.x = elem->layer * HSPACING * width;
        elem->rectangle.h = height;
        elem->rectangle.w = width;

        if (elements.size() == max_elements) {
            elem->rectangle.y = count * VSPACING * height;
        }
        else {
            elem->rectangle.y = step * (count + 1) + height * count;
        }

        count++;
    }
}

void setConnections(const vertex& element, const std::vector<vertex>& scheme, std::vector<line>& connections) {
    for(int input : element.ins) {
        line connect(line::rightToLeftLine(scheme[input].rectangle, element.rectangle));
        connections.push_back(connect);
    }
}

void drawElementsAndConnections(SDL_Renderer* renderer, const logic_scheme& elements, const std::vector<line>& connections) {
    SDL_SetRenderDrawColor(renderer, 255,255,255, SDL_ALPHA_OPAQUE);

    //Placing elements on the screen
    for(const vertex& element : elements.scheme) {
        SDL_RenderDrawRectF(renderer, &element.rectangle);
    }

    //Placing connection lines on the screen
    for(const line& connection_line : connections) {
        SDL_RenderDrawLineF(renderer, connection_line.x1, connection_line.y1, connection_line.x2, connection_line.y2);
    }
}

void drawBackground(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0,0,0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
}

int main(int argc, char* argv[]) {

    if(argc < 2) {
        std::cerr << "Filename was not provided\n";
        return NO_FILENAME_PROVIDED;
    }

    std::string filename(argv[1]);

    logic_scheme logicScheme;

    bench_parser parser(logicScheme);
    auto result = lorina::read_bench(filename, parser);
    if(result == lorina::return_code::parse_error) {
        std::cerr << "Parser failure\n";
        return PARSER_FAILURE;
    }

    logicScheme.find_and_connect_overlaps();
    logicScheme.print(std::cout);

    SDL_DisplayMode displayMode;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    std::vector<line> connections;

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not be initialized";
        return SDL_INIT_FAILURE;
    }

    SDL_GetCurrentDisplayMode(0, &displayMode);
    const int screen_height = displayMode.h;
    const int screen_width = displayMode.w;

    window = SDL_CreateWindow("test-viz", 0, 0, screen_width, screen_height, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    //Finding total number of layers in element with max layer
    int layersNum = 0;
    for(vertex& element : logicScheme.scheme) {
        if(element.layer > layersNum) layersNum = element.layer;
    }
    layersNum++;

    //Sorting elements in order by layers
    std::vector<std::vector<vertex*>> elements_by_layers(layersNum);
    for(int i = 0; i < logicScheme.scheme.size(); i++) {
        elements_by_layers[logicScheme.scheme[i].layer].push_back(&logicScheme.scheme[i]);
    }

    //Finding the number of elements in layer with the most elements
    int maxElems = 0;
    for(std::vector<vertex*>& elements : elements_by_layers) {
        if(elements.size() > maxElems) {
            maxElems = elements.size();
        }
    }

    const float rectHeight = screen_height/(VSPACING*maxElems);
    const float rectWidth = screen_width/(HSPACING*layersNum);

    for(std::vector<vertex*>& elements : elements_by_layers) {
        setRectangle(elements, maxElems, rectHeight, rectWidth, screen_height);
    }

    std::vector<line> connection_lines;
    for(std::vector<vertex*>& elements : elements_by_layers) {
        for(vertex* element : elements) {
            setConnections(*element, logicScheme.scheme, connection_lines);
        }
    }

    drawBackground(renderer);
    drawElementsAndConnections(renderer, logicScheme, connection_lines);

    SDL_RenderPresent(renderer);

    bool isRunning = true;
    while(isRunning) {
        SDL_Event event;

        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) isRunning = false;
        }
    }
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}