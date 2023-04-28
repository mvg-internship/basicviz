//===----------------------------------------------------------------------===/ /
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

// 1)Layer sweep algorithm - Algorithm for placing nodes in each layer based on the forward and backward pass of the net.
// link: https://rtsys.informatik.uni-kiel.de/~biblio/downloads/papers/jvlc13.pdf
// 2)AdditionalNetFeatures - a structure containing descriptions of vertices by layers,
// temporal distribution by vertices, and edges
// 3)crossCounting - graph edge intersection counting algorithm
// link: https://jgaa.info/accepted/2004/BarthMutzelJuenger2004.8.2.pdf
// Author: Wilhelm Barth, Michael J¨unger, and Petra Mutzel.
#include "minimization.h"
#include "layout.h"

#include <vector>
#include <algorithm>

typedef std::pair<TreeNode *, TreeNode *> Edge;

namespace {
  struct AdditionalNetFeatures {
    std::vector<std::vector<TreeNode::Id>> nodesByLayer;
//
    std::vector<std::vector<TreeNode::Id>> tempNodesByLayer;
    std::vector<std::vector<Edge>> netEdges;
    int intersections = -1;
    std::vector<int> accTree;

    void minimizeIntersections(Net &net);
    void setEdgesToOptimalCondition(Net &net);
    int crossCounting();
  };
}

float forwardRankDefinition(TreeNode &node, int nodeIndex, int portIndex) {
  return nodeIndex + portIndex / (node.succ.size() + 1);
}

float backwardRankDefinition(TreeNode &node, int nodeIndex, int portIndex) {
  int portOrderValue;
  int maxPortIndex = node.pred.size() + node.succ.size();
  if (portIndex <= maxPortIndex) {
    portOrderValue = maxPortIndex - portIndex + 1;
  } else {
    portOrderValue = maxPortIndex + node.succ.size() - portIndex + 1;
  }
  return nodeIndex + portOrderValue / (node.pred.size() + 1);
}

void sortNodes(Net &net, std::vector<TreeNode::Id> &layer) {
  std::sort(layer.begin(), layer.end(), [&net](auto x, auto y) -> bool {
    return net.getNode(x)->barycentricValue < net.getNode(y)->barycentricValue;
  });
  for (size_t i = 0; i < layer.size(); ++i) {
    net.getNode(layer[i])->number = i;
  }
}

int getAmountOfLayers(std::vector<TreeNode> &nodes) {
  int amountOfLayers = 0;
  for (size_t i = 0; i < nodes.size(); ++i) {
    if (nodes[i].layer > amountOfLayers)
      amountOfLayers = nodes[i].layer;
  }
  return amountOfLayers;
}

std::vector<std::vector<TreeNode::Id>> Net::getNodesByLayer() {
  int amountOfLayers = getAmountOfLayers(nodes);
  std::vector<std::vector<TreeNode::Id>> nodesByLayer(amountOfLayers + 1);
  for (TreeNode &node: nodes) {
    std::vector<TreeNode::Id> &layer = nodesByLayer[node.layer];
    layer.push_back(node.id);
    node.number = layer.size() - 1;
  }
  return nodesByLayer;
}

void edgesByLayers(Net &net,
                   std::vector<TreeNode::Id> &vec,
                   TreeNode *node,
                   std::vector<Edge> &edges) {
  for (TreeNode::Id id: vec) {
    TreeNode *connectedNode = net.getNode(id);
    if (connectedNode->layer > node->layer)
      continue;
    edges.emplace_back(connectedNode, node);
  }
}

std::vector<std::vector<Edge>> getNetEdges(Net &net,
                                           std::vector<std::vector<TreeNode::Id>> &tempNodesByLayer) {
  std::vector<std::vector<Edge>> netEdges;
  for (size_t i = 1; i < tempNodesByLayer.size(); ++i) {
    std::vector<Edge> edges;
    for (size_t j = 0; j < tempNodesByLayer[i].size(); ++j) {
      TreeNode *node = net.getNode(tempNodesByLayer[i][j]);
      edgesByLayers(net, node->pred, node, edges);
      edgesByLayers(net, node->succ, node, edges);
    }
    netEdges.push_back(std::move(edges));
  }
  return netEdges;
}

bool lexicographicSortCondition(const Edge &edge1, const Edge &edge2) {
  if (edge1.first->number == edge2.first->number) {
    return edge1.second->number < edge2.second->number;
  } else {
    return edge1.first->number < edge2.first->number;
  }
}

int nearestPow2(int x) {
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return ++x;
}

int AdditionalNetFeatures::crossCounting() {
  int crossCount = 0;
  for (size_t i = 0; i < netEdges.size(); ++i) {
    std::sort(netEdges[i].begin(), netEdges[i].end(), lexicographicSortCondition);

    const int numLeaves = nearestPow2(netEdges[i].size());
    const int firstLeafIndex = numLeaves - 1;
    const int treeSize = numLeaves * 2 - 1;

    accTree.clear();
    accTree.resize(treeSize, 0);

    for (size_t k = 0; k < netEdges[i].size(); k++) {
      int index = netEdges[i][k].second -> number + firstLeafIndex;
      ++accTree[index];
      while (index > 0) {
        if (index % 2)
          crossCount += accTree[index + 1];
        index = (index - 1) / 2;
        ++accTree[index];
      }
    }
  }
  return crossCount;
}

