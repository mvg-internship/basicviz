//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#define SDL_MAIN_HANDLED
#include "pugixml.hpp"
#include <SDL.h>

#include <vector>
#include <iostream>
#include <string>

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

const char * const parser_e_id = "e_id";
const char * const parser_c_id = "c_id";
const char * const parser_x = "x";
const char * const parser_y = "y";
const char * const parser_height = "height";
const char * const parser_width = "width";
const char * const parser_end_elem = "end_element";

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
};

float normalizedToScreenX(const float n_x, const int screen_w) {
  return n_x * screen_w;
}

float normalizedToScreenY(const float n_y, const int screen_h) {
  return n_y * screen_h;
}

void drawBackground(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);
}

int parseInput(const char *filename,
               std::vector<NormalizedElement> &elements_to_parse) {
  pugi::xml_document file;
  if (!file.load_file(filename)) return PARSER_FAILURE;
  pugi::xml_node elements = file.child("logic_scheme").child("elements");

  //Parsing elements from given file
  for (pugi::xml_node element = elements.first_child();
      element;
      element = element.next_sibling()) {
    NormalizedElement parsed_element;
    parsed_element.id = atoi(element.attribute(parser_e_id).value());
    parsed_element.point.n_x = std::stof(element.attribute(parser_x).value());
    parsed_element.point.n_y = std::stof(element.attribute(parser_y).value());
    parsed_element.n_h = std::stof(element.attribute(parser_height).value());
    parsed_element.n_w = std::stof(element.attribute(parser_width).value());

    //Parsing connections for given element
    for (pugi::xml_node connection = element.first_child();
        connection;
        connection = connection.next_sibling()) {
      NormalizedConnection parsed_connection;
      parsed_connection.id = atoi(connection.attribute(parser_c_id).value());
      parsed_connection.start_element_id = parsed_element.id;
      parsed_connection.end_element_id = atoi(connection.attribute(parser_end_elem).value());

      //Parsing vertices for given connection
      for (pugi::xml_node vertex = connection.first_child();
          vertex;
          vertex = vertex.next_sibling()) {
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

std::ostream &operator<<(std::ostream &out,
        const NormalizedElement &element_to_print) {
  out << element_to_print.id << ' ' <<
  element_to_print.point.n_x << ' ' <<
  element_to_print.point.n_y << std::endl;
  for (const NormalizedConnection &connection: element_to_print.connections) {
    out << "  " << connection.id;
    for (const NormalizedPoint &vertex: connection.vertices) {
      out << ' ' << vertex.n_x << ' '
      << vertex.n_y;
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

SDL_Texture* getTexture(SDL_Renderer *renderer,
          const std::vector<NormalizedElement> &elements_to_draw,
          const int screen_w,
          const int screen_h) {
  auto texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 1920, 1080);
  SDL_SetRenderTarget(renderer, texture);

  drawBackground(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

  //Placing elements on the texture
  for (const NormalizedElement &n_elem: elements_to_draw) {
    SDL_FRect rect;
    rect.x = normalizedToScreenX(n_elem.point.n_x, screen_w);
    rect.y = normalizedToScreenY(n_elem.point.n_y, screen_h);
    rect.w = normalizedToScreenX(n_elem.n_w, screen_w);
    rect.h = normalizedToScreenY(n_elem.n_h, screen_h);
    SDL_RenderDrawRectF(renderer, &rect);

    //Placing connection lines on the texture
    for (const NormalizedConnection &n_connection: n_elem.connections) {
      for (int i = 1; i < n_connection.vertices.size(); i++) {
        SDL_RenderDrawLineF(renderer,
                            normalizedToScreenX(n_connection.vertices[i - 1].n_x, screen_w),
                            normalizedToScreenY(n_connection.vertices[i - 1].n_y, screen_h),
                            normalizedToScreenX(n_connection.vertices[i].n_x, screen_w),
                            normalizedToScreenY(n_connection.vertices[i].n_y, screen_h));
      }

    }
  }

  //Set renderer to render window
  SDL_SetRenderTarget(renderer, nullptr);

  return texture;
}

void drawFrame(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect* src) {
  SDL_RenderCopy(renderer, texture, src, nullptr);
  SDL_RenderPresent(renderer);
}

void scaleFrame(SDL_Rect& src, int increase_by) {
  src.w += increase_by;
  src.h += increase_by;
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

  //Get screen dimensions
  SDL_DisplayMode display;
  SDL_GetCurrentDisplayMode(0, &display);
  static const int screen_w = display.w;
  static const int screen_h = display.h;

  SDL_Window *window = SDL_CreateWindow("test-viz", 0, 0,
                                        screen_w, screen_h,
                                        SDL_WINDOW_SHOWN);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                              SDL_RENDERER_ACCELERATED);
  auto texture = getTexture(renderer, normalized_elements, screen_w, screen_h);

  SDL_Rect frame_area{0, 0, screen_w, screen_h};

  //Event loop
  bool is_running = true;
  while (is_running) {
    drawFrame(renderer, texture, &frame_area);
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) is_running = false;
      if (event.type == SDL_KEYDOWN) {
        switch(event.key.keysym.sym) {
          case SDLK_KP_PLUS:
            scaleFrame(frame_area, 10);
            break;
          case SDLK_KP_MINUS:
            scaleFrame(frame_area, -10);
            break;
        }
      }
    }
  }

  //Shutdown
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}