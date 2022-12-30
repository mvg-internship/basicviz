#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <variant>

#include <lorina/bench.hpp>

#define SDL_MAIN_HANDLED

#include <SDL.h>

enum SPACING {
    HSPACING = 2,
    VSPACING = 2
};

struct Vertex {
    std::string type;
    std::string name;
    int layer;
    std::vector<int> ins;
    SDL_FRect position;
};

struct Line {
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
        Vertex input;
        input.type = "IN";
        input.name = name;
        input.layer = 0;

        scheme.emplace_back(input);
    }

    void add_output(const std::string& input) {
        Vertex output;
        output.type = "OUT";
        output.name = std::string("output" + std::to_string(outputNum));

        outputNum++;

        std::vector<std::string> ins;
        ins.emplace_back(input);

        queue.emplace_back(output, ins);

        scheme.emplace_back(output);
    }

    void add_dff(const std::string& input, const std::string& output) {
        Vertex dff;

        dff.type = "DFF";
        dff.name = output;

        std::vector<std::string> ins;
        ins.emplace_back(input);

        if(connection_search(dff, ins)) queue.emplace_back(dff, ins);

        scheme.emplace_back(dff);
    }

    void add_gate(std::vector<std::string> inputs, const std::string& output, const std::string& type) {
        Vertex gate;
        gate.type = type;
        gate.name = output;

        if(connection_search(gate, inputs)) queue.emplace_back(gate, inputs);

        scheme.emplace_back(gate);
    }

    //Method to check whether demanded input exists or not and connect it if possible
    bool connection_search(Vertex& element, std::vector<std::string>& ins) {
        for(int j = 0; j < scheme.size(); j++) {
            for(int i = 0; i < ins.size(); i++) {
                if(ins[i] == scheme[j].name) {
                    element.ins.emplace_back(j);
                    element.layer = scheme[j].layer + 1;
                    ins.erase(ins.begin() + i);
                    i--;
                }
            }
        }

        if(ins.empty()) return false;
        return true;
    }

    //Method to handle elements with lacking bonds
    void queue_handling() {
        while(!queue.empty()) {
            for (int i = 0; i < queue.size(); i++) {
                //Going through every element of the scheme to find ones that are in the queue and connecting them to the inputs
                for (int j = 0; j < scheme.size(); j++) {
                    if (std::get<0>(queue[i]).name == scheme[j].name) {
                        if (!connection_search(scheme[j], std::get<1>(queue[i]))) {
                            queue.erase(queue.begin() + i);
                            i--;
                        }
                    }
                }
            }
        }
    }

    void output() {
        for(auto vert : scheme) {
            std::cout << vert.type << ' ' << vert.name << ' ' << vert.layer << " INS: ";
            for(auto ins : vert.ins) {
                std::cout << scheme[ins].name << ' ';
            }
            std::cout << std::endl;
        }
        std::cout << "\n============\n";
        for(auto requests : queue) {
            std::cout << std::get<0>(requests).type;
        }
    }

    std::vector<Vertex> scheme;
    std::vector< std::tuple< Vertex, std::vector< std::string > > > queue;
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

void setPosition(std::vector<Vertex*> elements, const int max_elements, const float height, const float width, const int screen_height) {
    int count = 0;
    int freeSpaces = elements.size() + 1;
    float step = (screen_height - height * elements.size()) / freeSpaces;
    for(Vertex* elem : elements) {
        elem->position.x = elem->layer * HSPACING * width;
        elem->position.h = height;
        elem->position.w = width;

        if (elements.size() == max_elements) {
            elem->position.y = count * VSPACING * height;
        }
        else {
            elem->position.y = step * (count + 1) + height * count;
        }

        count++;
    }
}

void setConnections(Vertex* element, const std::vector<Vertex>& scheme, std::vector<Line>& connections) {
    for(int input : element->ins) {
        std::cout << input << ' ';
        Line connect;
        connect.x1 = scheme[input].position.x + scheme[input].position.w;
        connect.y1 = scheme[input].position.y + scheme[input].position.h/2;
        connect.x2 = element->position.x;
        connect.y2 = element->position.y + element->position.h/2;

        connections.push_back(connect);
    }
}

int main(int argc, char* argv[]) {

    if(argc != 2) return -1;

    std::string filename(argv[1]);

    logic_scheme logicScheme;

    bench_parser parser(logicScheme);
    auto result = lorina::read_bench(filename, parser);
    if(result != lorina::return_code::success) return -2;

    logicScheme.queue_handling();
    logicScheme.output();

    SDL_DisplayMode displayMode;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    std::vector<Line> connections;

    if(SDL_Init(SDL_INIT_VIDEO) < 0) return -3;

    SDL_GetCurrentDisplayMode(0, &displayMode);
    const int screen_height = displayMode.h;
    const int screen_width = displayMode.w;

    window = SDL_CreateWindow("test-viz", 0, 0, screen_width, screen_height, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    //Finding total number of layers in element with max layer
    int layersNum = 0;
    for(auto element : logicScheme.scheme) {
        if(element.layer > layersNum) layersNum = element.layer;
    }
    layersNum++;

    //Sorting elements in order by layers
    std::vector<std::vector<Vertex*>> elements_by_layers(layersNum);
    for(int i = 0; i < logicScheme.scheme.size(); i++) {
        elements_by_layers[logicScheme.scheme[i].layer].push_back(&logicScheme.scheme[i]);
    }

    //Finding the number of elements in layer with the most elements
    int maxElems = 0;
    for(std::vector<Vertex*> elements : elements_by_layers) {
        if(elements.size() > maxElems) {
            maxElems = elements.size();
        }
    }

    const float rectHeight = screen_height/(VSPACING*maxElems);
    const float rectWidth = screen_width/(HSPACING*layersNum);

    std::cout << rectWidth << ' ' << rectHeight;

    for(std::vector<Vertex*> elements : elements_by_layers) {
        setPosition(elements, maxElems, rectHeight, rectWidth, screen_height);
    }

    std::vector<Line> connection_lines;
    for(std::vector<Vertex*> elements : elements_by_layers) {
        for(Vertex* element : elements) {
            setConnections(element, logicScheme.scheme, connection_lines);
        }
    }

    SDL_SetRenderDrawColor(renderer, 0,0,0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255,255,255, SDL_ALPHA_OPAQUE);

    //Placing elements on the screen
    for(Vertex element : logicScheme.scheme) {
        SDL_RenderDrawRectF(renderer, &element.position);
    }

    //Placing connection lines on the screen
    for(auto line : connection_lines) {
        SDL_RenderDrawLineF(renderer, line.x1, line.y1, line.x2, line.y2);
    }

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