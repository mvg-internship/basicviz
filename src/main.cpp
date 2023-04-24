//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#define SDL_MAIN_HANDLED
#include "pugixml.hpp"

#include <vector>
#include <iostream>
#include <string>

#include <fstream>
#include "layout.h"
#include "netfmt_bench.h"
#include "main.h"
#include "minimization.h"

enum StatusCode {
  SUCCESS = 0,
  FILENAME_NOT_PROVIDED,
  RAW_PARSER_FAILURE,
  SDL_INIT_FAILURE,
  BENCH_READER_ERROR
};

const char *statusMessages[] = {
    "Success\n",
    "Filename was not provided\n",
    "Raw parser failure\n",
    "SDL could not be initialized\n",
    "Bench reader error\n"
};

const char *const parserElementId = "e_id";
const char *const parserConnectionId = "c_id";
const char *const parserX = "x";
const char *const parserY = "y";
const char *const parserHeight = "height";
const char *const parserWidth = "width";
const char *const parserEndElement = "end_element";
const char *const parserLogicScheme = "logic_scheme";
const char *const parserElements = "elements";
const char *const parserConnections = "connections";
const char *const parserOutlineColor = "outline_color";
const char *const parserFillColor = "fill_color";
const char *const parserColor = "color";
const char *const parserR = "r";
const char *const parserG = "g";
const char *const parserB = "b";

const float zoomInScalingFactor = 1.1f;
const float zoomOutScalingFactor = 0.9f;
const float mouseWheelScalingFactor = 0.1f;

const std::string printCompactMode = "--compact";
const std::string printDefaultMode = "--default";
const std::string parseRawMode = "--raw";

// struct NormalizedPoint {
//   float nX;
//   float nY;

//   NormalizedPoint(): nX(0), nY(0) {}
// };

// struct Connection {
//   unsigned int id;
//   unsigned int startElementId;
//   unsigned int endElementId;
//   SDL_Color color;
//   std::vector<NormalizedPoint> nVertices;
//   std::vector<SDL_FPoint> scrVertices;

//   Connection(): id(-1), startElementId(-1), endElementId(-1) {}
//   void normalizedToScreen(const int screenW, const int screenH);
//   void scale(const float scalingFactor, const int mouseX, const int mouseY);
//   void move(const int dx, const int dy);
// };

// struct Element {
//   unsigned int id;
//   NormalizedPoint nPoint;
//   float nW, nH;
//   SDL_FRect scrRect;
//   SDL_Color outlineColor;
//   SDL_Color fillColor;
//   std::vector<Connection> connections;

//   Element(): id(-1), nW(0), nH(0) {}
//   void normalizedToScreen(const int screenW, const int screenH);
//   void scale(const float scalingFactor, const int mouseX, const int mouseY);
//   void move(const int dx, const int dy);
// };

float normalizedToScreenX(const float nX, const int screenW) {
  return nX * screenW;
}

float normalizedToScreenY(const float nY, const int screenH) {
  return nY * screenH;
}

void Element::move(const int dx, const int dy) {
  scrRect.x += dx;
  scrRect.y += dy;

  for (Connection &connectionToMove : connections) {
    connectionToMove.move(dx, dy);
  }
}

void Connection::move(const int dx, const int dy) {
  for (SDL_FPoint &vertexToMove : scrVertices) {
    vertexToMove.x += dx;
    vertexToMove.y += dy;
  }
}

void Element::scale(
    const float scalingFactor,
    const int mouseX,
    const int mouseY) {
  scrRect.x = mouseX + (scrRect.x - mouseX) * scalingFactor;
  scrRect.y = mouseY + (scrRect.y - mouseY) * scalingFactor;
  scrRect.w *= scalingFactor;
  scrRect.h *= scalingFactor;

  for (Connection &connectionToScale : connections) {
    connectionToScale.scale(scalingFactor, mouseX, mouseY);
  }
}

