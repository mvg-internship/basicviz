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

struct NormalizedConnection {
  unsigned int id;
  unsigned int startElementId;
  unsigned int endElementId;
  std::vector<NormalizedPoint> nVertices;
  std::vector<SDL_FPoint> scrVertices;

  NormalizedConnection(): id(-1), startElementId(-1), endElementId(-1) {}
  void normalizedToScreen(const int screenW, const int screenH);
  void scale(const float scalingFactor, const int mouseX, const int mouseY);
  void move(const int dx, const int dy);
};

struct NormalizedElement {
  unsigned int id;
  NormalizedPoint nPoint;
  float nW, nH;
  SDL_FRect scrRect;
  std::vector<NormalizedConnection> connections;

  NormalizedElement(): id(-1), nW(0), nH(0) {}
  void normalizedToScreen(const int screenW, const int screenH);
  void scale(const float scalingFactor, const int mouseX, const int mouseY);
  void move(const int dx, const int dy);
};

#endif // MAIN_H_
