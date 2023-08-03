
//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#define SDL_MAIN_HANDLED
#include "pugixml.hpp"
#include <CLI/CLI.hpp>

#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <limits>

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
const char *const parserBackColor = "background_color";
const char *const parserR = "r";
const char *const parserG = "g";
const char *const parserB = "b";

const float zoomInScalingFactor = 1.1f;
const float zoomOutScalingFactor = 0.9f;
const float mouseWheelScalingFactor = 0.1f;

const float inf = std::numeric_limits<float>::infinity();

const unsigned int framesNum = 100;

const std::string optionFile = "file";
const std::string flagFPS = "--fps";
const std::string flagRaw = "--raw";
const std::string flagCompact = "--compact";
const std::string flagMinimize = "--minimize";
const std::string flagColors = "--colors";
const std::string flagDummy = "--dummy";
const std::string flagTexture = "--texturize";
const std::string flagWidthLimitation = "--limit";
const std::string flagTestNodesPlacement = "--testNodesPlacement";

const std::string texturePath = 
    std::string(ROOT_DIR) + 
    std::string("/assets/img/gates.bmp");
SDL_Texture *gatesTexture = NULL;

float normalizedToScreenX(const float nX, const int screenW) {
  return nX * screenW;
}

float normalizedToScreenY(const float nY, const int screenH) {
  return nY * screenH;
}

void ScreenType::setType(Type type) {
  switch (type) {
  case INPUT:
    textureRect = {0, 1838, 604, 186};
    break;
  case OUTPUT:
    textureRect = {988, 1838, 604, 186};
    break;
  case AND:
    textureRect = {0, 0, 607, 321};
    break;
  case NAND:
    textureRect = {832, 0, 653, 321};
    break;
  case OR:
    textureRect = {30, 701, 623, 321};
    break;
  case NOR:
    textureRect = {861, 701, 672, 321};
    break;
  case NOT:
    textureRect = {0, 2122, 505, 278};
    break;
  case DFF:
    textureRect = {0, 1176, 512, 506};
    break;
  default:
    break;
  }
}