void Connection::scale(
    const float scalingFactor,
    const int mouseX,
    const int mouseY) {
  for (SDL_FPoint &vertexToScale : scrVertices) {
    vertexToScale.x = mouseX + (vertexToScale.x - mouseX) * scalingFactor;
    vertexToScale.y = mouseY + (vertexToScale.y - mouseY) * scalingFactor;
  }
}

void Connection::normalizedToScreen(
    const int screenW,
    const int screenH) {
  for (const NormalizedPoint &nVertex : nVertices) {
    SDL_FPoint vertex;
    vertex.x = normalizedToScreenX(nVertex.nX, screenW);
    vertex.y = normalizedToScreenX(nVertex.nY, screenH);
    scrVertices.push_back(vertex);
  }
}

void Element::normalizedToScreen(
    const int screenW,
    const int screenH) {
  scrRect.x = normalizedToScreenX(nPoint.nX, screenW);
  scrRect.y = normalizedToScreenY(nPoint.nY, screenH);
  scrRect.w = normalizedToScreenX(nW, screenW);
  scrRect.h = normalizedToScreenY(nH, screenH);
}

void drawBackground(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);
}

int parseInput(
    std::istream &stream,
    std::vector<Element> &elementsToParse) {
  pugi::xml_document file;
  if (!file.load(stream)) {
    return RAW_PARSER_FAILURE;
  }

  pugi::xml_node elements = file.child(parserLogicScheme).child(parserElements);

  // Parsing elements from given file
  for (pugi::xml_node element = elements.first_child();
      element;
      element = element.next_sibling()) {
    Element parsedElement;
    parsedElement.id = atoi(element.attribute(parserElementId).value());
    parsedElement.nPoint.nX = std::stof(element.attribute(parserX).value());
    parsedElement.nPoint.nY = std::stof(element.attribute(parserY).value());
    parsedElement.nH = std::stof(element.attribute(parserHeight).value());
    parsedElement.nW = std::stof(element.attribute(parserWidth).value());

    // Reading outline color from the document and if there is not one detected
    // Defaults to white RGB(255, 255, 255)
    pugi::xml_node outlineColor = element.child(parserOutlineColor);
    if (outlineColor) {
      parsedElement.outlineColor.r = outlineColor.attribute(parserR).as_int();
      parsedElement.outlineColor.g = outlineColor.attribute(parserG).as_int();
      parsedElement.outlineColor.b = outlineColor.attribute(parserB).as_int();
    }
    else {
      parsedElement.outlineColor.r = 255;
      parsedElement.outlineColor.g = 255;
      parsedElement.outlineColor.b = 255;
    }
    // Reading fill color from the document and if there is not one detected
    // Defaults to black RGB(0, 0, 0)
    pugi::xml_node fillColor = element.child(parserFillColor);
    if (fillColor) {
      parsedElement.fillColor.r = fillColor.attribute(parserR).as_int();
      parsedElement.fillColor.g = fillColor.attribute(parserG).as_int();
      parsedElement.fillColor.b = fillColor.attribute(parserB).as_int();
    } else {
      parsedElement.fillColor.r = 0;
      parsedElement.fillColor.g = 0;
      parsedElement.fillColor.b = 0;
    }
    // Parsing connections for given element
    pugi::xml_node connections = element.child(parserConnections);
    for (pugi::xml_node connection = connections.first_child();
        connection;
        connection = connection.next_sibling()) {
      Connection parsedConnection;
      parsedConnection.id = atoi(connection.attribute(parserConnectionId).value());
      parsedConnection.startElementId = parsedElement.id;
      parsedConnection.endElementId = atoi(connection.attribute(parserEndElement).value());

      // Reading color from the document and if there is not one detected
      // Defaults to white RGB(255, 255, 255)
      pugi::xml_node color = connection.child(parserColor);
      if (color) {
        parsedConnection.color.r = color.attribute(parserR).as_int();
        parsedConnection.color.g = color.attribute(parserG).as_int();
        parsedConnection.color.b = color.attribute(parserB).as_int();
      } else {
        parsedConnection.color.r = 255;
        parsedConnection.color.g = 255;
        parsedConnection.color.b = 255;
      }
      // Parsing nVertices for given connection
      pugi::xml_node vertices = connection.child("vertices");
      for (pugi::xml_node vertex = vertices.first_child();
          vertex;
          vertex = vertex.next_sibling()) {
        NormalizedPoint parsedVertex;
        parsedVertex.nX = std::stof(vertex.attribute(parserX).value());
        parsedVertex.nY = std::stof(vertex.attribute(parserY).value());
        parsedConnection.nVertices.push_back(parsedVertex);
      }
      parsedElement.connections.push_back(parsedConnection);
    }
    elementsToParse.push_back(parsedElement);
  }
  return 0;
}

