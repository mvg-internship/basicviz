#include "graph_visualization.h"

void initIdAndSucc(std::vector<std::vector<int>> &input, std::vector<TreeNode> &nodes);
void predReinit(std::vector<TreeNode> &nodes);
void degReinit(std::vector<TreeNode> &nodes);

void algorithmDFS(int &exit, std::vector<TreeNode> &nodes, std::vector<bool> &usedId, TreeNode::nodeId id, TreeNode::nodeId prevId);
void cycleExistsDFS(std::vector<TreeNode> &nodes);

void heapify(std::vector<TreeNode> &nodes, int n, int i);
void idHeapSort(std::vector<TreeNode> &nodes);
TreeNode getTreeNodeById(std::vector<TreeNode> &nodes, TreeNode::nodeId id);
void removeIncidentArcs(std::vector<TreeNode> &nodes, TreeNode::nodeId id);
void greedyFAS(std::vector<TreeNode> &nodes, std::vector<std::pair<TreeNode::nodeId, TreeNode::nodeId>> &deletedEdges);

void algorithmASAP(std::vector<TreeNode> &nodes);

void addDummyNode(std::vector<TreeNode> &nodes, TreeNode up, TreeNode down, int order);
void addDummyNodes(std::vector<TreeNode> &nodes, std::vector<std::pair<TreeNode::nodeId, TreeNode::nodeId>> &deletedEdges);

int main() {
  std::vector<std::vector<int>> input = {};
  inputGraph(input);

  std::vector<TreeNode> nodes = {};
  transformationGraph(input, nodes);

  printInfo(nodes);

  return 0;
}

void inputGraph(std::vector<std::vector<int>> &input) {
  for (int i = 0; 1; i++) {
    int inputNum = -3;
    std::vector<int> in = {};
    input.push_back(in);
    while (1) {
      scanf("%i", &inputNum);
      if (inputNum == -1 || inputNum == -2) {
        break;
      }
      input[i].push_back(inputNum);
    }
    if (inputNum == -2) {
      break;
    }
  }
}

void initIdAndSucc(std::vector<std::vector<int>> &input, std::vector<TreeNode> &nodes) {
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
    nodes[i].pred = {};
  }

  for (int i = 0; i < nodes.size(); i++) {
    for (int j = 0; j < nodes[i].succ.size(); j++) {
      for (int k = 0; k < nodes.size(); k++) {
        if (nodes[k].id == nodes[i].succ[j]) {
          nodes[k].pred.push_back(nodes[i].id);
        }
      }
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

      for (int k = 0; k < nodes.size(); k++) {
        if (nodes[k].id == nodes[i].succ[j]) {
          nodes[k].degM++;
        }
      }
    }
  }
}

void algorithmDFS(int &exit, std::vector<TreeNode> &nodes, std::vector<bool> &usedId, TreeNode::nodeId id, TreeNode::nodeId prevId) {
  if (exit == 0) {
    usedId[id] = true;

    int numb = -1;
    for (int i = 0; i < nodes.size(); i++) {
      if (nodes[i].id == id) {
        numb = i;
        break;
      }
    }

    for (int i = 0; i < nodes[numb].succ.size(); i++) {
      if (!usedId[nodes[numb].succ[i]]) {
        algorithmDFS(exit, nodes, usedId, nodes[numb].succ[i], id);
      } else {
        if (id != prevId) {
          printf("Graph has cycle. \n");
          exit = 1;
          break;
        }
      }
    }
  }
}