void getBarycentricValueForPorts(Net &net,
                                 std::vector<TreeNode::Id> &vec,
                                 bool forwardLayerSweep,
                                 std::vector<std::pair<float, TreeNode::Id>> &barycentricValueForPorts) {
  for (size_t i = 0; i < vec.size(); ++i) {
    TreeNode *node = net.getNode(vec[i]);
    float bValue;
    if (forwardLayerSweep) {
      bValue = forwardRankDefinition(*node, node->number, i);
    } else {
      bValue = backwardRankDefinition(*node, node->number, i);
    }
    bValue /= float(node->succ.size() + node->pred.size());
    barycentricValueForPorts.push_back({bValue, node->id});
  }
}

void sortPorts(Net &net, std::vector<TreeNode::Id> &vec, bool flag) {
  std::vector<std::pair<float, TreeNode::Id>> barycentricValueForPorts;
  getBarycentricValueForPorts(net, vec, flag, barycentricValueForPorts);
  std::sort(barycentricValueForPorts.begin(), barycentricValueForPorts.end());
  for (size_t k = 0; k < barycentricValueForPorts.size(); ++k) {
    vec[k] = barycentricValueForPorts[k].second;
  }
}

void portOrderOptimization(Net &net, std::vector<std::vector<TreeNode::Id>> &nodesByLayer) {
  for (size_t i = 0; i < nodesByLayer.size(); ++i) {
    for (size_t j = 0; j < nodesByLayer[i].size(); ++j) {
      TreeNode *node = net.getNode(nodesByLayer[i][j]);
      sortPorts(net, node->pred, true);
      sortPorts(net, node->succ, false);
    }
  }
}

int totalRankForFixLayer(
    Net &net,
    std::vector<TreeNode::Id> &vec,
    TreeNode *node,
    int direction,
    int &connectionsToAdjacentLayer) {
  int rank = 0;
  for (size_t k = 0; k < vec.size(); ++k) {
    if (net.getNode(vec[k])->layer + direction != node->layer)
      continue;
    connectionsToAdjacentLayer += 1;
    int index = net.getNode(vec[k])->number;
    if (direction == 1) {
      rank += forwardRankDefinition(*net.getNode(vec[k]), index, k);
    } else {
      rank += backwardRankDefinition(*net.getNode(vec[k]), index, k);
    }
  }
  return rank;
}

void barycentricValueDefinition(Net &net, TreeNode *node, int direction) {
  int connectionsToAdjacentLayer = 0;
  float rank = 0;
  rank += totalRankForFixLayer(net, node->pred, node, direction, connectionsToAdjacentLayer);
  rank += totalRankForFixLayer(net, node->succ, node, direction, connectionsToAdjacentLayer);
  node->barycentricValue = rank / connectionsToAdjacentLayer;
}

bool stopAlgorithm(AdditionalNetFeatures &features) {
  int intersectionsAfterAlgorithm = features.crossCounting();
  if (features.intersections > intersectionsAfterAlgorithm || features.intersections == -1) {
    features.intersections = intersectionsAfterAlgorithm;
    std::copy(
        features.tempNodesByLayer.begin(),
        features.tempNodesByLayer.end(),
        features.nodesByLayer.begin());
    return false;
  } else {
    return true;
  }
}

void doLayerSweep(Net &net,
                  std::vector<TreeNode::Id> &layer,
                  int direction) {
  for (size_t j = 0; j < layer.size(); ++j) {
    barycentricValueDefinition(net, net.getNode(layer[j]), direction);
  }
  sortNodes(net, layer);
}

void AdditionalNetFeatures::minimizeIntersections(Net &net) {
  int direction = 1;
  while (true) {
    // forward layer sweeps: direction = 1; backwards layer sweeps direction = -1
    if (direction > 0) {
      for (auto it = tempNodesByLayer.begin() + 1, end = tempNodesByLayer.end(); it != end; ++it) {
        doLayerSweep(net, *it, direction);
      }
    } else {
      for (auto it = tempNodesByLayer.rbegin() - 1, end = tempNodesByLayer.rend(); it != end; ++it) {
        doLayerSweep(net, *it, direction);
      }
    }
    if (stopAlgorithm(*this))
      break;
    direction *= -1;
  }
}

void AdditionalNetFeatures::setEdgesToOptimalCondition(Net &net) {
  //fix changes after the last iteration of selection of vertex order
  for (size_t i = 0; i < nodesByLayer.size(); ++i) {
    for (size_t j = 0; j < nodesByLayer[i].size(); ++j) {
      net.getNode(nodesByLayer[i][j])->number = j;
    }
  }
}

void layerSweepAlgorithm(Net &net) {
  AdditionalNetFeatures features;

  features.nodesByLayer = net.getNodesByLayer();
  features.tempNodesByLayer.resize(features.nodesByLayer.size());
  std::copy(
      features.nodesByLayer.begin(),
      features.nodesByLayer.end(),
      features.tempNodesByLayer.begin());

  features.netEdges = getNetEdges(net, features.tempNodesByLayer);

  features.minimizeIntersections(net);

  features.setEdgesToOptimalCondition(net);

  portOrderOptimization(net, features.nodesByLayer);

  printf("\nintersections: %i\n", features.intersections);
}