std::ostream &operator<<(
    std::ostream &out,
    const Element &elementToPrint) {
  out << "Element id: " << elementToPrint.id
      << " x: " << elementToPrint.nPoint.nX
      << " y: " << elementToPrint.nPoint.nY
      << std::endl;
  for (const Connection &connection : elementToPrint.connections) {
    out << "  Connection id: " << connection.id;
    for (size_t i = 0; i < connection.nVertices.size(); i++) {
      out << " x" << i << ": " << connection.nVertices[i].nX
          << ": " << connection.nVertices[i].nY;
    }
    std::cout << std::endl;
  }
  return out;
}

void print(
    const std::string &printMode,
    const std::vector<Element> &elementsToPrint) {
  if (printMode == printCompactMode) {
    size_t connectionsCount = 0;
    for (const Element &element : elementsToPrint) {
      connectionsCount += element.connections.size();
    }
    std::cout << "Number of elements: "
        << elementsToPrint.size()
        << "\nNumber of connections: "
        << connectionsCount
        << std::endl;
  } else if (printMode == printDefaultMode) {
    for (const Element &element : elementsToPrint) {
      std::cout << element;
    }
  }
}

void convertNormToScreen(
    std::vector<Element> &elementsToConvert,
    const int screenW,
    const int screenH) {
  // Converting normalized coordinates to screen
  for (Element &nElem : elementsToConvert) {
    nElem.normalizedToScreen(screenW, screenH);

    for (Connection &nConnection : nElem.connections) {
      nConnection.normalizedToScreen(screenW, screenH);
    }
  }
}

void drawFrame(
    SDL_Renderer *renderer,
    const std::vector<Element> &elementsToDraw) {
  drawBackground(renderer);

  for (const Element &elementToDraw : elementsToDraw) {
    SDL_SetRenderDrawColor(renderer,
        elementToDraw.fillColor.r,
        elementToDraw.fillColor.g,
        elementToDraw.fillColor.b,
        SDL_ALPHA_OPAQUE);
    SDL_RenderFillRectF(renderer, &elementToDraw.scrRect);

    SDL_SetRenderDrawColor(renderer,
        elementToDraw.outlineColor.r,
        elementToDraw.outlineColor.g,
        elementToDraw.outlineColor.b,
        SDL_ALPHA_OPAQUE);
    SDL_RenderDrawRectF(renderer, &elementToDraw.scrRect);

    for (const Connection &connectionToDraw : elementToDraw.connections) {
      SDL_SetRenderDrawColor(renderer,
        connectionToDraw.color.r,
        connectionToDraw.color.g,
        connectionToDraw.color.b,
        SDL_ALPHA_OPAQUE);
      for (size_t i = 1; i < connectionToDraw.scrVertices.size(); i++) {
        SDL_RenderDrawLineF(renderer,
          connectionToDraw.scrVertices[i - 1].x,
          connectionToDraw.scrVertices[i - 1].y,
          connectionToDraw.scrVertices[i].x,
          connectionToDraw.scrVertices[i].y);
      }
    }
  }
  SDL_RenderPresent(renderer);
}

