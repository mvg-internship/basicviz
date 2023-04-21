#include <iostream>

#include "layout.h"
#include "netfmt_bench.h"

#include <cassert>

void initIdAndSucc(std::vector<std::vector<int>> &input,
                   std::vector<TreeNode> &nodes) {
  for (int i = 0; i < input.size(); i++) {
    TreeNode node = {};
    nodes.push_back(node);
    nodes[i].id = i;
    for (int j = 0; j < input[i].size(); j++) {
      nodes[i].succ.push_back(input[i][j]);
    }
  }
}

void predReinit(std::vector<TreeNode> &nodes) {
  for (int i = 0; i < nodes.size(); i++) {
    nodes[i].pred.clear();
  }

  for (int i = 0; i < nodes.size(); i++) {
    for (int j = 0; j < nodes[i].succ.size(); j++) {
      nodes[nodes[i].succ[j]].pred.push_back(i);
    }
  }
}

void degReinit(std::vector<TreeNode> &nodes) {
  for (int i = 0; i < nodes.size(); i++) {
    nodes[i].degP = 0;
    nodes[i].degM = 0;
  }

  for (int i = 0; i < nodes.size(); i++) {
    for (int j = 0; j < nodes[i].succ.size(); j++) {
      nodes[i].degP++;
      nodes[nodes[i].succ[j]].degM++;
    }
  }
}

void heapify(std::vector<TreeNode> &nodes, int n, int i) {
  int largest = i;
  int l = 2 * i + 1;
  int r = 2 * i + 2;

  if (l < n && nodes[l].id > nodes[largest].id) {
    largest = l;
  }

  if (r < n && nodes[r].id > nodes[largest].id) {
    largest = r;
  }

  if (largest != i) {
    std::swap(nodes[i], nodes[largest]);
    heapify(nodes, n, largest);
  }
}

void idHeapSort(std::vector<TreeNode> &nodes) {
  int n = 0;
  for (int i = 0; i < nodes.size(); i++) {
    n++;
  }

  for (int i = n / 2 - 1; i >= 0; i--) {
    heapify(nodes, n, i);
  }

  for (int i = n - 1; i >= 0; i--) {
    std::swap(nodes[0], nodes[i]);
    heapify(nodes, i, 0);
  }
}

bool algorithmDFS(TreeNode::nodeId id, std::vector<TreeNode> &nodes,
                  std::vector<bool> &usedId) {
  usedId[id] = true;

  for (int i = 0; i < nodes[id].succ.size(); i++) {
    if (usedId[nodes[id].succ[i]]) {
      return true;
    } else {
      bool exist = algorithmDFS(nodes[id].succ[i], nodes, usedId);
      if (exist) {
        return true;
      }
    }
  }

  usedId[id] = false;
  return false;
}

bool cycleExistsDFS(std::vector<TreeNode> &nodes) {
  degReinit(nodes);

  std::vector<bool> usedId = {};
  usedId.resize(nodes.size(), false);

  for (int i = 0; i < nodes.size(); i++) {
    if (nodes[i].degM == 0) {
      bool exist = algorithmDFS(nodes[i].id, nodes, usedId);
      if (exist) {
        return true;
      }
    }
  }

  return algorithmDFS(nodes[0].id, nodes, usedId);
}

struct EdgeCount {
  int degP = 0;
  int degM = 0;
};

void removeEdgeCount(int i, std::vector<EdgeCount> &edgeCounts,
                     std::vector<TreeNode> &nodes) {
  for (int j = 0; j < nodes[i].succ.size(); j++) {
    edgeCounts[nodes[i].succ[j]].degM--;
  }
  for (int j = 0; j < nodes[i].pred.size(); j++) {
    edgeCounts[nodes[i].pred[j]].degP--;
  }

  edgeCounts[i].degM = -1;
  edgeCounts[i].degP = -1;
}

