//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "layout.h"
#include "netfmt_bench.h"

#include <algorithm>
#include <cassert>
#include <map>

bool algorithmDFS(
    const TreeNode &node,
    std::vector<TreeNode> &nodes,
    std::vector<bool> &usedId) {
  usedId[node.id] = true;
  for (TreeNode::Id succId : node.succ) {
    if (usedId[succId]) {
      return true;
    }
    bool exist = algorithmDFS(nodes[succId], nodes, usedId);
    if (exist) {
      return true;
    }
  }
  usedId[node.id] = false;
  return false;
}

bool cycleExistsDFS(std::vector<TreeNode> &nodes) {
  if (nodes.size() == 0) {
    return false;
  }

  std::vector<bool> usedId(nodes.size(), false);
  for (size_t i = 0; i < nodes.size(); i++) {
    if (nodes[i].pred.size() == 0) {
      bool exist = algorithmDFS(nodes[i], nodes, usedId);
      if (exist) {
        return true;
      }
    }
  }
  return algorithmDFS(nodes[0], nodes, usedId);
}

struct EdgeCount {
  int degOut = 0;
  int degIn = 0;
};

void removeEdgeCount(
    int i,
    std::vector<EdgeCount> &edgeCounts,
    std::vector<TreeNode> &nodes) {
  const TreeNode &node = nodes[i];
  for (TreeNode::Id id : node.succ) {
    edgeCounts[id].degIn--;
  }
  for (TreeNode::Id id : node.pred) {
    edgeCounts[id].degOut--;
  }

  edgeCounts[i].degIn = -1;
  edgeCounts[i].degOut = -1;
}

bool comparatorId(TreeNode &first, TreeNode &second) {
  return first.id < second.id;
}

size_t dispatchSinks(
    std::vector<TreeNode> &nodes,
    std::vector<EdgeCount> &edgeCounts,
    std::vector<TreeNode::Id> &s2) {
  size_t decrlenNodes = 0;
  for (size_t i = 0; i < edgeCounts.size();) {
    if (edgeCounts[i].degOut == 0) {
      s2.push_back(i);
      removeEdgeCount(i, edgeCounts, nodes);
      decrlenNodes++;

      i = 0;
    } else {
      i++;
    }
  }
  return decrlenNodes;
}

size_t dispatchSources(
    std::vector<TreeNode> &nodes,
    std::vector<EdgeCount> &edgeCounts,
    std::vector<TreeNode::Id> &s1) {
  size_t decrlenNodes = 0;
  for (size_t i = 0; i < edgeCounts.size();) {
    if (edgeCounts[i].degIn == 0) {
      s1.push_back(i);
      removeEdgeCount(i, edgeCounts, nodes);
      decrlenNodes++;

      i = 0;
    } else {
      i++;
    }
  }
  return decrlenNodes;
}

// Finding and removing the minimum number of arcs to remove all cycles
// ADetails on the algorithm used can be found in:
// Jannis Pohlmann, Configurable Graph Drawing Algorithms for the TikZ Graphics
// Description Language, Diploma Thesis, Institute of Theoretical Computer
// Science, Universität zu Lübeck, 2011.
void greedyFAS(
    std::vector<TreeNode> &nodes,
    std::vector<std::pair<TreeNode::Id, TreeNode::Id>> &deletedEdges) {
  std::vector<TreeNode::Id> s1 = {}, s2 = {};

  std::vector<EdgeCount> edgeCounts;
  edgeCounts.reserve(nodes.size());
  for (const TreeNode &node : nodes) {
    edgeCounts.push_back({static_cast<int>(node.succ.size()),
                          static_cast<int>(node.pred.size())});
  }

  size_t lenNodes = nodes.size();
  while (lenNodes > 0) {
    lenNodes -= dispatchSinks(nodes, edgeCounts, s2);
    lenNodes -= dispatchSources(nodes, edgeCounts, s1);

    if (lenNodes > 0) {
      int max;
      for (size_t i = 0; i < edgeCounts.size(); i++) {
        if (edgeCounts[i].degOut >= 0 && edgeCounts[i].degIn >= 0) {
          max = i;
          break;
        }
      }
      for (size_t i = 0; i < edgeCounts.size(); i++) {
        if (edgeCounts[i].degOut - edgeCounts[i].degIn > max &&
            edgeCounts[i].degOut >= 0 && edgeCounts[i].degIn >= 0) {
          max = i;
        }
      }

      s1.push_back(max);
      removeEdgeCount(max, edgeCounts, nodes);

      lenNodes--;
    }
  }

  std::map<TreeNode::Id, TreeNode::Id> mp;
  int index = 0;
  for (auto it = s1.begin(), end = s1.end(); it != end; ++it) {
    mp[*it] = index++;
  }
  for (auto it = s2.rbegin(), end = s2.rend(); it != end; ++it) {
    mp[*it] = index++;
  }

  for (TreeNode &node : nodes) {
    for (int j = node.succ.size() - 1; j >= 0; j--) {
      if (mp[node.id] > mp[node.succ[j]]) {
        auto edge = std::make_pair(node.id, node.succ[j]);
        deletedEdges.push_back(edge);

        TreeNode &succ = nodes[node.succ[j]];
        succ.pred.erase(
            std::remove(succ.pred.begin(), succ.pred.end(), node.id),
            succ.pred.end());
        node.succ.erase(node.succ.begin() + j);
      }
    }
  }
}

