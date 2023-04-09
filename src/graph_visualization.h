#ifndef GRAPH_VISUALIZATION_H_
#define GRAPH_VISUALIZATION_H_

#include <iostream>
#include <vector>

struct TreeNode {
  using nodeId = int;

  std::vector<nodeId> succ = {};
  std::vector<nodeId> pred = {};
  nodeId id = 0;
  int degP = 0;
  int degM = 0;
  int layer = 0;
  int number = 0;
  bool isDummy = false;
  int x = 0;
  int y = 0;
};

void inputGraph(std::vector<std::vector<int>> &input);

void transformationGraph(std::vector<std::vector<int>> &input, std::vector<TreeNode> &output);

void printInfo(std::vector<TreeNode> &nodes);

struct Net {
  using nodeId = TreeNode::nodeId;

private:
  std::vector<TreeNode> nodes;
  std::vector<TreeNode::nodeId> sources = {};
  std::vector<TreeNode::nodeId> sinks = {};

public:
  const std::vector<nodeId> &getSources();
  const std::vector<nodeId> &getSinks();

  const std::vector<nodeId> &getSuccessors(nodeId id);
  const std::vector<nodeId> &getPredecessors(nodeId id);

  const TreeNode &getNode(nodeId id);
};

#endif // GRAPH_VISUALIZATION_H_