void cycleExistsDFS(std::vector<TreeNode> &nodes) {
  std::vector<bool> usedId = {};
  for (int i = 0; i < nodes.size(); i++) {
    usedId.push_back(false);
  }
  int exit = 0;
  for (int i = 0; i < nodes.size(); i++) {
    if (exit == 1) {
      break;
    }
    if (!usedId[nodes[i].id]) {
      algorithmDFS(exit, nodes, usedId, nodes[i].id, -1);
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

TreeNode getTreeNodeById(std::vector<TreeNode> &nodes, TreeNode::nodeId id) {
  TreeNode node = {};
  for (int i = 0; i < nodes.size(); i++) {
    if (nodes[i].id == id) {
      node = nodes[i];
      break;
    }
  }
  return node;
}

void removeIncidentArcs(std::vector<TreeNode> &nodes, TreeNode::nodeId id) {
  for (int i = 0; i < nodes.size(); i++) {
    for (int j = 0; j < nodes[i].succ.size(); j++) {
      if (nodes[i].succ[j] == id) {
        nodes[i].succ.erase(nodes[i].succ.begin() + j);
      }
    }
    for (int j = 0; j < nodes[i].pred.size(); j++) {
      if (nodes[i].pred[j] == id) {
        nodes[i].pred.erase(nodes[i].pred.begin() + j);
      }
    }
  }
}

void greedyFAS(std::vector<TreeNode> &nodes, std::vector<std::pair<TreeNode::nodeId, TreeNode::nodeId>> &deletedEdges) {
  degReinit(nodes);

  std::vector<TreeNode> copyNodes = nodes;
  std::vector<TreeNode> s1 = {}, s2 = {};

  while (nodes.size() != 0) {
    for (int i = 0; i < nodes.size(); i++) {
      if (nodes[i].degP == 0) {
        s2.push_back(copyNodes[nodes[i].id]);

        removeIncidentArcs(nodes, nodes[i].id);
        nodes.erase(nodes.begin() + i);
        degReinit(nodes);

        i = -1;
      }
    }

    for (int i = 0; i < nodes.size(); i++) {
      if (nodes[i].degM == 0) {
        s1.push_back(copyNodes[nodes[i].id]);

        removeIncidentArcs(nodes, nodes[i].id);
        nodes.erase(nodes.begin() + i);
        degReinit(nodes);

        i = -1;
      }
    }

    if (nodes.size() != 0) {
      int max = nodes[0].degP - nodes[0].degM;
      int posMax = 0;
      for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i].degP - nodes[i].degM > max) {
          max = nodes[i].degP - nodes[i].degM;
          posMax = i;
        }
      }
      TreeNode maxNode = getTreeNodeById(copyNodes, nodes[posMax].id);
      s1.push_back(maxNode);

      removeIncidentArcs(nodes, nodes[posMax].id);
      nodes.erase(nodes.begin() + posMax);
      degReinit(nodes);
    }
  }

  std::vector<TreeNode> s = {};
  for (int i = 0; i < s1.size(); i++) {
    s.push_back(s1[i]);
  }
  for (int i = 0; i < s2.size(); i++) {
    s.push_back(s2[s2.size() - 1 - i]);
  }

  for (int i = 0; i < s.size(); i++) {
    for (int j = 0; j < s[i].succ.size(); j++) {
      for (int k = 0; k < i; k++) {
        if (s[i].succ[j] == s[k].id) {
          auto edge = std::make_pair(s[i].id, s[i].succ[j]);
          deletedEdges.push_back(edge);

          s[i].succ.erase(s[i].succ.begin() + j);
        }
      }
    }
  }

  nodes = s;
  idHeapSort(nodes);
}

void algorithmASAP(std::vector<TreeNode> &nodes, std::vector<int> &lens_layer) {
  predReinit(nodes);
  std::vector<TreeNode> copyNodes = nodes;

  std::vector<TreeNode::nodeId> rmId = {};
  for (int i = 0; copyNodes.size() != 0; i++) {
    for (int j = 0; j < copyNodes.size(); j++) {
      if (copyNodes[j].pred.size() == 0) {
        rmId.push_back(copyNodes[j].id);
      }
    }

    int layer_len = 0;
    for (int j = 0; j < nodes.size(); j++) {
      for (int k = 0; k < rmId.size(); k++) {
        if (nodes[j].id == rmId[k]) {
          nodes[j].number = k;
          nodes[j].layer = i;
          layer_len++;
        }
      }
    }
    lens_layer.push_back(layer_len);

    for (int j = 0; j < copyNodes.size(); j++) {
      for (int k = 0; k < rmId.size(); k++) {
        if (copyNodes[j].id == rmId[k]) {
          removeIncidentArcs(copyNodes, copyNodes[j].id);
          copyNodes.erase(copyNodes.begin() + j);

          j = -1;
          break;
        }
      }
    }
    rmId.clear();
  }
}

