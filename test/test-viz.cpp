#define SDL_MAIN_HANDLED

//Standard libs includes
#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <variant>

//External libs includes
#include <lorina/bench.hpp>
#include <SDL.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

#define HOR_SPACING 2
#define VER_SPACING 2

struct Vertex {
    std::string type;
    std::string name;
    int layer;
    std::vector<Vertex> ins;
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
        _outputNum = 0;
    }

    void add_input(const std::string& _name) {
        Vertex input;
        input.type = "IN";
        input.name = _name;
        input.layer = 0;

        _scheme.emplace_back(input);
    }

    void add_output(const std::string& _input) {
        Vertex output;
        output.type = "OUT";
        output.name = std::string("output" + std::to_string(_outputNum));

        _outputNum++;

        std::vector<std::string> ins;
        ins.emplace_back(_input);

        _queue.emplace_back(output, ins);

        _scheme.emplace_back(output);
    }

    void add_dff(const std::string& _input, const std::string& _output) {
        Vertex dff;

        dff.type = "DFF";
        dff.name = _output;

        std::vector<std::string> ins;
        ins.emplace_back(_input);

        if(check_inputs(dff, ins)) _queue.emplace_back(dff, ins);

        _scheme.emplace_back(dff);
    }

    void add_gate(const std::vector<std::string>& _inputs, const std::string& _output, const std::string& _type) {
        Vertex gate;
        gate.type = _type;
        gate.name = _output;
        std::vector<std::string> ins = _inputs;

        if(check_inputs(gate, ins)) _queue.emplace_back(gate, ins);

        _scheme.emplace_back(gate);
    }

    //Method to check whether demanded input exists or not
    bool check_inputs(Vertex& element, std::vector<std::string>& ins) {
        for(Vertex& suggested_element : _scheme) {
            for(int i = 0; i < ins.size(); i++) {
                if(ins[i] == suggested_element.name) {
                    element.ins.emplace_back(suggested_element);
                    element.layer = suggested_element.layer + 1;
                    ins.erase(ins.begin() + i);
                    i--;
                }
            }
        }

        if(ins.empty()) return false;
        return true;
    }

    //Method to handle elements with lacking bonds
    void request_handling() {
        while(!_queue.empty()) {
            for (int i = 0; i < _queue.size(); i++) {
                //Going through every element of the scheme to find ones that are in the queue and connecting them to the inputs
                for (int j = 0; j < _scheme.size(); j++) {
                    if (std::get<0>(_queue[i]).name == _scheme[j].name) {
                        if (!check_inputs(_scheme[j], std::get<1>(_queue[i]))) {
                            _queue.erase(_queue.begin() + i);
                            i--;
                        }
                    }
                }
            }
        }
    }

    void output() {
        for(auto vert : _scheme) {
            std::cout << vert.type << ' ' << vert.name << ' ' << vert.layer << " INS: ";
            for(auto ins : vert.ins) {
                std::cout << ins.name << ' ';
            }
            std::cout << std::endl;
        }
        std::cout << "\n============\n";
        for(auto requests : _queue) {
            std::cout << std::get<0>(requests).type;
        }
    }

    std::vector<Vertex> _scheme;
    std::vector< std::tuple< Vertex, std::vector< std::string > > > _queue;
    int _outputNum;
};

class bench_parser : public lorina::bench_reader {
public:
    explicit bench_parser(logic_scheme& logicScheme) : _scheme(logicScheme) {}

    virtual void on_input(const std::string& name) const override {
        _scheme.add_input(name);
    }

    virtual void on_output(const std::string& name) const override {
        _scheme.add_output(name);
    }

    virtual void on_dff(const std::string &input, const std::string &output) const override {
        _scheme.add_dff(input, output);
    }

    virtual void on_gate(const std::vector<std::string>& inputs, const std::string& output, const std::string& type) const override {
        _scheme.add_gate(inputs, output, type);
    }

    logic_scheme& _scheme;
};

