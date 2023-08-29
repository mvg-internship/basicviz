//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "netfmt_bench.h"

#include "layout.h"
#include <lorina/bench.hpp>

#include <istream>
#include <string>
#include <vector>

#define UNUSED(x) do { (void) (x); } while(0)

namespace {

class BenchNetReader: public lorina::bench_reader {
  mutable std::unordered_map<std::string, Net::Id> nodeMap;
  Net &net;

public:
  explicit BenchNetReader(Net &net)
      : net(net) {
    return;
  }

  virtual void on_input(const std::string &name) const override {
    UNUSED(getNode(name, INPUT));
  }

  virtual void on_output(const std::string &name) const override {
    UNUSED(getNode(name, OUTPUT));
  }

  virtual void on_dff_input(const std::string &input) const override {
    UNUSED(getNode(input, DFF));
  }

  virtual void on_dff(
      const std::string &input, const std::string &output) const override {
    linkNodes(getNode(input), getNode(output));
  }

  virtual void on_gate(
      const std::vector<std::string> &inputs,
      const std::string &output,
      const std::string &type) const override {
    Type numType = NONE;
    if (type == "NOT") {
      numType = NOT;
    } else if (type == "AND") {
      numType = AND;
    } else if (type == "OR") {
      numType = OR;
    } else if (type == "NAND") {
      numType = NAND;
    } else if (type == "NOR") {
      numType = NOR;
    }
    TreeNode *dst = getNode(output, numType);
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

  TreeNode *getNode(const std::string &name, Type type = NONE) const {
    Net::Id id;

    auto it = nodeMap.find(name);
    if (it != nodeMap.end()) {
      id = it->second;
    } else {
      id = net.addNode(type);
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