// Assigning a layer and numbers in this layer for each vertex of the graph
void algorithmASAP(
    std::vector<TreeNode> &nodes,
    std::vector<std::pair<TreeNode::Id, TreeNode::Id>> &deletedEdges,
    std::vector<int> &lensLayer) {
  std::vector<int> removedNodes(nodes.size(), -1);

  std::vector<int> degInNodes;
  degInNodes.reserve(nodes.size());
  for (const TreeNode &node : nodes) {
    degInNodes.push_back(node.pred.size());
  }

  int nodesInLayer = 0;
  size_t removedCount = 0;
  for (size_t i = 0; removedCount < nodes.size(); i++) {
    for (size_t j = 0; j < nodes.size(); j++) {
      if (degInNodes[j] == 0 && removedNodes[j] == -1) {
        removedNodes[j] = 0;
        removedCount++;
      }
    }

    for (size_t j = 0; j < nodes.size(); j++) {
      if (removedNodes[j] == 0) {
        nodes[j].layer = i;
        nodes[j].number = nodesInLayer;
        nodesInLayer++;

        for (size_t k = 0; k < nodes[j].succ.size(); k++) {
          degInNodes[nodes[j].succ[k]]--;
        }
        removedNodes[j] = 1;
      }
    }
    lensLayer.push_back(nodesInLayer);
    nodesInLayer = 0;
  }

  for (auto [src, dst] : deletedEdges) {
    nodes[src].succ.push_back(dst);
    nodes[dst].pred.push_back(src);
  }
}

void addLineSegment(
    std::vector<TreeNode> &nodes,
    std::vector<int> &lensLayer,
    size_t idStart,
    size_t idEnd,
    bool isDownward) {
  int order;
  if (isDownward) {
    order = 1;
  } else {
    order = -1;
  }

  int idPrevSucc = nodes[idStart].succ[idEnd];
  nodes[idStart].succ[idEnd] = nodes.size();
  for (int i = nodes[idStart].layer + order;
       (i < nodes[idPrevSucc].layer && order == 1) ||
       (i > nodes[idPrevSucc].layer && order == -1);
       i += order) {
    TreeNode dummy = {};
    dummy.isDummy = true;
    dummy.id = static_cast<int>(nodes.size());
    dummy.layer = i;
    dummy.number = lensLayer[i];
    lensLayer[i]++;
    if (i == nodes[idStart].layer + order) {
      dummy.pred.push_back(idStart);
    } else {
      dummy.pred.push_back(dummy.id - 1);
    }
    if (i + order == nodes[idPrevSucc].layer) {
      dummy.succ.push_back(idPrevSucc);
      nodes[idPrevSucc].pred.push_back(dummy.id);
    } else {
      dummy.succ.push_back(dummy.id + 1);
    }
    nodes.push_back(dummy);
  }

  for (size_t i = 0; i < nodes[idPrevSucc].pred.size(); i++) {
    if (nodes[idPrevSucc].pred[i] == idStart) {
      nodes[idPrevSucc].pred.erase(nodes[idPrevSucc].pred.begin() + i);
      break;
    }
  }
}

bool addDummyNodes(std::vector<TreeNode> &nodes, std::vector<int> &lensLayer) {
  for (TreeNode &node : nodes) {
    for (size_t j = 0; j < node.succ.size(); j++) {
      int distance = nodes[node.succ[j]].layer - node.layer;
      if (distance > 1 || distance < -1) {
        addLineSegment(nodes, lensLayer, node.id, j, distance > 1);
        return false;
      }
    }
  }
  return true;
}

void addAllDummyNodes(
    std::vector<TreeNode> &nodes,
    std::vector<int> &lensLayer) {
  bool allDummysAdded = false;
  while (!allDummysAdded) {
    allDummysAdded = addDummyNodes(nodes, lensLayer);
  }
}

TreeNode::Id Net::addNode() {
  Id id = static_cast<Id>(nodes.size());
  nodes.emplace_back();
  nodes.back().id = id;

  return id;
}

