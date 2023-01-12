#include <iostream>
#include <vector>
#include <string>

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

struct pending_connection {
    vertex elementToBind;
    std::string input;
};

class line {
public:
    static line right_to_left_line(const SDL_FRect &src, const SDL_FRect &dst) {
        float x1, y1, x2, y2;
        get_right(src, x1, y1);
        get_left(dst, x2, y2);
        return { x1, y1, x2, y2 };
    }

    static void get_right(const SDL_FRect &rect, float &x, float &y) {
        x = rect.x + rect.w;
        y = rect.y + rect.h / 2;
    }

    static void get_left(const SDL_FRect &rect, float &x, float &y) {
        x = rect.x;
        y = rect.y + rect.h / 2;
    }

    float x1;
    float y1;
    float x2;
    float y2;
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

        push_back_queue(output, input);

        scheme.push_back(output);
    }

    void add_dff(const std::string& input, const std::string& output) {
        vertex dff;

        dff.type = "DFF";
        dff.name = output;

        if(!connect(dff, input)) push_back_queue(dff, input);

        scheme.push_back(dff);
    }

    void add_gate(const std::vector<std::string>& inputs, const std::string& output, const std::string& type) {
        vertex gate;
        gate.type = type;
        gate.name = output;

        for(int i = 0; i < inputs.size(); i++) {
            if(!connect(gate, inputs[i])) push_back_queue(gate, inputs[i]);
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
                for(auto j = queue.begin(); j != queue.end(); j++) {
                    if(scheme[i].name == j->elementToBind.name) {
                        if(connect(scheme[i], j->input)) {
                            j = queue.erase(j);
                            j--;
                        }
                    }
                }
            }
        }
    }

    void push_back_queue(const vertex& elementToBind, const std::string& input) {
        pending_connection queued_element;
        queued_element.elementToBind = elementToBind;
        queued_element.input = input;
        queue.push_back(queued_element);
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
        for(auto &queued_element : queue) {
            out << queued_element.elementToBind.type << ' ' << queued_element.elementToBind.name << "IN: " << queued_element.input << std::endl;
        }
    }

    std::vector<vertex> scheme;
    std::list<pending_connection> queue;
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

void setRectangle(std::vector<vertex*>& elements, const unsigned int max_elements, const float height, const float width, const int screen_height) {
    int count = 0;
    unsigned int freeSpaces = elements.size() + 1;
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
        line connect(line::right_to_left_line(scheme[input].rectangle, element.rectangle));
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

bool parseInput(const char* arg, logic_scheme& logicScheme) {
    std::string filename(arg);

    bench_parser parser(logicScheme);
    auto result = lorina::read_bench(filename, parser);
    if(result == lorina::return_code::parse_error) return false;

    logicScheme.find_and_connect_overlaps();
    logicScheme.print(std::cout);

    return true;
}

std::vector<line> prepareDrawData(logic_scheme& logicScheme, const int screen_h, const int screen_w) {
    //Finding number of layers by finding the max layer number
    int layersNum = 0;
    for(const vertex& element : logicScheme.scheme) {
        if(element.layer > layersNum) layersNum = element.layer;
    }
    layersNum++;

    //Creating vector with pre-allocated space and filling it layerNum-wise
    std::vector<std::vector<vertex*>> elementsByLayers(layersNum);
    for(auto & i : logicScheme.scheme) {
        elementsByLayers[i.layer].push_back(&i);
    }

    //Finding the number of elements in layer with the most elements
    unsigned int maxElems = 0;
    for(std::vector<vertex*>& elements : elementsByLayers) {
        if(elements.size() > maxElems) {
            maxElems = elements.size();
        }
    }

    const float rectHeight = screen_h/(VSPACING*maxElems);
    const float rectWidth = screen_w/(HSPACING*layersNum);

    for(std::vector<vertex*>& elements : elementsByLayers) {
        setRectangle(elements, maxElems, rectHeight, rectWidth, screen_h);
    }

    std::vector<line> connectionLines;
    for(std::vector<vertex*>& elements : elementsByLayers) {
        for(vertex* element : elements) {
            setConnections(*element, logicScheme.scheme, connectionLines);
        }
    }

    return connectionLines;
}

int main(int argc, char* argv[]) {

    //Parsing bench file
    if(argc < 2) {
        std::cerr << "Filename was not provided\n";
        return NO_FILENAME_PROVIDED;
    }

    logic_scheme logicScheme;

    if(!parseInput(argv[1], logicScheme)) {
        std::cerr << "Parser failure\n";
        return PARSER_FAILURE;
    }

    //SDL initialization
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not be initialized\n";
        return SDL_INIT_FAILURE;
    }

    SDL_DisplayMode displayInfo;
    SDL_GetCurrentDisplayMode(0, &displayInfo);

    SDL_Window* window = SDL_CreateWindow("test-viz", 0, 0, displayInfo.w, displayInfo.h, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    //Prepare draw data and draw
    drawBackground(renderer);
    drawElementsAndConnections(renderer, logicScheme, prepareDrawData(logicScheme, displayInfo.h, displayInfo.w));
    SDL_RenderPresent(renderer);

    //Event loop
    bool isRunning = true;
    while(isRunning) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) isRunning = false;
        }
    }

    //Shutdown
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}