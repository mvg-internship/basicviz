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
    FILENAME_NOT_PROVIDED = 1,
    PARSER_FAILURE,
    SDL_INIT_FAILURE
};

struct Vertex {
    std::string type;
    std::string name;
    int layer;
    std::vector<int> ins;
    SDL_FRect rectangle;
};

struct PendingConnection {
    Vertex element_to_bind;
    std::string input;
};

class Line {
public:
    static Line rightToLeftLine(const SDL_FRect &src, const SDL_FRect &dst) {
        float x1, y1, x2, y2;
        getRight(src, x1, y1);
        getLeft(dst, x2, y2);
        return { x1, y1, x2, y2 };
    }

    static void getRight(const SDL_FRect &rect, float &x, float &y) {
        x = rect.x + rect.w;
        y = rect.y + rect.h / 2;
    }

    static void getLeft(const SDL_FRect &rect, float &x, float &y) {
        x = rect.x;
        y = rect.y + rect.h / 2;
    }

    float x1;
    float y1;
    float x2;
    float y2;
};

class LogicScheme {
public:

    LogicScheme() {
        output_num = 0;
    }

    void addInput(const std::string& name) {
        Vertex input;
        input.type = "IN";
        input.name = name;
        input.layer = 0;

        scheme.push_back(input);
    }

    void addOutput(const std::string& input) {
        Vertex output;
        output.type = "OUT";
        output.name = std::string("output" + std::to_string(output_num));

        output_num++;

        pushBackQueue(output, input);

        scheme.push_back(output);
    }

    void addDff(const std::string& input, const std::string& output) {
        Vertex dff;

        dff.type = "DFF";
        dff.name = output;

        if(!connect(dff, input)) pushBackQueue(dff, input);

        scheme.push_back(dff);
    }

    void addGate(const std::vector<std::string>& inputs, const std::string& output, const std::string& type) {
        Vertex gate;
        gate.type = type;
        gate.name = output;

        for(int i = 0; i < inputs.size(); i++) {
            if(!connect(gate, inputs[i])) pushBackQueue(gate, inputs[i]);
        }

        scheme.push_back(gate);
    }

    //Method to check whether demanded input exists or not and connect it if possible
    bool connect(Vertex& element, const std::string& input) {
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
    void connectSchemeQueueOverlaps() {
        while(!queue.empty()) {
            for(int i = 0; i < scheme.size(); i++) {
                for(auto j = queue.begin(); j != queue.end(); j++) {
                    if(scheme[i].name == j->element_to_bind.name) {
                        if(connect(scheme[i], j->input)) {
                            j = queue.erase(j);
                            j--;
                        }
                    }
                }
            }
        }
    }

    void pushBackQueue(const Vertex& element_to_bind, const std::string& input) {
        PendingConnection queued_element;
        queued_element.element_to_bind = element_to_bind;
        queued_element.input = input;
        queue.push_back(queued_element);
    }

    void print(std::ostream& out) {
        for(Vertex& vert : scheme) {
            out << vert.type << ' ' << vert.name << ' ' << vert.layer << " INS: ";
            for(auto ins : vert.ins) {
                out << scheme[ins].name << ' ';
            }
            out << std::endl;
        }
        out << "\n====QUEUE===\n";
        for(auto &queued_element : queue) {
            out << queued_element.element_to_bind.type << ' ' << queued_element.element_to_bind.name << "IN: " << queued_element.input << std::endl;
        }
    }

    std::vector<Vertex> scheme;
    std::list<PendingConnection> queue;
    int output_num;
};

class BenchParser : public lorina::bench_reader {
public:
    explicit BenchParser(LogicScheme& logic_scheme) : scheme(logic_scheme) {}

    virtual void on_input(const std::string& name) const override {
        scheme.addInput(name);
    }

    virtual void on_output(const std::string& name) const override {
        scheme.addOutput(name);
    }

    virtual void on_dff(const std::string &input, const std::string &output) const override {
        scheme.addDff(input, output);
    }

    virtual void on_gate(const std::vector<std::string>& inputs, const std::string& output, const std::string& type) const override {
        scheme.addGate(inputs, output, type);
    }