const std::vector<TreeNode::Id> &Net::getSources() {
  if (!sourcesCalculated) {
    for (size_t i = 0; i < nodes.size(); i++) {
      if (nodes[i].pred.size() == 0) {
        sources.push_back(nodes[i].id);
      }
    }
  }
  return sources;
}

const std::vector<TreeNode::Id> &Net::getSinks() {
  if (!sinksCalculated) {
    for (size_t i = 0; i < nodes.size(); i++) {
      if (nodes[i].succ.size() == 0) {
        sinks.push_back(nodes[i].id);
      }
    }
  }
  return sinks;
}

const std::vector<TreeNode::Id> &Net::getSuccessors(TreeNode::Id id) const {
  const TreeNode *node = getNode(id);
  assert(node);
  return node->succ;
}

const std::vector<TreeNode::Id> &Net::getPredecessors(TreeNode::Id id) const {
  const TreeNode *node = getNode(id);
  assert(node);
  return node->pred;
}

const TreeNode *Net::getNode(TreeNode::Id id) const {
  if (id >= 0 && id < nodes.size()) {
    return nodes.data() + id;
  }
  return nullptr;
}

// Assigning a layer and a number, introducing dummy vertices
void Net::assignLayers() {
  std::vector<std::pair<TreeNode::Id, TreeNode::Id>> deletedEdges = {};
  greedyFAS(nodes, deletedEdges);

  std::vector<int> lensLayer = {};
  algorithmASAP(nodes, deletedEdges, lensLayer);

  addAllDummyNodes(nodes, lensLayer);
}

enum {
  ReductionWidth = 2,
  ReductionHeight = 4,
  ReductionRelationToGap = 2,
  GetMiddle = 2
};

void initPositionAndSize(
    std::vector<TreeNode> &nodes,
    std::vector<NormalizedElement> &normalizedElements,
    float nCellSize) {
  for (TreeNode &node : nodes) {
    NormalizedElement nElement = {};
    nElement.id = node.id;

    if (node.isDummy) {
      NormalizedPoint nPoint = {};
      nPoint.nX = nCellSize * node.number +
                  (nCellSize / ReductionWidth) / ReductionRelationToGap;
      nPoint.nY = nCellSize * node.layer +
                  (nCellSize / ReductionHeight) / ReductionRelationToGap;

      nElement.nPoint = nPoint;
      nElement.nH = 0;
      nElement.nW = 0;
    } else {
      NormalizedPoint nPoint = {};
      nPoint.nX = nCellSize * node.number;
      nPoint.nY = nCellSize * node.layer;

      nElement.nPoint = nPoint;
      nElement.nH = nCellSize / ReductionHeight;
      nElement.nW = nCellSize / ReductionWidth;
    }

    normalizedElements.push_back(nElement);
  }
}

void initConnections(
    std::vector<TreeNode> &nodes,
    std::vector<NormalizedElement> &normalizedElements) {
  int countConnections = 0;
  for (size_t i = 0; i < nodes.size(); i++) {
    for (size_t &succId : nodes[i].succ) {
      NormalizedConnection connection = {};

      connection.id = countConnections;
      connection.startElementId = nodes[i].id;
      connection.endElementId = succId;

      NormalizedPoint nPointStart = {};
      nPointStart.nX = normalizedElements[i].nPoint.nX +
                       normalizedElements[i].nW / GetMiddle;
      nPointStart.nY = normalizedElements[i].nPoint.nY +
                       normalizedElements[i].nH;

      NormalizedPoint nPointEnd = {};
      nPointEnd.nX = normalizedElements[succId].nPoint.nX +
                     normalizedElements[succId].nW / GetMiddle;
      nPointEnd.nY = normalizedElements[succId].nPoint.nY;

      connection.nVertices.push_back(nPointStart);
      connection.nVertices.push_back(nPointEnd);

      countConnections++;

      normalizedElements[i].connections.push_back(connection);
    }
  }
}

void Net::netTreeNodesToNormalizedElements(
    std::vector<NormalizedElement> &normalizedElements) {
  float maxNumber = -1, maxLayer = -1;
  for (TreeNode &node : nodes) {
    if (node.layer > maxLayer) {
      maxLayer = node.layer;
    }
    if (node.number > maxNumber) {
      maxNumber = node.number;
    }
  }
  float nCellSize = -1;

  if (maxLayer > maxNumber) {
    nCellSize = 1 / (maxLayer + 1);
  } else {
    nCellSize = 1 / (maxNumber + 1);
  }

  initPositionAndSize(nodes, normalizedElements, nCellSize);

  initConnections(nodes, normalizedElements);
}
