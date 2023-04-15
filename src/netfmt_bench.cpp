#include "netfmt_bench.h"

#include "layout.h"
#include <lorina/bench.hpp>

#include <istream>
#include <string>
#include <vector>

#define UNUSED(x) do { (void) (x); } while(0)

namespace {

class BenchNetReader: public lorina::bench_reader {
  mutable std::unordered_map<std::string, Net::nodeId> nodeMap;
  Net &net;

public:
  explicit BenchNetReader(Net &net)
      : net(net) {
    return;
  }

  virtual void on_input(const std::string &name) const override {
    UNUSED(getNode(name));
  }

  virtual void on_output(const std::string &name) const override {
    UNUSED(getNode(name));
  }

  virtual void on_dff_input(const std::string &input) const override {
    UNUSED(getNode(input));
  }

  virtual void on_dff(
      const std::string &input, const std::string &output) const override {
    linkNodes(getNode(input), getNode(output));
  }

  virtual void on_gate(
      const std::vector<std::string> &inputs,
      const std::string &output,
      const std::string &type) const override {
    UNUSED(type);

    TreeNode *dst = getNode(output);
    for (const std::string &input: inputs) {
      linkNodes(getNode(input), dst);
    }
  }

  virtual void on_assign(
      const std::string &input, const std::string &output) const override {
    linkNodes(getNode(input), getNode(output));
  }

private:
  void linkNodes(TreeNode *src, TreeNode *dst) const {
    src->succ.push_back(dst->id);
    dst->pred.push_back(src->id);
  }

  TreeNode *getNode(const std::string &name) const {
    Net::nodeId id;

    auto it = nodeMap.find(name);
    if (it != nodeMap.end()) {
      id = it->second;
    } else {
      id = net.addNode();
      nodeMap.emplace(name, id);
    }
    return net.getNode(id);
  }
};

} // end namespace

bool
readNetFromBench(std::istream &is, Net &net) {
  BenchNetReader reader(net);
  auto result = lorina::read_bench(is, reader);
  return result == lorina::return_code::success;
}
