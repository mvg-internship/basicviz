//===----------------------------------------------------------------------===/ /
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "layout.h"

#include <vector>

float forwardRankDefinition(TreeNode node, int nodeIndex, int portIndex){
  return nodeIndex + portIndex / (node.succ.size() + 1);
}

float backwardRankDefinition(TreeNode node, int nodeIndex, int portIndex) {
  int portOrderValue;
  int maxPortIndex = node.pred.size() + node.succ.size();
  if (portIndex <= maxPortIndex) {
    portOrderValue = maxPortIndex - portIndex + 1;
  } else {
    portOrderValue = maxPortIndex + node.succ.size() - portIndex + 1;
  }
  return nodeIndex + portOrderValue / (node.pred.size() + 1);
}

void sortNodes(Net &net, std::vector<TreeNode::nodeId> &layer) {
  for (int i = 0; i < layer.size() - 1; ++i) {
    for (int j = 0; j < layer.size() - i - 1; ++j) {
      if (net.getNode(layer[j])->barycentricValue > net.getNode(layer[j + 1])->barycentricValue) {
        std::swap(net.getNode(layer[j])->number, net.getNode(layer[j + 1])->number);
        std::swap(layer[j], layer[j + 1]);
      }
    }
  }
}

int getAmountOfLayers(std::vector < TreeNode > &nodes) {
  int amountOfLayers = 0;
  for (int i = 0; i < nodes.size(); ++i) {
    if (nodes[i].layer > amountOfLayers)
      amountOfLayers = nodes[i].layer;
    }
  return amountOfLayers;
}

std::vector<std::vector<TreeNode::nodeId>> Net::getNodesByLayer() {
  int amountOfLayers = getAmountOfLayers(this->nodes);
  std::vector<std::vector<TreeNode::nodeId>> nodesByLayer(amountOfLayers + 1);
  for (TreeNode &node: nodes) {
    std::vector<TreeNode::nodeId> &layer = nodesByLayer[node.layer];
    layer.push_back(node.id);
    node.number = layer.size() - 1;
  }
  return nodesByLayer;
}

int getPortIndex(TreeNode::nodeId id, TreeNode node) {
  for (int i = 0; i < node.succ.size(); ++i) {
    if (node.succ[i] == id)
      return i;
    }
  for (int i = 0; i < node.pred.size(); ++i) {
    if (node.pred[i] == id)
      return i;
    }
}

void makeEdges(Net &net, std::vector<TreeNode::nodeId> vec, int j, TreeNode *node, std::vector<std::pair<int, int>> &edges){
  for (TreeNode::nodeId &id : vec) {
  TreeNode *connectedNode = net.getNode(id);
  if (connectedNode->layer > node->layer)
    continue;
  edges.emplace_back(connectedNode->number, j);
  }
}

int crossCounting(Net &net, std::vector<std::vector<TreeNode::nodeId>> &tempNodesByLayer) {
  int crossNum = 0;
  for (int i = 1; i < tempNodesByLayer.size(); ++i) {
    std::vector<std::pair<int, int>> edges;
    for (int j = 0; j < tempNodesByLayer[i].size(); ++j) {
      TreeNode *node = net.getNode(tempNodesByLayer[i][j]);
      getEdges(net, node->pred, j, node, edges);
      getEdges(net, node->succ, j, node, edges);
    }
    std::sort(edges.begin(), edges.end());
    for (int j = 0; j < edges.size() - 1; ++j) {
      for (int k = 0; k < edges.size() - 1; ++k) {
        if (edges[k].second > edges[k + 1].second) {
          std::swap(edges[k], edges[k + 1]);
          crossNum += 1;
        }
      }
    }
  }
  return crossNum;
}

std::vector<std::pair<float, TreeNode::nodeId>> getBarycentricValueForPorts(Net &net, std::vector<TreeNode::nodeId> &vec, bool forwardLayerSweep){
  std::vector<std::pair<float, TreeNode::nodeId>> b;
  for (TreeNode::nodeId &id : vec) {
    TreeNode *node = net.getNode(id);
    float bValue;
    if (forwardLayerSweep){
      bValue = forwardRankDefinition(*node,node->number,
        getPortIndex(node->id,*node))/
        float(node->succ.size()+node->pred.size());
    } else{
      bValue = backwardRankDefinition(*node, node->number,
        getPortIndex(node->id, *node)) /
        float(node->succ.size() + node->pred.size());
    }
    b.push_back({ bValue, node->id });
  }
  return b;
}

