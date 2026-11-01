// Opt-in graph-corpus invariant/oracle test and bounded spring-layout smoke.
// Oracles are stable TSV; layouts are checked for invariants, NOT pixel equality.
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFSpring.h>
#include <ffgraph/FFGraphLabel.h>

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
#include <unordered_set>
#include <vector>

namespace {
struct HeadlessSpring : FFSpring {
  void init(const FFGraph& g) override { FFLayoutLabel::init(g); }
  FFGraphLabel* proto() const override { return new HeadlessSpring; }
};
void require(bool okay, const std::string& reason) {
  if (!okay) { std::cerr << "graph corpus layout: " << reason << '\n'; std::abort(); }
}
std::vector<std::string> columns(const std::string& line) {
  std::vector<std::string> result;
  std::istringstream stream(line);
  std::string part;
  while (std::getline(stream, part, '\t')) result.push_back(part);
  return result;
}
using Node = std::tuple<std::string, double, double>;
using Edge = std::tuple<int, int, int, std::string>; // edge ID, source ID, target ID, label
struct Oracle {
  std::string name;
  std::map<int, Node> nodes;
  std::multiset<Edge> edges;
};
Oracle readOracle(const std::filesystem::path& file) {
  std::ifstream input(file);
  require(static_cast<bool>(input), "cannot open oracle " + file.string());
  Oracle expected;
  std::string line;
  unsigned graphLines = 0;
  while (std::getline(input, line)) {
    if (line.empty() || line.front() == '#') continue;
    const auto parts = columns(line);
    if (parts[0] == "graph") {
      require(parts.size() == 2 && graphLines++ == 0, "invalid oracle graph line");
      expected.name = parts[1];
    } else if (parts[0] == "node") {
      require(parts.size() == 5, "invalid oracle node line");
      require(expected.nodes.emplace(std::stoi(parts[1]), Node(parts[2],
              std::stod(parts[3]), std::stod(parts[4]))).second, "duplicate oracle node");
    } else if (parts[0] == "edge") {
      require(parts.size() == 5, "invalid oracle edge line");
      expected.edges.emplace(std::stoi(parts[1]), std::stoi(parts[2]),
                             std::stoi(parts[3]), parts[4]);
    } else {
      require(false, "unknown oracle record");
    }
  }
  require(graphLines == 1, "missing oracle graph record");
  return expected;
}
void exercise(const std::filesystem::path& file, const std::filesystem::path& oracleFile,
              size_t expected_nodes, size_t expected_edges, const std::string& case_name,
              unsigned max_iterations) {
  const Oracle oracle = readOracle(oracleFile);
  require(oracle.nodes.size() == expected_nodes && oracle.edges.size() == expected_edges,
          case_name + ": oracle counts agree with cases.tsv");
  FFGraph graph(YES);
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  graph->load(String(file.string()), YES);
  require(static_cast<size_t>(graph->allNodes().length()) == expected_nodes, case_name + ": imported nodes");
  require(static_cast<size_t>(graph->allEdges().length()) == expected_edges, case_name + ": imported edges");
  require(std::string(graph->getName().c_str()) == oracle.name, case_name + ": graph name");
  std::unordered_set<FFNodeClass*> nodes;
  std::map<int, Node> importedNodes;
  std::multiset<Edge> importedEdges;
  for (Pix p = graph->allNodes().first(); p; graph->allNodes().next(p)) {
    auto* n = graph->allNodes()(p);
    nodes.insert(n);
    const auto* graphic = n->getGraphicNode();
    require(graphic != nullptr, case_name + ": missing graphic node");
    require(importedNodes.emplace(n->getValue(), Node(std::string(n->getName().c_str()),
            graphic->getx(), graphic->gety())).second, case_name + ": duplicate imported node ID");
  }
  for (const auto& [id, expected] : oracle.nodes) {
    const auto it = importedNodes.find(id);
    require(it != importedNodes.end(), case_name + ": missing imported node ID");
    require(std::get<0>(it->second) == std::get<0>(expected) &&
            std::abs(std::get<1>(it->second) - std::get<1>(expected)) < 1e-4 &&
            std::abs(std::get<2>(it->second) - std::get<2>(expected)) < 1e-4,
            case_name + ": imported label/coordinates mismatch for node " + std::to_string(id));
  }
  for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
    FFEdgeClass* edge = graph->allEdges()(p);
    require(nodes.count(edge->source()) && nodes.count(edge->target()),
            case_name + ": edge endpoints owned by graph");
    importedEdges.emplace(edge->getValue(), edge->source()->getValue(),
                          edge->target()->getValue(), std::string(edge->getName().c_str()));
  }
  require(importedEdges == oracle.edges, case_name + ": imported edges/topology mismatch");
  auto* spring = new HeadlessSpring;
  graph->addGraphLabel(spring);
  require(spring->setMaxIteration(max_iterations), case_name + ": bounded iterations");
  spring->layout();
  for (Pix p = graph->allNodes().first(); p; graph->allNodes().next(p)) {
    const auto* node = graph->allNodes()(p)->getGraphicNode();
    require(node != nullptr, case_name + ": graphic node after layout");
    for (double value : node->getCoords().v)
      require(std::isfinite(value), case_name + ": nonfinite position after spring");
  }
  for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
    auto* graphic = graph->allEdges()(p)->getGraphicEdge();
    require(graphic && graphic->anzPoints() >= 2, case_name + ": edge route");
  }
  graph->clear();
}
}
int main(int argc, char** argv) {
  require(argc == 2 || argc == 5,
          "usage: test_graph_corpus_layout_pass11 CORPUS_DIR [MAX_NODES MAX_EDGES ITERATIONS]");
  const size_t maxNodes = argc == 5 ? std::stoul(argv[2]) : 64;
  const size_t maxEdges = argc == 5 ? std::stoul(argv[3]) : 256;
  const unsigned iterations = argc == 5 ? static_cast<unsigned>(std::stoul(argv[4])) : 2;
  require(maxNodes > 0 && maxEdges > 0 && iterations > 0 && iterations <= 20,
          "invalid graph limits or iteration count");
  const std::filesystem::path root(argv[1]);
  std::ifstream listing(root / "cases.tsv");
  require(static_cast<bool>(listing), "cannot open cases.tsv");
  std::string line;
  unsigned checked = 0;
  while (std::getline(listing, line)) {
    if (line.empty() || line.front() == '#') continue;
    const auto fields = columns(line);
    require(fields.size() == 5, "invalid TSV row");
    if (fields[0] != "graph") continue;
    const auto nodes = std::stoul(fields[3]);
    const auto edges = std::stoul(fields[4]);
    // Keep default CTest bounded. The full standard/stress corpus is for
    // separate integration checks and layout/performance benchmarking.
    if (nodes > maxNodes || edges > maxEdges) continue;
    exercise(root / fields[2], root / "oracle" / (fields[1] + ".tsv"),
             nodes, edges, fields[1], iterations);
    ++checked;
  }
  require(checked >= 15, "not enough graph cases exercised");
  std::cout << "verified import oracles and bounded spring layout for "
            << checked << " generated graphs\n";
}