    LogicScheme& scheme;
};

void setRectangle(std::vector<Vertex*>& elements, const unsigned int max_elements, const float height, const float width, const int screen_height) {
    int count = 0;
    unsigned int free_spaces = elements.size() + 1;
    float step = (screen_height - height * elements.size()) / free_spaces;
    for(Vertex* elem : elements) {
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

void setConnections(const Vertex& element, const std::vector<Vertex>& scheme, std::vector<Line>& connections) {
    for(int input : element.ins) {
        Line connect(Line::rightToLeftLine(scheme[input].rectangle, element.rectangle));
        connections.push_back(connect);
    }
}

void drawElementsAndConnections(SDL_Renderer* renderer, const LogicScheme& elements, const std::vector<Line>& connections) {
    SDL_SetRenderDrawColor(renderer, 255,255,255, SDL_ALPHA_OPAQUE);

    //Placing elements on the screen
    for(const Vertex& element : elements.scheme) {
        SDL_RenderDrawRectF(renderer, &element.rectangle);
    }

    //Placing connection Lines on the screen
    for(const Line& connection_Line : connections) {
        SDL_RenderDrawLineF(renderer, connection_Line.x1, connection_Line.y1, connection_Line.x2, connection_Line.y2);
    }
}

void drawBackground(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0,0,0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
}

bool parseInput(const char* arg, LogicScheme& logic_scheme) {
    std::string filename(arg);

    BenchParser parser(logic_scheme);
    auto result = lorina::read_bench(filename, parser);
    if(result == lorina::return_code::parse_error) return false;

    logic_scheme.connectSchemeQueueOverlaps();
    logic_scheme.print(std::cout);

    return true;
}

std::vector<Line> prepareDrawData(LogicScheme& logic_scheme, const int screen_h, const int screen_w) {
    //Finding number of layers by finding the max layer number
    int layers_num = 0;
    for(const Vertex& element : logic_scheme.scheme) {
        if(element.layer > layers_num) layers_num = element.layer;
    }
    layers_num++;

    //Creating vector with pre-allocated space and filling it layerNum-wise
    std::vector<std::vector<Vertex*>> elementsByLayers(layers_num);
    for(auto & i : logic_scheme.scheme) {
        elementsByLayers[i.layer].push_back(&i);
    }

    //Finding the number of elements in layer with the most elements
    unsigned int max_elements = 0;
    for(std::vector<Vertex*>& elements : elementsByLayers) {
        if(elements.size() > max_elements) {
            max_elements = elements.size();
        }
    }

    const float rect_height = screen_h/(VSPACING*max_elements);
    const float rect_width = screen_w/(HSPACING*layers_num);

    for(std::vector<Vertex*>& elements : elementsByLayers) {
        setRectangle(elements, max_elements, rect_height, rect_width, screen_h);
    }

    std::vector<Line> connection_lines;
    for(std::vector<Vertex*>& elements : elementsByLayers) {
        for(Vertex* element : elements) {
            setConnections(*element, logic_scheme.scheme, connection_lines);
        }
    }

    return connection_lines;
}

int main(int argc, char* argv[]) {

    //Parsing bench file
    if(argc < 2) {
        std::cerr << "Filename was not provided\n";
        return FILENAME_NOT_PROVIDED;
    }

    LogicScheme logic_scheme;

    if(!parseInput(argv[1], logic_scheme)) {
        std::cerr << "Parser failure\n";
        return PARSER_FAILURE;
    }

    //SDL initialization
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not be initialized\n";
        return SDL_INIT_FAILURE;
    }

    SDL_DisplayMode display_info;
    SDL_GetCurrentDisplayMode(0, &display_info);

    SDL_Window* window = SDL_CreateWindow("test-viz", 0, 0, display_info.w, display_info.h, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    //Prepare draw data and draw
    drawBackground(renderer);
    drawElementsAndConnections(renderer, logic_scheme, prepareDrawData(logic_scheme, display_info.h, display_info.w));
    SDL_RenderPresent(renderer);

    //Event loop
    bool is_running = true;
    while(is_running) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) is_running = false;
        }
    }

    //Shutdown
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}