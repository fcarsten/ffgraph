// Metamorphic graph import/layout checks: topology must be invariant under
// node ID relabelling, declaration reordering, undirected edge reversal and
// changes to initial XY. Exact post-layout coordinates are NOT a contract.
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFSpring.h>
#include <ffgraph/FFGraphLabel.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace {
constexpr double translateX = 64.125;
constexpr double translateY = -32.750;
constexpr double scale = 2.0;

struct HeadlessSpring : FFSpring {
  void init(const FFGraph& g) override { FFLayoutLabel::init(g); }
  FFGraphLabel* proto() const override { return new HeadlessSpring; }
};

void require(bool okay, const std::string& context) {
  if (!okay) {
    std::cerr << "metamorphic graph regression: " << context << '\n';
    std::abort();
  }
}

std::vector<std::string> columns(const std::string& line) {
  std::vector<std::string> result;
  std::istringstream stream(line);
  std::string part;
  while (std::getline(stream, part, '\t')) result.push_back(part);
  return result;
}

struct NodeInfo { int id; double x; double y; };
using EdgeInfo = std::tuple<int, std::string, std::string, std::string>;
struct Snapshot {
  std::string graphName;
  std::map<std::string, NodeInfo> nodes; // key: stable label, not numeric node ID
  std::multiset<EdgeInfo> edges; // sorted endpoints, preserves loops and multiplicity
};

Snapshot snapshot(const FFGraph& graph, const std::string& context, bool requireRoutes) {
  Snapshot result;
  result.graphName = std::string(graph->getName().c_str());
  std::map<const FFNodeClass*, std::string> nodeNames;
  std::set<int> ids;
  for (Pix p = graph->allNodes().first(); p; graph->allNodes().next(p)) {
    auto* node = graph->allNodes()(p);
    auto* graphic = node->getGraphicNode();
    require(graphic != nullptr, context + ": graphic node");
    const std::string name(node->getName().c_str());
    require(ids.insert(node->getValue()).second, context + ": unique node ID");
    const NodeInfo info{node->getValue(), graphic->getx(), graphic->gety()};
    require(std::isfinite(info.x) && std::isfinite(info.y), context + ": finite XY");
    require(result.nodes.emplace(name, info).second, context + ": unique node label");
    nodeNames.emplace(node, name);
  }
  for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
    auto* edge = graph->allEdges()(p);
    const auto from = nodeNames.find(edge->source());
    const auto to = nodeNames.find(edge->target());
    require(from != nodeNames.end() && to != nodeNames.end(), context + ": owned endpoints");
    const std::string a = std::min(from->second, to->second);
    const std::string b = std::max(from->second, to->second);
    result.edges.emplace(edge->getValue(), std::string(edge->getName().c_str()), a, b);
    if (requireRoutes) {
      auto* route = edge->getGraphicEdge();
      require(route && route->anzPoints() >= 2, context + ": edge route");
    }
  }
  return result;
}

void checkEquivalent(const Snapshot& original, const Snapshot& changed,
                     const std::string& kind, const std::string& context) {
  require(original.graphName == changed.graphName, context + ": graph name");
  require(original.nodes.size() == changed.nodes.size(), context + ": node count");
  require(original.edges == changed.edges, context + ": labelled undirected edge multiset");
  size_t remapped = 0;
  for (const auto& [label, before] : original.nodes) {
    const auto it = changed.nodes.find(label);
    require(it != changed.nodes.end(), context + ": node label " + label);
    const auto& after = it->second;
    const double x = kind == "translate_xy" ? before.x + translateX :
                     kind == "scale_xy" ? before.x * scale : before.x;
    const double y = kind == "translate_xy" ? before.y + translateY :
                     kind == "scale_xy" ? before.y * scale : before.y;
    require(std::abs(after.x - x) < 0.001 && std::abs(after.y - y) < 0.001,
            context + ": expected input coordinate transform for " + label);
    if (kind == "relabel_ids") {
      if (before.id != after.id) ++remapped;
    } else {
      require(before.id == after.id, context + ": node ID must be unchanged");
    }
  }
  if (kind == "relabel_ids" && !original.nodes.empty())
    require(remapped == original.nodes.size(), context + ": node ID bijection exercised");
}

void runLayout(FFGraph& graph, const Snapshot& before, const std::string& context) {
  auto* spring = new HeadlessSpring;
  graph->addGraphLabel(spring);
  require(spring->setMaxIteration(2), context + ": bounded spring iterations");
  spring->layout();
  const auto after = snapshot(graph, context + " after layout", true);
  require(before.graphName == after.graphName, context + ": layout preserves graph name");
  // The historical spring layout explicitly renumbers node/edge `value`
  // fields for its internal arrays. Do not assert numeric ID stability after
  // layout: preserve labelled incidence, edge multiplicity and valid geometry.
  auto topology = [](const Snapshot& state) {
    std::multiset<std::tuple<std::string, std::string, std::string>> result;
    for (const auto& edge : state.edges)
      result.emplace(std::get<1>(edge), std::get<2>(edge), std::get<3>(edge));
    return result;
  };
  require(before.nodes.size() == after.nodes.size() && topology(before) == topology(after),
          context + ": layout preserves labelled topology and edge multiplicity");
  for (const auto& [name, node] : before.nodes) {
    (void)node;
    require(after.nodes.find(name) != after.nodes.end(),
            context + ": layout preserves node labels");
  }
}

void pairTest(const std::filesystem::path& root, const std::vector<std::string>& row) {
  require(row.size() == 6, "invalid metamorphic cases.tsv row");
  const std::string context = row[0] + " / " + row[1];
  FFGraph original(YES), variant(YES);
  for (FFGraph* graph : {&original, &variant}) {
    (*graph)->setNodeIndex((*graph)->addNodeLabel(new FFGraphicNode));
    (*graph)->setEdgeIndex((*graph)->addEdgeLabel(new FFGraphicEdge));
  }
  original->load(String((root / row[2]).string()), YES);
  variant->load(String((root / row[3]).string()), YES);
  const Snapshot reference = snapshot(original, context + " base", true);
  const Snapshot changed = snapshot(variant, context + " variant", true);
  require(reference.nodes.size() == std::stoul(row[4]) &&
          reference.edges.size() == std::stoul(row[5]), context + ": expected sizes");
  checkEquivalent(reference, changed, row[1], context);
  runLayout(original, reference, context + " base");
  runLayout(variant, changed, context + " variant");
  original->clear();
  variant->clear();
}
}

int main(int argc, char** argv) {
  require(argc == 2, "usage: test_graph_corpus_metamorphic_pass11 CORPUS_DIR");
  const std::filesystem::path root(argv[1]);
  std::ifstream listing(root / "cases.tsv");
  require(static_cast<bool>(listing), "cannot open cases.tsv");
  std::string line;
  size_t checked = 0;
  while (std::getline(listing, line)) {
    if (line.empty() || line.front() == '#') continue;
    pairTest(root, columns(line));
    ++checked;
  }
  require(checked >= 55, "insufficient metamorphic pairs");
  std::cout << "verified " << checked << " GML metamorphic import/layout pairs\n";
}
