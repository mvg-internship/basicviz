#ifndef LAYOUT_H_
#define LAYOUT_H_

#include <vector>

#include "main.h"

struct TreeNode {
  using nodeId = int;

  std::vector<nodeId> succ = {};
  std::vector<nodeId> pred = {};
  nodeId id = 0;
  int layer = 0;
  int number = 0;
  bool isDummy = false;
  float barycentricValue = 0;
};

struct Net {
  using nodeId = TreeNode::nodeId;

private:
  std::vector<TreeNode> nodes;
  std::vector<TreeNode::nodeId> sources = {};
  std::vector<TreeNode::nodeId> sinks = {};
  bool sourcesCalculated = false;
  bool sinksCalculated = false;

public:
  const std::vector<nodeId> &getSources();
  const std::vector<nodeId> &getSinks();

  const std::vector<nodeId> &getSuccessors(nodeId id) const;
  const std::vector<nodeId> &getPredecessors(nodeId id) const;

  nodeId addNode();

  TreeNode *getNode(nodeId id) {
    const Net &net = *this;
    return const_cast<TreeNode *>(net.getNode(id));
  }

  const TreeNode *getNode(nodeId id) const;

  void transformationGraph();
  void netTreeNodesToNormalizedElements(
      std::vector<NormalizedElement> &normalizedElements);

  std::vector<std::vector<TreeNode::nodeId>> &getNodesByLayer();
};

#endif // LAYOUT_H_