void greedyFAS(
    std::vector<TreeNode> &nodes,
    std::vector<std::pair<TreeNode::nodeId, TreeNode::nodeId>> &deletedEdges) {
  degReinit(nodes);
  predReinit(nodes);
  std::vector<TreeNode::nodeId> s1 = {}, s2 = {};
  std::vector<EdgeCount> edgeCounts = {};
  for (int i = 0; i < nodes.size(); i++) {
    EdgeCount edgeCount = {};
    edgeCount.degP = nodes[i].degP;
    edgeCount.degM = nodes[i].degM;
    edgeCounts.push_back(edgeCount);
  }
  int lenNodes = static_cast<int>(nodes.size());
  while (lenNodes > 0) {
    for (int i = 0; i < edgeCounts.size(); i++) {
      if (edgeCounts[i].degP == 0) {
        s2.insert(s2.cbegin(), i);
        removeEdgeCount(i, edgeCounts, nodes);
        lenNodes--;

        i = -1;
      }
    }

    for (int i = 0; i < edgeCounts.size(); i++) {
      if (edgeCounts[i].degM == 0) {
        s1.push_back(i);
        removeEdgeCount(i, edgeCounts, nodes);
        lenNodes--;

        i = -1;
      }
    }

    if (lenNodes > 0) {
      int max;
      for (int i = 0; i < edgeCounts.size(); i++) {
        if (edgeCounts[i].degP >= 0 && edgeCounts[i].degM >= 0) {
          max = i;
          break;
        }
      }
      for (int i = 0; i < edgeCounts.size(); i++) {
        if (edgeCounts[i].degP - edgeCounts[i].degM > max &&
            edgeCounts[i].degP >= 0 && edgeCounts[i].degM >= 0) {
          max = i;
        }
      }

      s1.push_back(max);
      removeEdgeCount(max, edgeCounts, nodes);

      lenNodes--;
    }
  }

  lenNodes = static_cast<int>(nodes.size());
  for (int i = 0; i < s1.size(); i++) {
    nodes.push_back(nodes[s1[i]]);
  }
  for (int i = 0; i < s2.size(); i++) {
    nodes.push_back(nodes[s2[i]]);
  }
  for (int i = 0; i < lenNodes; i++) {
    nodes.erase(nodes.cbegin());
  }

  for (int i = 0; i < nodes.size(); i++) {
    for (int j = 0; j < nodes[i].succ.size(); j++) {
      for (int k = 0; k < i; k++) {
        if (nodes[i].succ[j] == nodes[k].id) {
          auto edge = std::make_pair(nodes[i].id, nodes[i].succ[j]);
          deletedEdges.push_back(edge);

          nodes[i].succ.erase(nodes[i].succ.begin() + j);
        }
      }
    }
  }

  idHeapSort(nodes);
  degReinit(nodes);
}

void algorithmASAP(
    std::vector<TreeNode> &nodes,
    std::vector<std::pair<TreeNode::nodeId, TreeNode::nodeId>> &deletedEdges,
    std::vector<int> &lensLayer) {
  std::vector<int> removedNodes = {};
  removedNodes.resize(nodes.size(), -1);

  int nodesInLayer = 0;
  int removedCount = 0;
  for (int i = 0; removedCount < nodes.size(); i++) {
    for (int j = 0; j < nodes.size(); j++) {
      if (nodes[j].degM == 0 && removedNodes[j] == -1) {
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
          nodes[nodes[j].succ[k]].degM--;
        }
        removedNodes[j] = 1;
      }
    }
    lensLayer.push_back(nodesInLayer);
    nodesInLayer = 0;
  }

  for (int i = 0; i < deletedEdges.size(); i++) {
    nodes[deletedEdges[i].first].succ.push_back(deletedEdges[i].second);
  }
}