void addDummyNode(std::vector<TreeNode> &nodes, std::vector<int> &lens_layer, TreeNode up, TreeNode down, int order) {
  TreeNode dummy = {};
  int countId = 0;
  for (int i = 0; i < nodes.size(); i++) {
    countId++;
  }
  dummy.succ.push_back(down.id);
  dummy.pred.push_back(up.id);
  int max = -1;
  for (int i = 0; i < nodes.size(); i++) {
    if (nodes[i].id > max) {
      max = nodes[i].id;
    }
  }
  dummy.id = max + 1;
  dummy.layer = up.layer + order;
  dummy.number = lens_layer[dummy.layer];
  lens_layer[dummy.layer]++;
  dummy.isDummy = true;

  for (int i = 0; i < nodes.size(); i++) {
    if (nodes[i].id == up.id) {
      nodes[i].succ.push_back(dummy.id);
      for (int j = 0; j < nodes[i].succ.size(); j++) {
        if (nodes[i].succ[j] == down.id) {
          nodes[i].succ.erase(nodes[i].succ.begin() + j);
          break;
        }
      }
    }
    if (nodes[i].id == down.id) {
      nodes[i].pred.push_back(dummy.id);
      for (int j = 0; j < nodes[i].pred.size(); j++) {
        if (nodes[i].pred[j] == up.id) {
          nodes[i].pred.erase(nodes[i].pred.begin() + j);
          break;
        }
      }
    }
  }

  nodes.push_back(dummy);
}

void addDummyNodes(std::vector<TreeNode> &nodes, std::vector<int> &lens_layer, std::vector<std::pair<TreeNode::nodeId, TreeNode::nodeId>> &deletedEdges) {
  int flag = 1;
  while (flag) {
    flag = 0;
    for (int i = 0; i < nodes.size(); i++) {
      for (int j = 0; j < nodes[i].succ.size(); j++) {
        TreeNode succNode = getTreeNodeById(nodes, nodes[i].succ[j]);
        if (nodes[i].layer + 1 < succNode.layer) {
          addDummyNode(nodes, lens_layer, nodes[i], succNode, 1);
          flag = 1;
          break;
        }
      }
      if (flag == 1) {
        break;
      }
    }
  }

  for (int i = 0; i < deletedEdges.size(); i++) {
    TreeNode firstNode = getTreeNodeById(nodes, deletedEdges[i].first);
    TreeNode secondNode = getTreeNodeById(nodes, deletedEdges[i].second);

    if (firstNode.layer == secondNode.layer + 1) {
      for (int j = 0; j < nodes.size(); j++) {
        if (nodes[j].id == firstNode.id) {
          nodes[j].succ.push_back(secondNode.id);
        }
        if (nodes[j].id == secondNode.id) {
          nodes[j].pred.push_back(firstNode.id);
        }
      }
    } else {
      int countDummy = firstNode.layer - (secondNode.layer + 1);
      addDummyNode(nodes, lens_layer, firstNode, secondNode, -1);
      for (int j = 1; j < countDummy; j++) {
        addDummyNode(nodes, lens_layer, nodes.back(), secondNode, -1);
      }
    }
  }
}

void transformationGraph(std::vector<std::vector<int>> &input, std::vector<TreeNode> &output) {
  initIdAndSucc(input, output);

  std::vector<std::pair<TreeNode::nodeId, TreeNode::nodeId>> deletedEdges = {};
  greedyFAS(output, deletedEdges);

  std::vector<int> lens_layer = {};
  algorithmASAP(output, lens_layer);

  addDummyNodes(output, lens_layer, deletedEdges);

  predReinit(output);
  degReinit(output);
}

void printInfo(std::vector<TreeNode> &nodes) {
  for (int i = 0; i < nodes.size(); i++) {
    if (nodes[i].isDummy == true) {
      printf("№%i (dummy)| number %i, layer %i, dp %i, dm %i", nodes[i].id, nodes[i].number, nodes[i].layer, nodes[i].degP, nodes[i].degM);
    } else {
      printf("№%i| number %i, layer %i, dp %i, dm %i", nodes[i].id, nodes[i].number, nodes[i].layer, nodes[i].degP, nodes[i].degM);
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

const std::vector<TreeNode::nodeId> &Net::getSources() {
  degReinit(nodes);
  for (int i = 0; i < nodes.size(); i++) {
    if (nodes[i].degM == 0) {
      sources.push_back(nodes[i].id);
    }
  }
  return sources;
}

const std::vector<TreeNode::nodeId> &Net::getSinks() {
  degReinit(nodes);
  for (int i = 0; i < nodes.size(); i++) {
    if (nodes[i].degP == 0) {
      sinks.push_back(nodes[i].id);
    }
  }
  return sinks;
}

const std::vector<TreeNode::nodeId> &Net::getSuccessors(TreeNode::nodeId id) {
    return nodes[id].succ;
}

const std::vector<TreeNode::nodeId> &Net::getPredecessors(TreeNode::nodeId id) {
    return nodes[id].pred;
}

const TreeNode &Net::getNode(TreeNode::nodeId id) {
    return nodes[id];
}
