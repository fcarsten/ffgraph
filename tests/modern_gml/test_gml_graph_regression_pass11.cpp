// Curated GML regression cases retained after removing GTControl.
// Independent graph oracles are exercised by test_gml_corpus_oracle_pass11.
#include "gml_events.h"
#include "gml_graph_adapter.h"
#include "../graph_corpus/graph_snapshot.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <algorithm>

namespace {
void require(bool condition, const std::string& explanation) {
  if (!condition) throw std::runtime_error(explanation);
}
void initialise(FFGraph& graph) {
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
}
ffgraph_test::GraphSnapshot replay(const std::string& source) {
  FFGraph graph(YES);
  initialise(graph);
  auto parsed = ffgraph_gml::parse(source);
  require(parsed.ok, "unexpected grammar rejection: " + parsed.events.error);
  auto result = ffgraph_gml::applyToGraph(parsed, *graph.operator->());
  require(result.ok, "unexpected semantic rejection: " + result.error);
  return ffgraph_test::capture(*graph.operator->());
}
void curated(const char* label, const std::string& source,
             size_t nodes, size_t edges, const std::string& graphName) {
  const auto first = replay(source);
  if (!graphName.empty())
    require(first.name == graphName, std::string(label) + ": graph name");
  require(first.nodes.size() == nodes && first.edges.size() == edges,
          std::string(label) + ": wrong graph sizes");
  for (int i = 0; i < 5; ++i)
    require(ffgraph_test::difference(first, replay(source)).empty(),
            std::string(label) + ": repeated replay changed graph");
}
void topologyAndGeometryGolden() {
  const auto graph = replay(R"(graph [ label "fixture"
 node [ id 1 label "alpha" graphics [ x -12.5 y 42 z 3 ] ]
 node [ id 2 label "beta" graphics [ x 4 y -6.25 ] ]
 edge [ source 1 target 2 ]
 edge [ source 1 target 2 ]
 edge [ source 2 target 2 ] ])");
  require(graph.name == "fixture" && graph.nodes.size() == 2 && graph.edges.size() == 3,
          "curated golden graph shape changed");
  require(std::get<0>(graph.nodes[0]) == 1 &&
          std::get<1>(graph.nodes[0]) == "alpha" &&
          std::get<2>(graph.nodes[0]) == -12.5 &&
          std::get<3>(graph.nodes[0]) == 42.0 &&
          std::get<4>(graph.nodes[0]) == 3.0,
          "node 1 ID/label/XYZ differs from independent fixture");
  require(std::get<0>(graph.nodes[1]) == 2 &&
          std::get<1>(graph.nodes[1]) == "beta" &&
          std::get<2>(graph.nodes[1]) == 4.0 &&
          std::get<3>(graph.nodes[1]) == -6.25,
          "node 2 ID/label/XY differs from independent fixture");
  int parallels = 0, loops = 0;
  for (const auto& edge : graph.edges) {
    if (std::get<0>(edge) == 1 && std::get<1>(edge) == 2) ++parallels;
    if (std::get<0>(edge) == 2 && std::get<1>(edge) == 2) ++loops;
  }
  require(parallels == 2 && loops == 1, "parallel/loop multiplicities changed");
}
void syntaxAndSemanticFailures() {
  FFGraph graph(YES);
  initialise(graph);
  FFNode node = graph->addNode();
  node->setName("existing");
  node->setValue(42);
  const auto before = ffgraph_test::capture(*graph.operator->());
  for (const char* source : {
       "graph [ node [ id 1", "graph [ node [ id ] ]",
       "graph [ edge [ source 1 ] ]", "graph ]"}) {
    for (int i = 0; i < 8; ++i) {
      const auto parsed = ffgraph_gml::parse(source);
      const auto applied = ffgraph_gml::applyToGraph(parsed, *graph.operator->());
      require(!applied.ok, "invalid GML accepted");
      require(ffgraph_test::difference(before, ffgraph_test::capture(*graph.operator->())).empty(),
              "failed import mutated the destination");
    }
  }
}
void generated(const std::filesystem::path& corpus) {
  std::ifstream cases(corpus / "cases.tsv");
  require(bool(cases), "missing corpus cases.tsv");
  std::string line;
  unsigned count = 0;
  while (std::getline(cases, line)) {
    if (line.empty() || line.front() == '#') continue;
    std::istringstream row(line);
    std::string category, name, relative, nodeCount, edgeCount;
    require(bool(std::getline(row, category, '\t') &&
                 std::getline(row, name, '\t') &&
                 std::getline(row, relative, '\t') &&
                 std::getline(row, nodeCount, '\t') &&
                 std::getline(row, edgeCount)), "invalid cases.tsv row");
    if (category != "graph") continue;
    std::ifstream fixture(corpus / relative, std::ios::binary);
    std::ifstream reference(corpus / "oracle" / (name + ".tsv"));
    require(bool(fixture) && bool(reference), "missing graph or oracle for " + name);
    const std::string bytes((std::istreambuf_iterator<char>(fixture)),
                             std::istreambuf_iterator<char>());
    const auto expected = ffgraph_test::readOracle(reference);
    const auto actual = replay(bytes);
    require(ffgraph_test::difference(expected, actual).empty(),
            name + ": " + ffgraph_test::difference(expected, actual));
    require(actual.nodes.size() == std::stoul(nodeCount) &&
            actual.edges.size() == std::stoul(edgeCount),
            name + ": advertised node/edge counts differ");
    ++count;
  }
  require(count >= 15, "too few generated graph regressions");
  std::cout << "verified " << count << " generated graph oracles\n";
}
}
int main(int argc, char** argv) {
  try {
    curated("loop and parallel edges",
      "graph [ label \"fixture\" directed 1 creator \"ffgraph\"\n"
      " node [ id 1 label \"alpha\" graphics [ x -12.5 y 42 z 3 ] ]\n"
      " node { id 2 label \"beta\" graphics { x 4 y -6.25 } }\n"
      " edge [ id 7 source 1 target 2 label \"first\" ]\n"
      " edge [ source 1 target 2 label \"parallel\" ]\n"
      " edge [ source 2 target 2 label \"self\" ] ]\n", 2, 3, "fixture");
    curated("minimal", "graph [ node [ id 1 ] ]\n", 1, 0, "");
    curated("empty", "graph []\n", 0, 0, "");
    topologyAndGeometryGolden();
    syntaxAndSemanticFailures();
    require(argc <= 2, "usage: test_gml_graph_regression_pass11 [corpus_dir]");
    if (argc == 2) generated(argv[1]);
    std::cout << "curated GML regressions passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "GML graph regression: " << error.what() << '\n';
    return 1;
  }
}
