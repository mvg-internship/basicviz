#include <vector>
#include <iostream>
#include <string>
#include <windows.h>

#include "pugixml.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>

const int screen_width = GetSystemMetrics(SM_CXSCREEN);
const int screen_height = GetSystemMetrics(SM_CYSCREEN);

enum StatusCode {
    SUCCESS = 0,
    FILENAME_NOT_PROVIDED,
    PARSER_FAILURE,
    SDL_INIT_FAILURE
};

const char *status_messages[] = {
        "Success\n",
        "Filename was not provided\n",
        "Parser failure\n",
        "SDL could not be initialized\n"
};

const char *parser_e_id = "e_id";
const char *parser_c_id = "c_id";
const char *parser_x = "x";
const char *parser_y = "y";
const char *parser_height = "height";
const char *parser_width = "width";
const char *parser_end_elem = "end_element";

struct NormalizedPoint {
    float n_x;
    float n_y;

    NormalizedPoint() : n_x(0), n_y(0) {}
};

struct NormalizedConnection {
    unsigned int id;
    unsigned int start_element_id;
    unsigned int end_element_id;
    std::vector<NormalizedPoint> vertices;

    NormalizedConnection() : id(-1), start_element_id(-1), end_element_id(-1) {}
};

struct NormalizedElement {
    unsigned int id;
    NormalizedPoint point;
    float n_w;
    float n_h;
    std::vector<NormalizedConnection> connections;

    NormalizedElement() : id(-1), n_w(0), n_h(0) {}

    friend std::ostream &operator<<(std::ostream &out, const NormalizedElement &element_to_print);
};

float normalizedToScreenX(const float n_x) {
    return n_x * screen_width;
}

float normalizedToScreenY(const float n_y) {
    return n_y * screen_height;
}

void drawBackground(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
}

int parseInput(const char *filename, std::vector<NormalizedElement> &elements_to_parse) {
    pugi::xml_document file;
    if (!file.load_file(filename)) return PARSER_FAILURE;
    pugi::xml_node elements = file.child("logic_scheme").child("elements");

    //Parsing file for elements
    for (pugi::xml_node element = elements.first_child(); element; element = element.next_sibling()) {
        NormalizedElement parsed_element;
        parsed_element.id = atoi(element.attribute(parser_e_id).value());
        parsed_element.point.n_x = std::stof(element.attribute(parser_x).value());
        parsed_element.point.n_y = std::stof(element.attribute(parser_y).value());
        parsed_element.n_h = std::stof(element.attribute(parser_height).value());
        parsed_element.n_w = std::stof(element.attribute(parser_width).value());

        //Parsing elements for connections
        for (pugi::xml_node connection = element.first_child(); connection; connection = connection.next_sibling()) {
            NormalizedConnection parsed_connection;
            parsed_connection.id = atoi(connection.attribute(parser_c_id).value());
            parsed_connection.start_element_id = parsed_element.id;
            parsed_connection.end_element_id = atoi(connection.attribute(parser_end_elem).value());

            //Parsing connections for vertices
            for (pugi::xml_node vertex = connection.first_child(); vertex; vertex = vertex.next_sibling()) {
                NormalizedPoint parsed_vertex;
                parsed_vertex.n_x = std::stof(vertex.attribute(parser_x).value());
                parsed_vertex.n_y = std::stof(vertex.attribute(parser_y).value());
                parsed_connection.vertices.push_back(parsed_vertex);
            }

            parsed_element.connections.push_back(parsed_connection);
        }

        elements_to_parse.push_back(parsed_element);
    }

    return 0;
}

std::ostream &operator<<(std::ostream &out, const NormalizedElement &element_to_print) {
    out << element_to_print.id << ' ' << element_to_print.point.n_x << ' ' << element_to_print.point.n_y << std::endl;
    for (const NormalizedConnection &connection: element_to_print.connections) {
        out << "    " << connection.id;
        for (const NormalizedPoint &vertex: connection.vertices) {
            out << ' ' << vertex.n_x << ' ' << vertex.n_y;
        }
        std::cout << std::endl;
    }
    return out;
}

void print(const std::vector<NormalizedElement> &elements_to_print) {
    for (const NormalizedElement &element: elements_to_print) {
        std::cout << element;
    }
}

void draw(SDL_Renderer *renderer, const std::vector<NormalizedElement> &elements_to_draw) {
    drawBackground(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

    for (const NormalizedElement &n_elem: elements_to_draw) {
        SDL_FRect rect;
        rect.x = normalizedToScreenX(n_elem.point.n_x);
        rect.y = normalizedToScreenY(n_elem.point.n_y);
        rect.w = normalizedToScreenX(n_elem.n_w);
        rect.h = normalizedToScreenY(n_elem.n_h);
        SDL_RenderDrawRectF(renderer, &rect);

        //Placing connection Lines on the screen
        for (const NormalizedConnection &n_connection: n_elem.connections) {
            //СДелать копирование
            for (int i = 1; i < n_connection.vertices.size(); i++) {
                SDL_RenderDrawLineF(renderer, normalizedToScreenX(n_connection.vertices[i - 1].n_x),
                                    normalizedToScreenY(n_connection.vertices[i - 1].n_y),
                                    normalizedToScreenX(n_connection.vertices[i].n_x),
                                    normalizedToScreenY(n_connection.vertices[i].n_y));
            }

        }
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]) {

    //Parse text file
    if (argc < 2) {
        std::cerr << status_messages[FILENAME_NOT_PROVIDED];
        return FILENAME_NOT_PROVIDED;
    }

    std::vector<NormalizedElement> normalized_elements;

    int code = parseInput(argv[1], normalized_elements);
    if (code) {
        std::cerr << status_messages[code];
        return code;
    }

    print(normalized_elements);

    //Prepare draw data and draw
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << status_messages[SDL_INIT_FAILURE];
        return SDL_INIT_FAILURE;
    }

    std::cout << status_messages[SUCCESS];

    SDL_Window *window = SDL_CreateWindow("test-viz", 0, 0, screen_width, screen_height, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    draw(renderer, normalized_elements);

    //Event loop
    bool is_running = true;
    while (is_running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) is_running = false;
        }
    }

    //Shutdown
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}