Type ScreenType::getType() const {
  return type;
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

void drawBackground(SDL_Renderer *renderer, 
    const SDL_Color &backgroundColor, 
    const bool drawColor) {
  if (drawColor) {
    SDL_SetRenderDrawColor(renderer, 
        backgroundColor.r,
        backgroundColor.g,
        backgroundColor.b,
        SDL_ALPHA_OPAQUE);
  } else {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  }
  SDL_RenderClear(renderer);
}

int parseInput(
    std::istream &stream,
    std::vector<Element> &elementsToParse,
    SDL_Color &backgroundColor) {
  pugi::xml_document file;
  if (!file.load(stream)) {
    return RAW_PARSER_FAILURE;
  }

  pugi::xml_node parsedBackColor = file.child(parserLogicScheme).child(parserBackColor);
  backgroundColor.r = parsedBackColor.attribute(parserR).as_int(255);
  backgroundColor.g = parsedBackColor.attribute(parserG).as_int(255);
  backgroundColor.b = parsedBackColor.attribute(parserB).as_int(255);

  pugi::xml_node elements = file.child(parserLogicScheme).child(parserElements);

  // Parsing elements from given file
  for (pugi::xml_node element = elements.first_child();
      element;
      element = element.next_sibling()) {
    Element parsedElement;
    parsedElement.id = element.attribute(parserElementId).as_int(-1);
    parsedElement.nPoint.nX = element.attribute(parserX).as_float();
    parsedElement.nPoint.nY = element.attribute(parserY).as_float();
    parsedElement.nH = element.attribute(parserHeight).as_float();
    parsedElement.nW = element.attribute(parserWidth).as_float();

    // Reading outline color from the document and if there is not one detected
    // Defaults to cyan RGB(3, 161, 252)
    pugi::xml_node outlineColor = element.child(parserOutlineColor);
    if (outlineColor) {
      parsedElement.outlineColor.r = outlineColor.attribute(parserR).as_int();
      parsedElement.outlineColor.g = outlineColor.attribute(parserG).as_int();
      parsedElement.outlineColor.b = outlineColor.attribute(parserB).as_int();
    }
    else {
      parsedElement.outlineColor.r = 3;
      parsedElement.outlineColor.g = 161;
      parsedElement.outlineColor.b = 252;
    }
    // Reading fill color from the document and if there is not one detected
    // Defaults to cyan RGB(3, 161, 252)
    pugi::xml_node fillColor = element.child(parserFillColor);
    if (fillColor) {
      parsedElement.fillColor.r = fillColor.attribute(parserR).as_int();
      parsedElement.fillColor.g = fillColor.attribute(parserG).as_int();
      parsedElement.fillColor.b = fillColor.attribute(parserB).as_int();
    } else {
      parsedElement.fillColor.r = 3;
      parsedElement.fillColor.g = 161;
      parsedElement.fillColor.b = 252;
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
      // Defaults to black RGB(0, 0, 0)
      pugi::xml_node color = connection.child(parserColor);
      if (color) {
        parsedConnection.color.r = color.attribute(parserR).as_int();
        parsedConnection.color.g = color.attribute(parserG).as_int();
        parsedConnection.color.b = color.attribute(parserB).as_int();
      } else {
        parsedConnection.color.r = 0;
        parsedConnection.color.g = 0;
        parsedConnection.color.b = 0;
      }
      // Parsing nVertices for given connection
      pugi::xml_node vertices = connection.child("vertices");
      for (pugi::xml_node vertex = vertices.first_child();
          vertex;
          vertex = vertex.next_sibling()) {
        NormalizedPoint parsedVertex;
        parsedVertex.nX = vertex.attribute(parserX).as_float();
        parsedVertex.nY = vertex.attribute(parserY).as_float();
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
    const std::vector<Element> &elementsToPrint,
    bool printCompact) {
  if (printCompact) {
    size_t connectionsCount = 0;
    for (const Element &element : elementsToPrint) {
      connectionsCount += element.connections.size();
    }
    std::cout << "Number of elements: "
        << elementsToPrint.size()
        << "\nNumber of connections: "
        << connectionsCount
        << std::endl;
  } else {
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

float maxi(const float arr[], int n) {
  float m = 0;
  for (int i = 0; i < n; ++i)
    if (m < arr[i])
      m = arr[i];
  return m;
}

float mini(const float arr[], int n) {
  float m = 1;
  for (int i = 0; i < n; ++i)
    if (m > arr[i])
      m = arr[i];
  return m;
}

bool checkSegmentRectCollision(
    SDL_FPoint &point1,
    SDL_FPoint &point2,
    const int screenW,
    const int screenH) {
  float x1 = point1.x;
  float y1 = point1.y;
  float x2 = point2.x;
  float y2 = point2.y;

  float p1 = -(x2 - x1);
  float p2 = -p1;
  float p3 = -(y2 - y1);
  float p4 = -p3;
  
  float q1 = x1;
  float q2 = screenW - x1;
  float q3 = y1;
  float q4 = screenH - y1;

  float posarr[5], negarr[5];
  int posind = 1, negind = 1;
  posarr[0] = 1;
  negarr[0] = 0;

  if ((p1 == 0 && q1 < 0) || (p3 == 0 && q3 < 0)) {
    return false;
  }
  if (p1 != 0) {
    float r1 = q1 / p1;
    float r2 = q2 / p2;
    if (p1 < 0) {
      negarr[negind++] = r1;
      posarr[posind++] = r2;
    } else {
      negarr[negind++] = r2;
      posarr[posind++] = r1;
    }
  }
  if (p3 != 0) {
    float r3 = q3 / p3;
    float r4 = q4 / p4;
    if (p3 < 0) {
      negarr[negind++] = r3;
      posarr[posind++] = r4;
    } else {
      negarr[negind++] = r4;
      posarr[posind++] = r3;
    }
  }
  float rn1, rn2;

  rn1 = maxi(negarr, negind);
  rn2 = mini(posarr, posind);

  if (rn1 > rn2) {
    return false;
  }
  point1.x = x1 + p2 * rn1;
  point1.y = y1 + p4 * rn1;

  point2.x = x1 + p2 * rn2;
  point2.y = y1 + p4 * rn2;

  return true;
}

void drawFrame(
    SDL_Renderer *renderer,
    const std::vector<Element> &elementsToDraw,
    const SDL_Color &backgroundColor,
    const int screenW,
    const int screenH,
    const bool drawColor,
    const bool texturize) {
  drawBackground(renderer, backgroundColor, drawColor);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

  for (const Element &elementToDraw : elementsToDraw) {
    if (drawColor && !texturize) {
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
    }
    if (texturize) {
      SDL_RenderCopyF(renderer,
          gatesTexture,
          &elementToDraw.scrType.textureRect,
          &elementToDraw.scrRect);
    } else {
      SDL_RenderDrawRectF(renderer, &elementToDraw.scrRect);
    }
    for (const Connection &connectionToDraw : elementToDraw.connections) {
      if (drawColor) {
        SDL_SetRenderDrawColor(renderer,
          connectionToDraw.color.r,
          connectionToDraw.color.g,
          connectionToDraw.color.b,
          SDL_ALPHA_OPAQUE);
      }
      for (size_t i = 1; i < connectionToDraw.scrVertices.size(); i++) {
        SDL_FPoint point1 = connectionToDraw.scrVertices[i - 1];
        SDL_FPoint point2 = connectionToDraw.scrVertices[i];

        bool collision = checkSegmentRectCollision(point1,
            point2,
            screenW,
            screenH);
        if (collision) {
              SDL_RenderDrawLineF(renderer,
                  point1.x,
                  point1.y,
                  point2.x,
                  point2.y);
        }
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

void defaultColor(std::vector<Element> &elementsToColor, SDL_Color &backgroundColor) {
  for (Element &elementToColor : elementsToColor) {
    elementToColor.fillColor = {3, 161, 252, 255};
    elementToColor.outlineColor = {3, 161, 252, 255};
    for (Connection &connectionToColor : elementToColor.connections) {
      connectionToColor.color = {0, 0, 0, 255};
    }
  }
  backgroundColor = {255, 255, 255, 255};
}

int main(int argc, char *argv[]) {
  std::cout << ROOT_DIR;
  CLI::App cliApp;

  std::string filename;
  cliApp.add_option(optionFile, 
      filename, 
      "File to parse")->required();

  bool parseRaw = false;
  auto cliRaw = cliApp.add_flag(flagRaw, 
      parseRaw, 
      "Parse graph represented as FLG file");

  bool printCompact = false;
  cliApp.add_flag(flagCompact, 
      printCompact, 
      "Print quantity of elements and connections");

  bool drawColor = false;
  cliApp.add_flag(flagColors, 
      drawColor, 
      "Draw color");

  bool showFPS = false;
  cliApp.add_flag(flagFPS, 
      showFPS, 
      "Show average frame time, FPS and BENCH file processing time");

  bool processMinimize = false;
  cliApp.add_flag(flagMinimize, 
      processMinimize, 
      "Minimize intersections")->excludes(cliRaw);

  bool showDummy = false;
  cliApp.add_flag(flagDummy, 
      showDummy, 
      "Show dummy nodes")->excludes(cliRaw);

  bool texturize = false;
  cliApp.add_flag(flagTexture, 
      texturize, 
      "Texturize nodes");

  size_t widthLimitation = 1000;
  cliApp.add_option(flagWidthLimitation, 
      widthLimitation, 
      "Graph width limitation");
  
  bool testNodesPlacement = false;
  cliApp.add_flag(flagTestNodesPlacement, 
      testNodesPlacement, 
      "Testing nodes placement");  
  
  CLI11_PARSE(cliApp, argc, argv);

  std::ifstream ifs(filename);
  std::vector<Element> normalizedElements = {};
  SDL_Color backgroundColor;
  if (parseRaw) {
    if (parseInput(ifs, normalizedElements, backgroundColor)) {
      std::cerr << statusMessages[RAW_PARSER_FAILURE];
      return RAW_PARSER_FAILURE;
    }
  } else {
    Net net = {};
    if (!readNetFromBench(ifs, net)) {
      std::cerr << statusMessages[BENCH_READER_ERROR];
      return BENCH_READER_ERROR;
    }
    auto start = std::chrono::high_resolution_clock::now();
    net.assignLayers(widthLimitation, testNodesPlacement);
    if (processMinimize) {
      minimizeIntersections(net);
    }
    net.netTreeNodesToNormalizedElements(normalizedElements, showDummy);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    if (showFPS) {
      std::cout << "BENCH processing time: " 
          << duration
          << " ms." 
          << std::endl;
    }
    defaultColor(normalizedElements, backgroundColor);
  }
  // Prepare draw data and draw
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << statusMessages[SDL_INIT_FAILURE];
    return SDL_INIT_FAILURE;
  }

  print(normalizedElements, printCompact);

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
  SDL_Surface *gatesSurface = SDL_LoadBMP(texturePath.c_str());
  gatesTexture = SDL_CreateTextureFromSurface(renderer, gatesSurface);
  SDL_FreeSurface(gatesSurface);

  convertNormToScreen(normalizedElements, screenW, screenH);
  drawFrame(renderer, normalizedElements, backgroundColor, screenW, screenH, drawColor, texturize);

  // Event loop
  bool isRunning = true;
  bool isDragging = false;
  int mouseX1 = 0;
  int mouseY1 = 0;
  int mouseX2 = 0;
  int mouseY2 = 0;

  auto start = std::chrono::high_resolution_clock::now();
  unsigned int count = 0;
  while (isRunning) {
    if (count == framesNum) {
      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
      auto avgFrameTime = duration / count;
      auto avgFPS = 1000/avgFrameTime;
      start = std::chrono::high_resolution_clock::now();
      count = 0;
      if (showFPS) {
        std::cout << "Avg frame time: " 
            << avgFrameTime 
            << " ms. FPS:" 
            << avgFPS 
            << std::endl;
      }     
    }
    count++;
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
      } else if (event.type == SDL_MOUSEWHEEL) {
        scaleViewport(scaleMouseWheel(event.wheel.y), normalizedElements);
      }
    }
    drawFrame(renderer, normalizedElements, backgroundColor, screenW, screenH, drawColor, texturize);
  }
  // Shutdown
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyTexture(gatesTexture);
  SDL_Quit();
  return 0;
}
