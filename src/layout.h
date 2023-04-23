//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#ifndef LAYOUT_H_
#define LAYOUT_H_

#include <vector>

#include "main.h"

struct TreeNode {
  using Id = int;

  std::vector<Id> succ = {};
  std::vector<Id> pred = {};
  Id id = 0;
  int layer = 0;
  int number = 0;
  bool isDummy = false;
  float barycentricValue = 0;
};

struct Net {
  using Id = TreeNode::Id;

private:
  std::vector<TreeNode> nodes;
  std::vector<TreeNode::Id> sources = {};
  std::vector<TreeNode::Id> sinks = {};
  bool sourcesCalculated = false;
  bool sinksCalculated = false;

public:
  const std::vector<Id> &getSources();
  const std::vector<Id> &getSinks();

  const std::vector<Id> &getSuccessors(Id id) const;
  const std::vector<Id> &getPredecessors(Id id) const;

  Id addNode();

  TreeNode *getNode(Id id) {
    const Net &net = *this;
    return const_cast<TreeNode *>(net.getNode(id));
  }

  const TreeNode *getNode(Id id) const;

  void assignLayers();
  void netTreeNodesToNormalizedElements(
      std::vector<NormalizedElement> &normalizedElements);

  std::vector<std::vector<TreeNode::Id>> &getNodesByLayer();
};

#endif // LAYOUT_H_