void scaleViewport(
    const float scalingFactor,
    std::vector<Element> &elementsToScale) {
  int mouseX, mouseY;
  SDL_GetMouseState(&mouseX, &mouseY);

  for (Element &elementToScale : elementsToScale) {
    elementToScale.scale(scalingFactor, mouseX, mouseY);
  }
}

void moveViewport(
    const int dx,
    const int dy,
    std::vector<Element> &elementsToScale) {
  for (Element &elementToScale : elementsToScale) {
    elementToScale.move(dx, dy);
  }
}

float scaleMouseWheel(const Sint32 mouseWheelY) {
  return 1 + mouseWheelY * mouseWheelScalingFactor;
}

int main(int argc, char *argv[]) {
  // Parse text file
  if (argc < 2) {
    std::cerr << statusMessages[FILENAME_NOT_PROVIDED];
    return FILENAME_NOT_PROVIDED;
  }
  
  std::string printMode = printDefaultMode;
  if (argc >= 3) {
    printMode = argv[2];
  }

  bool parseRaw = false;
  if (argc >= 4) {
    if (argv[3] == parseRawMode) {
      parseRaw = true;
    }
  }

  std::ifstream ifs(argv[1]);
  std::vector<Element> normalizedElements = {};
  if (parseRaw) {
    if (parseInput(ifs, normalizedElements)) {
      std::cerr << statusMessages[RAW_PARSER_FAILURE];
      return RAW_PARSER_FAILURE;
    }
  } else {
    Net net = {};
    if (!readNetFromBench(ifs, net)) {
      std::cerr << statusMessages[BENCH_READER_ERROR];
      return BENCH_READER_ERROR;
    }
    net.assignLayers();
    net.netTreeNodesToNormalizedElements(normalizedElements);
  }
  // Prepare draw data and draw
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << statusMessages[SDL_INIT_FAILURE];
    return SDL_INIT_FAILURE;
  }

  print(printMode, normalizedElements);

  std::cout << statusMessages[SUCCESS];

  // Get screen dimensions
  SDL_DisplayMode display;
  SDL_GetCurrentDisplayMode(0, &display);
  const int screenW = display.w;
  const int screenH = display.h;

  SDL_Window *window = 
      SDL_CreateWindow("test-viz", 0, 0, screenW, screenH, SDL_WINDOW_SHOWN);
  SDL_Renderer *renderer = 
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  convertNormToScreen(normalizedElements, screenW, screenH);
  drawFrame(renderer, normalizedElements);

  // Event loop
  bool isRunning = true;
  bool isDragging = false;
  int mouseX1 = 0;
  int mouseY1 = 0;
  int mouseX2 = 0;
  int mouseY2 = 0;
  while (isRunning) {
    SDL_Event event;
    // User input handler
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        isRunning = false;
      } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        isDragging = true;
        SDL_GetMouseState(&mouseX1, &mouseY1);
      } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        isDragging = false;
        mouseX1 = 0;
        mouseY1 = 0;
        mouseX2 = 0;
        mouseY2 = 0;
      } else if (isDragging && SDL_GetMouseState(&mouseX2, &mouseY2)) {
        moveViewport(mouseX2 - mouseX1, mouseY2 - mouseY1, normalizedElements);
        drawFrame(renderer, normalizedElements);
        SDL_GetMouseState(&mouseX1, &mouseY1);
      } else if (event.type == SDL_KEYDOWN) {
        // Keyboard input handler
        switch (event.key.keysym.sym) {
        case SDLK_KP_PLUS:
          scaleViewport(zoomInScalingFactor, normalizedElements);
          break;
        case SDLK_KP_MINUS:
          scaleViewport(zoomOutScalingFactor, normalizedElements);
          break;
        case SDLK_ESCAPE:
          isRunning = false;
          break;
        }
        drawFrame(renderer, normalizedElements);
      } else if (event.type == SDL_MOUSEWHEEL) {
        scaleViewport(scaleMouseWheel(event.wheel.y), normalizedElements);
        drawFrame(renderer, normalizedElements);
      }
    }
  }
  // Shutdown
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
