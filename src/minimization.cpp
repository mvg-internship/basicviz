//===----------------------------------------------------------------------===/ /
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "layout.h"

#include <vector>

typedef std::pair<TreeNode *, TreeNode *> Edge;

namespace {
  struct AdditionalNetFeatures {
    std::vector<std::vector<TreeNode::Id>> nodesByLayer;
    std::vector<std::vector<TreeNode::Id>> tempNodesByLayer;
    std::vector<std::vector<Edge>> netEdges;
    int intersections = -1;
    std::vector<int> accTree;

    void minimizeIntersections(Net &net);
    void setEdgesToOptimalCondition(Net &net);
    int crossCounting(Net &net, AdditionalNetFeatures &features);
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
  std::sort(layer.begin(), layer.end(), [&net](const auto &x,
    const auto &y) -> bool {
    return net.getNode(x)->barycentricValue < net.getNode(y)->barycentricValue;
  });
  for (int i = 0; i < layer.size(); ++i) {
    net.getNode(layer[i])->number = i;
  }
}

int getAmountOfLayers(std::vector<TreeNode> &nodes) {
  int amountOfLayers = 0;
  for (int i = 0; i < nodes.size(); ++i) {
    if (nodes[i].layer > amountOfLayers)
      amountOfLayers = nodes[i].layer;
  }
  return amountOfLayers;
}

std::vector<std::vector<TreeNode::Id>>Net::getNodesByLayer() {
  int amountOfLayers = getAmountOfLayers(nodes);
  std::vector<std::vector<TreeNode::Id>> nodesByLayer(amountOfLayers + 1);
  for (TreeNode &node: nodes) {
    std::vector<TreeNode::Id> &layer = nodesByLayer[node.layer];
    layer.push_back(node.id);
    node.number = layer.size() - 1;
  }
  return nodesByLayer;
}

int getPortIndex(TreeNode::Id id, TreeNode &node) {
  for (int i = 0; i < node.succ.size(); ++i) {
    if (node.succ[i] == id)
      return i;
  }
  for (int i = 0; i < node.pred.size(); ++i) {
    if (node.pred[i] == id)
      return i;
  }
}

void edgesByLayers(Net &net,
     std::vector<TreeNode::Id> &vec,
     TreeNode *node,
     std::vector<Edge> &edges) {
  for (TreeNode::Id &id: vec) {
    TreeNode *connectedNode = net.getNode(id);
    if (connectedNode->layer > node->layer)
      continue;
    edges.emplace_back(connectedNode, node);
  }
}

std::vector<std::vector<Edge>> getNetEdges(Net &net,
                               std::vector<std::vector<TreeNode::Id>> &tempNodesByLayer) {
  std::vector<std::vector<Edge>> netEdges;
  for (int i = 1; i < tempNodesByLayer.size(); ++i) {
    std::vector<Edge> edges;
    for (int j = 0; j < tempNodesByLayer[i].size(); ++j) {
      TreeNode *node = net.getNode(tempNodesByLayer[i][j]);
      edgesByLayers(net, node->pred, node, edges);
      edgesByLayers(net, node->succ, node, edges);
    }
    netEdges.push_back(edges);
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

int AdditionalNetFeatures::crossCounting(Net &net, AdditionalNetFeatures &features) {
  int crossCount = 0;
  for (int i = 0; i < netEdges.size(); ++i) {
    std::sort(netEdges[i].begin(), netEdges[i].end(), lexicographicSortCondition);

    const int numLeaves = nearestPow2(netEdges[i].size());
    const int firstLeafIndex = numLeaves - 1;
    const int treeSize = numLeaves * 2 - 1;

    features.accTree.clear();
    features.accTree.resize(treeSize, 0);

    for (int k = 0; k < netEdges[i].size(); k++) {
      int index = netEdges[i][k].second -> number + firstLeafIndex;
      ++features.accTree[index];
      while (index > 0) {
        if (index % 2)
          crossCount += features.accTree[index + 1];
        index = (index - 1) / 2;
        ++features.accTree[index];
      }
    }
  }
  return crossCount;
}

void getBarycentricValueForPorts(Net &net,
     std::vector<TreeNode::Id> &vec,
     bool forwardLayerSweep,
     std::vector<std::pair<float, TreeNode::Id>> &barycentricValueForPorts) {
  for (TreeNode::Id &id: vec) {
    TreeNode *node = net.getNode(id);
    float bValue;
    if (forwardLayerSweep) {
      bValue = forwardRankDefinition(*node, node->number, getPortIndex(node->id, *node)) /
               float(node->succ.size() + node->pred.size());
    } else {
      bValue = backwardRankDefinition(*node, node->number, getPortIndex(node->id, *node)) /
               float(node->succ.size() + node->pred.size());
    }
    barycentricValueForPorts.push_back({bValue, node->id});
  }
}

void sortPorts(Net &net, std::vector<TreeNode::Id> &vec, bool flag) {
  std::vector<std::pair<float, TreeNode::Id>> barycentricValueForPorts;
  getBarycentricValueForPorts(net, vec, flag, barycentricValueForPorts);
  std::sort(barycentricValueForPorts.begin(), barycentricValueForPorts.end());
  for (int k = 0; k < barycentricValueForPorts.size(); ++k) {
    vec[k] = barycentricValueForPorts[k].second;
  }
}

void portOrderOptimization(Net &net, std::vector<std::vector<TreeNode::Id>> &nodesByLayer) {
  for (int i = 0; i < nodesByLayer.size(); ++i) {
    for (int j = 0; j < nodesByLayer[i].size(); ++j) {
      TreeNode *node = net.getNode(nodesByLayer[i][j]);
      sortPorts(net, node->pred, true);
      sortPorts(net, node->succ, false);
    }
  }
}

int totalRankForFixLayer(Net &net,
    std::vector<TreeNode::Id> &vec,
    TreeNode *node,
    int direction,
    int &connectionsToAdjacentLayer) {
  int rank = 0;
  for (int k = 0; k < vec.size(); ++k) {
    if (net.getNode(vec[k])->layer + direction != node->layer)
      continue;
    connectionsToAdjacentLayer += 1;
    int index = net.getNode(vec[k])->number;
    int portIndex = getPortIndex(node->id, *net.getNode(vec[k]));
    if (direction == 1) {
      rank += forwardRankDefinition(*net.getNode(vec[k]), index, portIndex);
    } else {
      rank += backwardRankDefinition(*net.getNode(vec[k]), index, portIndex);
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

bool stopAlgorithm(Net &net, AdditionalNetFeatures &features) {
  int intersectionsAfterAlgorithm = features.crossCounting(net, features);
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

void doLayerSweeps(Net &net,
     std::vector<std::vector<TreeNode::Id>> &tempNodesByLayer,
     int direction) {
  for (int i = direction == 1 ? 1 : tempNodesByLayer.size() - 2;
       direction == 1 ? i < tempNodesByLayer.size() : i >= 0;
       i += direction) {
    for (int j = 0; j < tempNodesByLayer[i].size(); ++j) {
      barycentricValueDefinition(net, net.getNode(tempNodesByLayer[i][j]), direction);
    }
    sortNodes(net, tempNodesByLayer[i]);
  }
}

void AdditionalNetFeatures::minimizeIntersections(Net &net) {
  int direction = 1;
  while (true) {
    // forward layer sweeps: direction = 1; backwards layer sweeps direction = -1
    doLayerSweeps(net, tempNodesByLayer, direction);
    if (stopAlgorithm(net, *this))
      break;
    direction *= -1;
  }
}

void AdditionalNetFeatures::setEdgesToOptimalCondition(Net &net) {
  //fix changes after the last iteration of selection of vertex order
  for (int i = 0; i < nodesByLayer.size(); ++i) {
    for (int j = 0; j < nodesByLayer[i].size(); ++j) {
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