void buildRectangles(std::vector<Vertex>& elems, std::vector<SDL_FRect>& rects, std::vector<Line>& connections, int maxElem, double height, double width) {
    if(elems.size() == maxElem) {
        int count = 0;
        for(auto elem : elems) {
            SDL_FRect rect;
            rect.x = elem.layer * 2 * width;
            rect.y = count * 2 * height;
            rect.h = height;
            rect.w = width;
            count++;
            rects.emplace_back(rect);

            for(auto input : elem.ins) {
                Line connection;
                connection.x2 = rect.x;
                connection.y2 = rect.y + rect.h/2;
                connection.x1 = input.layer * 2 * width + width;
                connection.y1 = 0;
                connections.emplace_back(connection);
            }
        }
    }

    else {
        int count = 0;
        int freeSpaces = elems.size() + 1;
        double step = (SCREEN_HEIGHT - height * elems.size()) / freeSpaces;
        for(auto elem : elems) {
            SDL_FRect rect;
            rect.x = elem.layer * 2 * width;
            rect.y = step * (count + 1) + height * count;
            rect.h = height;
            rect.w = width;
            count++;
            rects.emplace_back(rect);

            for(auto input : elem.ins) {
                Line connection;
                connection.x2 = rect.x;
                connection.y2 = rect.y + rect.h/2;
                connection.x1 = input.layer * 2 * width + width;
                connection.y1 = 0;
                connections.emplace_back(connection);
            }
        }
    }


}

void setPosition(std::vector<Vertex>& elements, const int max_elements, const float height, const float width) {
    if(elements.size() == max_elements) {
        int count = 0;
        for(Vertex& elem : elements) {
            elem.position.x = elem.layer * HOR_SPACING * width;
            elem.position.y = count * VER_SPACING * height;
            elem.position.h = height;
            elem.position.w = width;
            count++;
        }
    }

    else {
        int count = 0;
        int freeSpaces = elements.size() + 1;
        float step = (SCREEN_HEIGHT - height * elements.size()) / freeSpaces;
        for(Vertex& elem : elements) {
            elem.position.x = elem.layer * HOR_SPACING * width;
            elem.position.y = step * (count + 1) + height * count;
            elem.position.h = height;
            elem.position.w = width;
            count++;
        }
    }
}

void setConnections(Vertex& element, const std::vector<std::vector<Vertex>> elements, std::vector<Line>& connections) {
    for(auto input : element.ins) {
        for(auto layered_elements : elements) {
            for(auto compared_element : layered_elements) {
                if(input.name == compared_element.name) {
                    Line connect;
                    connect.x1 = compared_element.position.x + compared_element.position.w;
                    connect.y1 = compared_element.position.y + compared_element.position.h/2;
                    connect.x2 = element.position.x;
                    connect.y2 = element.position.y + element.position.h/2;
                    connections.emplace_back(connect);
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {

    if(argc != 2) return -1;

    std::string filename(argv[1]);

    logic_scheme logicScheme;

    bench_parser parser(logicScheme);
    auto result = lorina::read_bench(filename, parser);
    if(result != lorina::return_code::success) return -2;

    //Clearing the queue
    logicScheme.request_handling();
    logicScheme.output();

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    std::vector<Line> connections;
    if(SDL_Init(SDL_INIT_VIDEO) < 0) return -3;
    window = SDL_CreateWindow("test-viz", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    //Finding total number of layers in element with max layer
    int layersNum = 0;
    for(auto element : logicScheme._scheme) {
        if(element.layer > layersNum) layersNum = element.layer;
    }
    layersNum++;

    //Sorting elements in order by layers
    std::vector<std::vector<Vertex>> elements_by_layers(layersNum);
    for(Vertex& element : logicScheme._scheme) {
        elements_by_layers[element.layer].emplace_back(element);
    }

    //Finding the number of elements in layer with the most elements
    int maxElems = 0;
    for(auto elements : elements_by_layers) {
        if(elements.size() > maxElems) {
            maxElems = elements.size();
        }
    }

    const float rectHeight = SCREEN_HEIGHT/(VER_SPACING*maxElems);
    const float rectWidth = SCREEN_WIDTH/(HOR_SPACING*layersNum);

    std::cout << rectWidth << ' ' << rectHeight;

    for(std::vector<Vertex>& elements : elements_by_layers) {
        setPosition(elements, maxElems, rectHeight, rectWidth);
    }

    std::vector<Line> connection_lines;
    for(std::vector<Vertex>& elements : elements_by_layers) {
        for(Vertex& element : elements) {
            setConnections(element, elements_by_layers, connection_lines);
        }
    }

    SDL_SetRenderDrawColor(renderer, 0,0,0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255,255,255, SDL_ALPHA_OPAQUE);

    //Placing elements on the screen
    for(auto elements : elements_by_layers) {
        for(auto element : elements) {
            SDL_RenderDrawRectF(renderer, &element.position);
        }
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