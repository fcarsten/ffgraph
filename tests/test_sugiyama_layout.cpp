#include "sugiyama_layout_snapshot.h"
#include <ffgraph/FFSugi.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>

namespace {
struct HeadlessSugi : FFSugi {
  void init(const FFGraph& g) override { FFLayoutLabel::init(g); }
  FFGraphLabel* proto() const override { return new HeadlessSugi; }
};
struct Fixture { const char* name; int nodes; bool directed; std::vector<std::pair<int,int>> edges; };
const std::vector<Fixture> fixtures = {
  {"long-edges", 8, true, {{1,2},{2,3},{3,4},{4,5},{5,6},{6,7},{7,8},{1,8},{2,7}}},
  {"bipartite", 6, true, {{1,4},{1,5},{1,6},{2,4},{2,5},{2,6},{3,4},{3,5},{3,6}}},
  {"cycle-parallel-isolated", 7, false, {{1,2},{2,3},{3,4},{4,1},{1,2},{3,3},{4,5},{5,6}}},
  {"star", 17, false, {{1,2},{1,3},{1,4},{1,5},{1,6},{1,7},{1,8},{1,9},{1,10},
                      {1,11},{1,12},{1,13},{1,14},{1,15},{1,16},{1,17}}}
};
ffgraph_test::LayoutSnapshot exercise(const Fixture& fixture, int dimension) {
  FFGraph graph(YES);
  graph->setName(fixture.name);
  graph->_setDirected(fixture.directed);
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  std::vector<FFNode> nodes;
  std::map<FFEdge, std::pair<FFNode,FFNode>> edges;
  for (int i = 0; i < fixture.nodes; ++i) {
    auto* node = graph->addNode(FFPoint(i * 17, i % 3 * 23, i % 5 * 11));
    node->setValue(i + 1); node->setName(String(("node" + std::to_string(i + 1)).c_str()));
    nodes.push_back(node);
  }
  int id = 0;
  for (const auto& [source, target] : fixture.edges) {
    auto* edge = graph->addEdge(nodes[source - 1], nodes[target - 1]);
    edge->setValue(++id); edge->setName(String(("edge" + std::to_string(id)).c_str()));
    edge->setWeight(1 + (id % 3) * 0.5f);
    edges.emplace(edge, std::make_pair(edge->source(), edge->target()));
  }
  const auto ids = ffgraph_test::layoutIds(*graph.operator->());
  auto* sugi = new HeadlessSugi;
  sugi->setDimension(dimension);
  graph->addGraphLabel(sugi);
  sugi->layout();
  if (graph->getNumNodes() != fixture.nodes || graph->getNumEdges() != static_cast<int>(edges.size()))
    throw std::runtime_error("layout changed graph size");
  for (const auto& [edge, endpoints] : edges)
    if (!graph->allEdges().contains(edge) || edge->source() != endpoints.first || edge->target() != endpoints.second)
      throw std::runtime_error("layout changed edge incidence");
  return ffgraph_test::captureLayout(*graph.operator->(), ids);
}
}
int main(int argc, char** argv) {
  try {
    const bool record = argc == 3 && std::string(argv[1]) == "--record";
    if (!record && argc != 2) throw std::runtime_error("usage: test_sugiyama_layout [--record] ORACLE_DIR");
    const std::filesystem::path root(argv[record ? 2 : 1]);
    if (record && !std::filesystem::create_directory(root)) throw std::runtime_error("oracle directory exists");
    for (const auto& fixture : fixtures) for (int dimension : {2,3}) {
      const auto snapshot = exercise(fixture, dimension);
      const auto path = root / (std::string(fixture.name) + "-" + std::to_string(dimension) + ".layout");
      if (record) {
        std::ofstream out(path);
        ffgraph_test::writeLayout(out, snapshot);
        if (!out) throw std::runtime_error("cannot write oracle");
      } else {
        std::ifstream in(path);
        if (!in || !ffgraph_test::sameLayout(ffgraph_test::readLayout(in), snapshot))
          throw std::runtime_error("layout differs from pre-optimization oracle: " + path.string());
      }
    }
    std::cout << "verified 8 complete Sugiyama layouts (2D/3D)\n";
  } catch (const std::exception& error) {
    std::cerr << "Sugiyama layout: " << error.what() << '\n';
    return 1;
  }
}