void sortPorts(Net& net, std::vector<std::vector<TreeNode::nodeId>>& nodesByLayer) {
  for (int i = 1; i < nodesByLayer.size(); ++i) {
    for (int j = 0; j < nodesByLayer[i].size(); ++j) {
      TreeNode *node = net.getNode(nodesByLayer[i][j]);
      std::vector<std::pair<float, TreeNode::nodeId>> bPred, bSucc;

      bPred = getBarycentricValueForPorts(net, node->pred,true);
      bSucc = getBarycentricValueForPorts(net, node->succ,false);

      std::sort(bPred.begin(), bPred.end());
      std::sort(bSucc.begin(), bSucc.end());

      for (int k = 0; k < bPred.size(); ++k) {
        node->pred[k] = bPred[k].second;
      }
      for (int k = 0; k < bSucc.size(); ++k) {
        node->succ[k] = bSucc[k].second;
      }
    }
  }
}

int totalRankForFixLayer(Net &net, std::vector<TreeNode::nodeId> &vec, TreeNode *node, int increment, int &connectionsToAdjacentLayer){
  int rank = 0;
  for (int k = 0; k < vec.size(); ++k) {
    if (net.getNode(vec[k])->layer + increment != node->layer)
      continue;
    connectionsToAdjacentLayer += 1;
    int index = net.getNode(vec[k])->number;
    int portIndex = getPortIndex(node->id, *net.getNode(vec[k]));
    if (increment == 1) {
      rank += forwardRankDefinition( *net.getNode(vec[k]), index, portIndex);
    } else {
      rank += backwardRankDefinition( *net.getNode(vec[k]), index, portIndex);
    }
  }
  return rank;
}

void barycentricValueDefinition(Net &net, TreeNode *node, int increment) {
  int connectionsToAdjacentLayer = 0;
  float rank = 0;
  rank += totalRankForFixLayer(net, node->pred, node, increment,connectionsToAdjacentLayer);
  rank += totalRankForFixLayer(net, node->succ, node, increment,connectionsToAdjacentLayer);
  node->barycentricValue = rank / connectionsToAdjacentLayer;
}

bool stopAlgorithm(Net &net, std::vector<std::vector<TreeNode::nodeId>> &tempNodesByLayer, int &intersections, std::vector<std::vector<TreeNode::nodeId>> &nodesByLayer){
  int intersectionsAfterAlgorithm = crossCounting(net, tempNodesByLayer);
  if (intersections > intersectionsAfterAlgorithm || intersections == -1) {
    intersections = intersectionsAfterAlgorithm;
    std::copy(tempNodesByLayer.begin(), tempNodesByLayer.end(), nodesByLayer.begin());
    return false;
  } else {
    return true;
  }
}

void layerSweepAlgorithm(Net &net) {
  std::vector<std::vector<TreeNode::nodeId>> nodesByLayer = net.getNodesByLayer();
  std::vector<std::vector<TreeNode::nodeId>> tempNodesByLayer(nodesByLayer.size());
  std::copy(nodesByLayer.begin(), nodesByLayer.end(), tempNodesByLayer.begin());

  int intersections = -1;
  while (true) {
    //            forward layer sweeps
    for (int i = 1; i < tempNodesByLayer.size(); ++i) {
      for (int j = 0; j < tempNodesByLayer[i].size(); ++j) {
        barycentricValueDefinition(net, net.getNode(tempNodesByLayer[i][j]), 1);
      }
      sortNodes(net, tempNodesByLayer[i]);
    }
    if (stopAlgorithm(net, tempNodesByLayer, intersections, nodesByLayer))
        break;

    //            backwards layer sweeps
    for (int i = tempNodesByLayer.size() - 2; i >= 0; i--) {
      for (int j = 0; j < tempNodesByLayer[i].size(); ++j) {
        barycentricValueDefinition(net, net.getNode(tempNodesByLayer[i][j]), -1);
      }
        sortNodes(net, tempNodesByLayer[i]);
    }
    if (stopAlgorithm(net, tempNodesByLayer, intersections, nodesByLayer))
      break;
  }

  for (int i = 0; i < nodesByLayer.size(); ++i) {
    for (int j = 0; j < nodesByLayer[i].size(); ++j) {
      net.getNode(
        nodesByLayer[i][j])->number = j; //fix changes after the last iteration of selection of vertex order
    }
  }
  sortPorts(net, nodesByLayer);
  printf("\nintersections: %i\n", intersections);
}