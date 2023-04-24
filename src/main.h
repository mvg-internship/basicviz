//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#ifndef MAIN_H_
#define MAIN_H_

#include <SDL.h>

struct NormalizedPoint {
  float nX;
  float nY;

  NormalizedPoint(): nX(0), nY(0) {}
};

struct Connection {
  unsigned int id;
  unsigned int startElementId;
  unsigned int endElementId;
  SDL_Color color;
  std::vector<NormalizedPoint> nVertices;
  std::vector<SDL_FPoint> scrVertices;

  Connection(): id(-1), startElementId(-1), endElementId(-1) {
    color = {255, 255, 255, 255};
  }
  void normalizedToScreen(const int screenW, const int screenH);
  void scale(const float scalingFactor, const int mouseX, const int mouseY);
  void move(const int dx, const int dy);
};

struct Element {
  unsigned int id;
  NormalizedPoint nPoint;
  float nW, nH;
  SDL_FRect scrRect;
  SDL_Color outlineColor;
  SDL_Color fillColor;
  std::vector<Connection> connections;

  Element(): id(-1), nW(0), nH(0) {
    outlineColor = {255, 255, 255, 255};
    fillColor = {0, 0, 0, 255};
  }
  void normalizedToScreen(const int screenW, const int screenH);
  void scale(const float scalingFactor, const int mouseX, const int mouseY);
  void move(const int dx, const int dy);
};

#endif // MAIN_H_
