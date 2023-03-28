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
  std::vector<NormalizedPoint> n_vertices;
  std::vector<SDL_FPoint> scr_vertices;

  NormalizedConnection() : id(-1), start_element_id(-1), end_element_id(-1) {}
  void normalizedToScreen(const int screen_w, const int screen_h);
};

struct NormalizedElement {
  unsigned int id;
  NormalizedPoint n_point;
  float n_w, n_h;
  SDL_FRect scr_rect;
  std::vector<NormalizedConnection> connections;

  NormalizedElement() : id(-1), n_w(0), n_h(0) {}
  void normalizedToScreen(const int screen_w, const int screen_h);
};

float normalizedToScreenX(const float n_x, const int screen_w) {
  return n_x * screen_w;
}

float normalizedToScreenY(const float n_y, const int screen_h) {
  return n_y * screen_h;
}

void NormalizedConnection::normalizedToScreen(const int screen_w, const int screen_h) {
  for(const NormalizedPoint& n_vertex : n_vertices) {
      SDL_FPoint vertex;
      vertex.x = normalizedToScreenX(n_vertex.n_x, screen_w);
      vertex.y = normalizedToScreenX(n_vertex.n_y, screen_h);
      scr_vertices.push_back(vertex);
    }
}

void NormalizedElement::normalizedToScreen(const int screen_w, const int screen_h) {
    scr_rect.x = normalizedToScreenX(n_point.n_x, screen_w);
    scr_rect.y = normalizedToScreenY(n_point.n_y, screen_h);
    scr_rect.w = normalizedToScreenX(n_w, screen_w);
    scr_rect.h = normalizedToScreenY(n_h, screen_h);
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
    parsed_element.n_point.n_x = std::stof(element.attribute(parser_x).value());
    parsed_element.n_point.n_y = std::stof(element.attribute(parser_y).value());
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

      //Parsing n_vertices for given connection
      for (pugi::xml_node vertex = connection.first_child();
          vertex;
          vertex = vertex.next_sibling()) {
        NormalizedPoint parsed_vertex;
        parsed_vertex.n_x = std::stof(vertex.attribute(parser_x).value());
        parsed_vertex.n_y = std::stof(vertex.attribute(parser_y).value());
        parsed_connection.n_vertices.push_back(parsed_vertex);
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
  element_to_print.n_point.n_x << ' ' <<
  element_to_print.n_point.n_y << std::endl;
  for (const NormalizedConnection &connection: element_to_print.connections) {
    out << "  " << connection.id;
    for (const NormalizedPoint &vertex: connection.n_vertices) {
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

void convertNormToScreen(std::vector<NormalizedElement> &elements_to_convert,
          const int screen_w,
          const int screen_h) {
  //Converting normalized coordinates to screen
  for (NormalizedElement &n_elem: elements_to_convert) {
    n_elem.normalizedToScreen(screen_w, screen_h);

    for (NormalizedConnection &n_connection: n_elem.connections) {
      n_connection.normalizedToScreen(screen_w, screen_h);  
    }
  }
}

void drawFrame(SDL_Renderer* renderer, const std::vector<NormalizedElement>& elements_to_draw) {
  drawBackground(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
  
  for(const NormalizedElement& element_to_draw : elements_to_draw) {
    SDL_RenderDrawRectF(renderer, &element_to_draw.scr_rect);
    for(const NormalizedConnection& connection_to_draw : element_to_draw.connections) {
      for(size_t i = 1; i < connection_to_draw.scr_vertices.size(); i++) {
        SDL_RenderDrawLineF(renderer,
          connection_to_draw.scr_vertices[i - 1].x,
          connection_to_draw.scr_vertices[i - 1].y,
          connection_to_draw.scr_vertices[i].x,
          connection_to_draw.scr_vertices[i].y);
      }
    } 
  }
  SDL_RenderPresent(renderer);
}

void scaleFrame(float scaling_factor, std::vector<NormalizedElement>& elements_to_scale) {
  int mouse_x, mouse_y;
  SDL_GetMouseState(&mouse_x, &mouse_y);

  std::cout << mouse_x << ' ' << mouse_y << std::endl;
  
  for(NormalizedElement& element_to_scale : elements_to_scale) {
    element_to_scale.scr_rect.x = mouse_x + (element_to_scale.scr_rect.x - mouse_x) * scaling_factor;
    element_to_scale.scr_rect.y = mouse_y + (element_to_scale.scr_rect.y - mouse_y) * scaling_factor;
    element_to_scale.scr_rect.w *= scaling_factor;
    element_to_scale.scr_rect.h *= scaling_factor;
 
    for(NormalizedConnection& connection_to_scale : element_to_scale.connections) {
      for(SDL_FPoint& vertex_to_scale : connection_to_scale.scr_vertices) {
        vertex_to_scale.x = mouse_x + (vertex_to_scale.x - mouse_x) * scaling_factor;
        vertex_to_scale.y = mouse_y + (vertex_to_scale.y - mouse_y) * scaling_factor;
      }
    }
  }
}

void moveViewport(int dx, int dy, std::vector<NormalizedElement>& elements_to_scale) {
  for(NormalizedElement& element_to_scale : elements_to_scale) {
    element_to_scale.scr_rect.x += dx;
    element_to_scale.scr_rect.y += dy;
 
    for(NormalizedConnection& connection_to_scale : element_to_scale.connections) {
      for(SDL_FPoint& vertex_to_scale : connection_to_scale.scr_vertices) {
        vertex_to_scale.x += dx;
        vertex_to_scale.y += dy;
      }
    }
  }
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
  convertNormToScreen(normalized_elements, screen_w, screen_h);
  drawFrame(renderer, normalized_elements);
  
  //Event loop
  bool is_running = true;
  bool is_dragging = false;
  int mouse_x1 = 0;
  int mouse_y1 = 0;
  int mouse_x2 = 0;
  int mouse_y2 = 0;
  while (is_running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) is_running = false;
      else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        is_dragging = true;
        SDL_GetMouseState(&mouse_x1, &mouse_y1);
      }
      else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        is_dragging = false;
        mouse_x1 = 0;
        mouse_y1 = 0;
        mouse_x2 = 0;
        mouse_y2 = 0;
      }
      else if(is_dragging && SDL_GetMouseState(&mouse_x2, &mouse_y2)) {
        moveViewport(mouse_x2 - mouse_x1, mouse_y2 - mouse_y1, normalized_elements);
        drawFrame(renderer, normalized_elements);
        SDL_GetMouseState(&mouse_x1, &mouse_y1);
      }
      else if (event.type == SDL_KEYDOWN) {
        switch(event.key.keysym.sym) {
          case SDLK_KP_PLUS:
            scaleFrame(1.1, normalized_elements);
            drawFrame(renderer, normalized_elements);
            break;
          case SDLK_KP_MINUS:
            scaleFrame(0.9, normalized_elements);
            drawFrame(renderer, normalized_elements);
            break;
          case SDLK_ESCAPE:
            is_running = false;
            break;
        }
      }
      else if (event.type == SDL_MOUSEWHEEL) {
        scaleFrame(1 + event.wheel.y * 0.1, normalized_elements);
        drawFrame(renderer, normalized_elements);
      }
    }
  }

  //Shutdown
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
