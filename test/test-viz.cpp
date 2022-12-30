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

        scheme.push_back(input);
    }

    void add_output(const std::string& input) {
        Vertex output;
        output.type = "OUT";
        output.name = std::string("output" + std::to_string(outputNum));

        outputNum++;

        queue.emplace_back(output, input);

        scheme.push_back(output);
    }

    void add_dff(const std::string& input, const std::string& output) {
        Vertex dff;

        dff.type = "DFF";
        dff.name = output;

        if(!connection_search(dff, input)) queue.emplace_back(dff, input);

        scheme.push_back(dff);
    }

    void add_gate(std::vector<std::string> inputs, const std::string& output, const std::string& type) {
        Vertex gate;
        gate.type = type;
        gate.name = output;

        for(int i = 0; i < inputs.size(); i++) {
            if(connection_search(gate, inputs[i])) inputs.erase(inputs.begin() + i);
        }

        for(std::string input : inputs) {
            queue.emplace_back(gate, input);
        }

        scheme.push_back(gate);
    }

    //Method to check whether demanded input exists or not and connect it if possible
    bool connection_search(Vertex& element, const std::string& input) {
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
    void queue_handling() {
        while(!queue.empty()) {
            for(int i = 0; i < scheme.size(); i++) {
                for(int j = 0; j < queue.size(); j++) {
                    if(scheme[i].name == std::get<0>(queue[j]).name) {
                        if(connection_search(scheme[i], std::get<1>(queue[j]))) {
                            queue.erase(queue.begin() + j);
                            j--;
                            continue;
                        }
                    }
                }
            }
        }
    }

    void output(std::ostream& out) {
        for(Vertex& vert : scheme) {
            out << vert.type << ' ' << vert.name << ' ' << vert.layer << " INS: ";
            for(auto ins : vert.ins) {
                out << scheme[ins].name << ' ';
            }
            std::cout << std::endl;
        }
        std::cout << "\n====QUEUE===\n";
        for(auto &requests : queue) {
            out << std::get<0>(requests).type;
        }
    }

    std::vector<Vertex> scheme;
    std::vector<std::tuple<Vertex, std::string>> queue;
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
    for(int& input : element->ins) {
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
    logicScheme.output(std::cout);

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
    for(Vertex& element : logicScheme.scheme) {
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
    for(std::vector<Vertex*>& elements : elements_by_layers) {
        if(elements.size() > maxElems) {
            maxElems = elements.size();
        }
    }

    const float rectHeight = screen_height/(VSPACING*maxElems);
    const float rectWidth = screen_width/(HSPACING*layersNum);

    for(std::vector<Vertex*>& elements : elements_by_layers) {
        setPosition(elements, maxElems, rectHeight, rectWidth, screen_height);
    }

    std::vector<Line> connection_lines;
    for(std::vector<Vertex*>& elements : elements_by_layers) {
        for(Vertex* element : elements) {
            setConnections(element, logicScheme.scheme, connection_lines);
        }
    }

    SDL_SetRenderDrawColor(renderer, 0,0,0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255,255,255, SDL_ALPHA_OPAQUE);

    //Placing elements on the screen
    for(Vertex& element : logicScheme.scheme) {
        SDL_RenderDrawRectF(renderer, &element.position);
    }

    //Placing connection lines on the screen
    for(Line& line : connection_lines) {
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