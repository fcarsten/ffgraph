#ifndef FFGRAPH_TEST_GRAPH_SNAPSHOT_H
#define FFGRAPH_TEST_GRAPH_SNAPSHOT_H

// Reusable format-independent graph oracle: preserve directed endpoint order,
// IDs, labels, coordinates and parallel-edge multiplicity.  Not a layout oracle.
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFGraphicNode.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace ffgraph_test {

using Node = std::tuple<int, std::string, double, double, double>;
using Edge = std::tuple<int, int, int, std::string>; // source ID, target ID, edge ID, label

struct GraphSnapshot {
  std::string name;
  std::vector<Node> nodes;
  std::vector<Edge> edges;
};

inline GraphSnapshot capture(FFGraphClass& graph) {
  GraphSnapshot result;
  result.name = graph.getName().c_str();
  std::map<const FFNodeClass*, int> member_nodes;
  for (Pix p = graph.allNodes().first(); p; graph.allNodes().next(p)) {
    FFNodeClass* node = graph.allNodes()(p);
    if (!node || !node->getGraphicNode())
      throw std::runtime_error("missing graph node/graphic label");
    const auto* graphic = node->getGraphicNode();
    const double x = graphic->getx(), y = graphic->gety(), z = graphic->getz();
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
      throw std::runtime_error("non-finite node coordinate");
    result.nodes.emplace_back(node->getValue(), node->getName().c_str(), x, y, z);
    member_nodes.emplace(node, node->getValue());
  }
  for (Pix p = graph.allEdges().first(); p; graph.allEdges().next(p)) {
    FFEdgeClass* edge = graph.allEdges()(p);
    if (!edge || !member_nodes.count(edge->source()) || !member_nodes.count(edge->target()))
      throw std::runtime_error("edge has foreign/null endpoint");
    result.edges.emplace_back(member_nodes.at(edge->source()),
                              member_nodes.at(edge->target()), edge->getValue(),
                              edge->getName().c_str());
  }
  std::sort(result.nodes.begin(), result.nodes.end());
  std::sort(result.edges.begin(), result.edges.end());
  return result;
}

inline std::string difference(const GraphSnapshot& expected, const GraphSnapshot& actual) {
  if (expected.name != actual.name)
    return "graph name differs: expected='" + expected.name + "' actual='" + actual.name + "'";
  if (expected.nodes.size() != actual.nodes.size())
    return "node count differs: expected=" + std::to_string(expected.nodes.size()) +
           " actual=" + std::to_string(actual.nodes.size());
  if (expected.edges.size() != actual.edges.size())
    return "edge count differs: expected=" + std::to_string(expected.edges.size()) +
           " actual=" + std::to_string(actual.edges.size());
  for (size_t i = 0; i < expected.nodes.size(); ++i) {
    if (expected.nodes[i] != actual.nodes[i]) {
      std::ostringstream out;
      out << std::setprecision(std::numeric_limits<double>::max_digits10);
      out << "node[" << i << "] differs: expected=(";
      std::apply([&](const auto&... fields) { ((out << fields << ','), ...); }, expected.nodes[i]);
      out << ") actual=(";
      std::apply([&](const auto&... fields) { ((out << fields << ','), ...); }, actual.nodes[i]);
      return out.str() + ')';
    }
  }
  for (size_t i = 0; i < expected.edges.size(); ++i) {
    if (expected.edges[i] != actual.edges[i]) {
      std::ostringstream out;
      out << "edge[" << i << "] differs: expected=(";
      std::apply([&](const auto&... fields) { ((out << fields << ','), ...); }, expected.edges[i]);
      out << ") actual=(";
      std::apply([&](const auto&... fields) { ((out << fields << ','), ...); }, actual.edges[i]);
      return out.str() + ')';
    }
  }
  return {};
}

// Independent oracle from corpus/generate.py; unlike importer comparison,
// this catches matching bugs in *both* implementations.
inline GraphSnapshot readOracle(std::istream& input) {
  GraphSnapshot result;
  std::string line;
  bool found_name = false;
  while (std::getline(input, line)) {
    if (line.empty() || line[0] == '#') continue;
    std::istringstream row(line);
    std::string tag;
    std::getline(row, tag, '\t');
    if (tag == "graph") {
      if (found_name || !std::getline(row, result.name) || result.name.empty())
        throw std::runtime_error("invalid oracle graph header");
      found_name = true;
    } else if (tag == "node") {
      std::string id, label, x, y;
      if (!std::getline(row, id, '\t') || !std::getline(row, label, '\t') ||
          !std::getline(row, x, '\t') || !std::getline(row, y) ||
          id.empty() || label.empty() || x.empty() || y.empty())
        throw std::runtime_error("invalid oracle node");
      result.nodes.emplace_back(std::stoi(id), label,
                                static_cast<FFCoord>(std::stod(x)),
                                static_cast<FFCoord>(std::stod(y)), 0.0);
    } else if (tag == "edge") {
      std::string id, from, to, label;
      if (!std::getline(row, id, '\t') || !std::getline(row, from, '\t') ||
          !std::getline(row, to, '\t') || !std::getline(row, label))
        throw std::runtime_error("invalid oracle edge");
      result.edges.emplace_back(std::stoi(from), std::stoi(to), std::stoi(id), label);
    } else {
      throw std::runtime_error("unknown oracle row kind: " + tag);
    }
  }
  if (!found_name) throw std::runtime_error("oracle lacks graph header");
  std::sort(result.nodes.begin(), result.nodes.end());
  std::sort(result.edges.begin(), result.edges.end());
  return result;
}

} // namespace ffgraph_test
#endif