void removeIncidentArcs(std::vector<TreeNode> &nodes, TreeNode::nodeId id) {
  for (TreeNode &node : nodes) {
    node.succ.erase(std::remove(node.succ.begin(), node.succ.end(), id),
                    node.succ.end());
    node.pred.erase(std::remove(node.pred.begin(), node.pred.end(), id),
                    node.pred.end());
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
      if (nodes[nodes[i].succ[j]].layer - nodes[i].layer > 1) {
        addLineSegment(nodes, lensLayer, i, j, 1);
        return false;
      }
      if (nodes[nodes[i].succ[j]].layer - nodes[i].layer < -1) {
        addLineSegment(nodes, lensLayer, i, j, -1);
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

void printInfo(std::vector<TreeNode> &nodes) {
  degReinit(nodes);
  predReinit(nodes);
  for (int i = 0; i < nodes.size(); i++) {
    if (nodes[i].isDummy == true) {
      printf("№%i (dummy)| number %i, layer %i, dp %i, dm %i", nodes[i].id,
             nodes[i].number, nodes[i].layer, nodes[i].degP, nodes[i].degM);
    } else {
      printf("№%i| number %i, layer %i, dp %i, dm %i", nodes[i].id,
             nodes[i].number, nodes[i].layer, nodes[i].degP, nodes[i].degM);
    }
    printf("\nsucc: ");
    for (int j = 0; j < nodes[i].succ.size(); j++) {
      printf("%i ", nodes[i].succ[j]);
    }
    printf("\npred: ");
    for (int j = 0; j < nodes[i].pred.size(); j++) {
      printf("%i ", nodes[i].pred[j]);
    }
    printf("\n");
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
    degReinit(nodes);
    for (int i = 0; i < nodes.size(); i++) {
      if (nodes[i].degM == 0) {
        sources.push_back(nodes[i].id);
      }
    }
  }
  return sources;
}

const std::vector<TreeNode::nodeId> &Net::getSinks() {
  if (!sinksCalculated) {
    degReinit(nodes);
    for (int i = 0; i < nodes.size(); i++) {
      if (nodes[i].degP == 0) {
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

  predReinit(nodes);
  degReinit(nodes);
}

void Net::netTreeNodesToNormalizedElements(
    std::vector<NormalizedElement> &normalizedElements) {
  float maxNumber = -1, maxLayer = -1;
  for (int i = 0; i < nodes.size(); i++) {
    if (nodes[i].layer > maxLayer) {
      maxLayer = nodes[i].layer;
    }
    if (nodes[i].number > maxNumber) {
      maxNumber = nodes[i].number;
    }
  }
  float nCellSize = -1;

  if (maxLayer > maxNumber) {
    nCellSize = 1.0 / (maxLayer + 1);
  } else {
    nCellSize = 1.0 / (maxNumber + 1);
  }

  for (int i = 0; i < nodes.size(); i++) {
    NormalizedElement nElement = {};
    nElement.id = nodes[i].id;

    if (nodes[i].isDummy) {
      NormalizedPoint nPoint = {};
      nPoint.nX = nCellSize * nodes[i].number + (nCellSize / 2) / 2;
      nPoint.nY = nCellSize * nodes[i].layer + (nCellSize / 4) / 2;

      nElement.nPoint = nPoint;
      nElement.nH = 0;
      nElement.nW = 0;
    } else {
      NormalizedPoint nPoint = {};
      nPoint.nX = nCellSize * nodes[i].number;
      nPoint.nY = nCellSize * nodes[i].layer;

      nElement.nPoint = nPoint;
      nElement.nH = nCellSize / 4;
      nElement.nW = nCellSize / 2;
    }

    normalizedElements.push_back(nElement);
  }

  int countConnections = 0;
  for (int i = 0; i < nodes.size(); i++) {
    for (int j = 0; j < nodes[i].succ.size(); j++) {
      NormalizedConnection connection = {};

      connection.id = countConnections;
      connection.startElementId = nodes[i].id;
      connection.endElementId = nodes[i].succ[j];

      NormalizedPoint nPointStart = {};
      nPointStart.nX =
          normalizedElements[i].nPoint.nX + normalizedElements[i].nW / 2;
      nPointStart.nY =
          normalizedElements[i].nPoint.nY + normalizedElements[i].nH;

      NormalizedPoint nPointEnd = {};
      nPointEnd.nX = normalizedElements[nodes[i].succ[j]].nPoint.nX +
                     normalizedElements[nodes[i].succ[j]].nW / 2;
      nPointEnd.nY = normalizedElements[nodes[i].succ[j]].nPoint.nY;

      connection.nVertices.push_back(nPointStart);
      connection.nVertices.push_back(nPointEnd);

      countConnections++;

      normalizedElements[i].connections.push_back(connection);
    }
  }
}
