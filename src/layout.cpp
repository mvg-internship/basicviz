#include <iostream>

#include "layout.h"
#include "netfmt_bench.h"

#include <cassert>
#include <algorithm>

bool algorithmDFS(
    const TreeNode &node,
    std::vector<TreeNode> &nodes,
    std::vector<bool> &usedId) {
  usedId[node.id] = true;
  for (TreeNode::nodeId succId: node.succ) {
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

  for (int i = 0; i < nodes.size(); i++) {
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

void removeEdgeCount(int i, std::vector<EdgeCount> &edgeCounts,
                     std::vector<TreeNode> &nodes) {
  const TreeNode &node = nodes[i];
  for (TreeNode::nodeId id: node.succ) {
    edgeCounts[id].degIn--;
  }
  for (TreeNode::nodeId id: node.pred) {
    edgeCounts[id].degOut--;
  }

  edgeCounts[i].degIn = -1;
  edgeCounts[i].degOut = -1;
}

bool comparatorId(TreeNode &first, TreeNode &second) {
    return first.id < second.id;
}

void greedyFAS(
    std::vector<TreeNode> &nodes,
    std::vector<std::pair<TreeNode::nodeId, TreeNode::nodeId>> &deletedEdges) {
  std::vector<TreeNode::nodeId> s1 = {}, s2 = {};
  
    std::vector<EdgeCount> edgeCounts;
    edgeCounts.reserve(nodes.size());
    for (const TreeNode &node: nodes) {
        edgeCounts.push_back({ static_cast<int>(node.succ.size()), static_cast<int>(node.pred.size()) });
    }
    
  size_t lenNodes = nodes.size();
  while (lenNodes > 0) {
    for (int i = 0; i < edgeCounts.size(); i++) {
      if (edgeCounts[i].degOut == 0) {
        s2.push_back(i);
        removeEdgeCount(i, edgeCounts, nodes);
        lenNodes--;

        i = -1;
      }
    }

    for (int i = 0; i < edgeCounts.size(); i++) {
      if (edgeCounts[i].degIn == 0) {
        s1.push_back(i);
        removeEdgeCount(i, edgeCounts, nodes);
        lenNodes--;

        i = -1;
      }
    }

    if (lenNodes > 0) {
      int max;
      for (int i = 0; i < edgeCounts.size(); i++) {
        if (edgeCounts[i].degOut >= 0 && edgeCounts[i].degIn >= 0) {
          max = i;
          break;
        }
      }
      for (int i = 0; i < edgeCounts.size(); i++) {
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

    std::vector<TreeNode::nodeId> &ordered = s1;
    ordered.insert(ordered.end(), s2.rbegin(), s2.rend());

    for (int i = 0; i < ordered.size(); i++) {
        for (int j = 0; j < nodes.size(); j++) {
            if (nodes[j].id == ordered[i]) {
                std::swap(nodes[i], nodes[j]);
            }
        }
    }

  for (int i = 0; i < nodes.size(); i++) {
    for (int j = nodes[i].succ.size() - 1; j >= 0; j--) {
      for (int k = 0; k < i; k++) {
        if (nodes[i].succ[j] == nodes[k].id) {
          auto edge = std::make_pair(nodes[i].id, nodes[i].succ[j]);
          deletedEdges.push_back(edge);

          nodes[i].succ.erase(nodes[i].succ.begin() + j);
        }
      }
    }
  }

  std::sort(nodes.begin(), nodes.end(), comparatorId);
    
  for (int i = 0; i < nodes.size(); i++) {
    nodes[i].pred.clear();
  }
  for (int i = 0; i < nodes.size(); i++) {
    for (int j = 0; j < nodes[i].succ.size(); j++) {
      nodes[nodes[i].succ[j]].pred.push_back(i);
    }
  }
}

void algorithmASAP(
    std::vector<TreeNode> &nodes,
    std::vector<std::pair<TreeNode::nodeId, TreeNode::nodeId>> &deletedEdges,
    std::vector<int> &lensLayer) {
  std::vector<int> removedNodes(nodes.size(), -1);

    std::vector<int> degInNodes;
    degInNodes.reserve(nodes.size());
    for (const TreeNode &node: nodes) {
        degInNodes.push_back(node.pred.size());
    }

  int nodesInLayer = 0;
  int removedCount = 0;
  for (int i = 0; removedCount < nodes.size(); i++) {
    for (int j = 0; j < nodes.size(); j++) {
      if (degInNodes[j] == 0 && removedNodes[j] == -1) {
        removedNodes[j] = 0;
        removedCount++;
      }
    }

    for (int j = 0; j < nodes.size(); j++) {
      if (removedNodes[j] == 0) {
        nodes[j].layer = i;
        nodes[j].number = nodesInLayer;
        nodesInLayer++;

        for (int k = 0; k < nodes[j].succ.size(); k++) {
          degInNodes[nodes[j].succ[k]]--;
        }
        removedNodes[j] = 1;
      }
    }
    lensLayer.push_back(nodesInLayer);
    nodesInLayer = 0;
  }

  for (int i = 0; i < deletedEdges.size(); i++) {
    nodes[deletedEdges[i].first].succ.push_back(deletedEdges[i].second);
    nodes[deletedEdges[i].second].pred.push_back(deletedEdges[i].first);
  }
}

void addLineSegment(std::vector<TreeNode> &nodes, std::vector<int> &lensLayer,
                    int idStart, int idEnd, int order) {
  int idPrevSucc = nodes[idStart].succ[idEnd];
  nodes[idStart].succ[idEnd] = nodes.size();
  for (int i = nodes[idStart].layer + order;
       (i < nodes[idPrevSucc].layer && order == 1) ||
       (i > nodes[idPrevSucc].layer && order == -1);
       i = i + order) {
    TreeNode dummy = {};
    dummy.isDummy = true;
    dummy.id = static_cast<int>(nodes.size());
    dummy.layer = i;
    dummy.number = lensLayer[i];
    lensLayer[i]++;
    if (i + order == nodes[idPrevSucc].layer) {
      dummy.succ.push_back(idPrevSucc);
    } else {
      dummy.succ.push_back(dummy.id + 1);
    }
    nodes.push_back(dummy);
  }
}

bool addDummyNodes(std::vector<TreeNode> &nodes, std::vector<int> &lensLayer) {
  for (int i = 0; i < nodes.size(); i++) {
    for (int j = 0; j < nodes[i].succ.size(); j++) {
      if ((nodes[nodes[i].succ[j]].layer - nodes[i].layer > 1) || (nodes[nodes[i].succ[j]].layer - nodes[i].layer < -1)) {
        int order = -1;
        if (nodes[nodes[i].succ[j]].layer - nodes[i].layer > 1) {
          order = 1;
        }
        addLineSegment(nodes, lensLayer, i, j, order);
        return false;
      }
    }
  }
  return true;
}

void addAllDummyNodes(std::vector<TreeNode> &nodes,
                      std::vector<int> &lensLayer) {
  bool allDummysAdded = false;
  while (!allDummysAdded) {
    allDummysAdded = addDummyNodes(nodes, lensLayer);
  }
}

TreeNode::nodeId Net::addNode() {
  nodeId id = static_cast<nodeId>(nodes.size());
  nodes.emplace_back();
  nodes.back().id = id;

  return id;
}

const std::vector<TreeNode::nodeId> &Net::getSources() {
  if (!sourcesCalculated) {
    for (int i = 0; i < nodes.size(); i++) {
      if (nodes[i].pred.size() == 0) {
        sources.push_back(nodes[i].id);
      }
    }
  }
  return sources;
}

const std::vector<TreeNode::nodeId> &Net::getSinks() {
  if (!sinksCalculated) {
    for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i].succ.size() == 0) {
        sinks.push_back(nodes[i].id);
      }
    }
  }
  return sinks;
}

const std::vector<TreeNode::nodeId> &
Net::getSuccessors(TreeNode::nodeId id) const {
  const TreeNode *node = getNode(id);
  assert(node);
  return node->succ;
}

const std::vector<TreeNode::nodeId> &
Net::getPredecessors(TreeNode::nodeId id) const {
  const TreeNode *node = getNode(id);
  assert(node);
  return node->pred;
}

const TreeNode *Net::getNode(TreeNode::nodeId id) const {
  if (id >= 0 && id < nodes.size()) {
    return nodes.data() + id;
  }
  return nullptr;
}

void Net::transformationGraph() {
  std::vector<std::pair<TreeNode::nodeId, TreeNode::nodeId>> deletedEdges = {};
  greedyFAS(nodes, deletedEdges);
 
  std::vector<int> lensLayer = {};
  algorithmASAP(nodes, deletedEdges, lensLayer);

  addAllDummyNodes(nodes, lensLayer);
    
  for (int i = 0; i < nodes.size(); i++) {
    nodes[i].pred.clear();
  }
  for (int i = 0; i < nodes.size(); i++) {
    for (int j = 0; j < nodes[i].succ.size(); j++) {
      nodes[nodes[i].succ[j]].pred.push_back(i);
    }
  }
}

enum libConstants {
    REDUCTION_WIDTH = 2,
    REDUCTION_HEIGHT = 4,
    REDUCTION_RELATION_TO_GAP = 2,
    GET_MIDDLE = 2
};

void initPositionAndSize(std::vector<TreeNode> &nodes, std::vector<NormalizedElement> &normalizedElements, float nCellSize) {
    for (TreeNode &node : nodes) {
      NormalizedElement nElement = {};
      nElement.id = node.id;

      if (node.isDummy) {
        NormalizedPoint nPoint = {};
        nPoint.nX = nCellSize * node.number + (nCellSize / REDUCTION_WIDTH) / REDUCTION_RELATION_TO_GAP;
        nPoint.nY = nCellSize * node.layer + (nCellSize / REDUCTION_HEIGHT) / REDUCTION_RELATION_TO_GAP;

        nElement.nPoint = nPoint;
        nElement.nH = 0;
        nElement.nW = 0;
      } else {
        NormalizedPoint nPoint = {};
        nPoint.nX = nCellSize * node.number;
        nPoint.nY = nCellSize * node.layer;

        nElement.nPoint = nPoint;
        nElement.nH = nCellSize / REDUCTION_HEIGHT;
        nElement.nW = nCellSize / REDUCTION_WIDTH;
      }

      normalizedElements.push_back(nElement);
    }
}

void initConnections(std::vector<TreeNode> &nodes, std::vector<NormalizedElement> &normalizedElements) {
    int countConnections = 0;
    for (int i = 0; i < nodes.size(); i++) {
      for (int &succId : nodes[i].succ) {
        NormalizedConnection connection = {};

        connection.id = countConnections;
        connection.startElementId = nodes[i].id;
        connection.endElementId = succId;

        NormalizedPoint nPointStart = {};
        nPointStart.nX =
            normalizedElements[i].nPoint.nX + normalizedElements[i].nW / GET_MIDDLE;
        nPointStart.nY =
            normalizedElements[i].nPoint.nY + normalizedElements[i].nH;

        NormalizedPoint nPointEnd = {};
        nPointEnd.nX = normalizedElements[succId].nPoint.nX +
                       normalizedElements[succId].nW / GET_MIDDLE;
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
