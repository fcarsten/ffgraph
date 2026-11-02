// Spring force norms must not overflow/underflow for representable lengths.
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFSpring.h>

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
void require(bool okay, const char* message) {
  if (!okay) throw std::runtime_error(message);
}
struct HeadlessSpring : FFSpring {
  void init(const FFGraph& g) override { FFLayoutLabel::init(g); }
  FFGraphLabel* proto() const override { return new HeadlessSpring; }
};
void forceNorms() {
  HeadlessSpring spring;
  for (const FFCoord scale : {1.0f, 1.0e19f, 1.0e-30f}) {
    // Analytic 3-4-5 and 3-4-12-13 norms, with a nonzero Z-only force.
    FFPoint forces[] = {FFPoint(), FFPoint(3 * scale, 4 * scale, 0),
                        FFPoint(0, 0, 12 * scale), FFPoint(3 * scale, 4 * scale, 12 * scale)};
    FFCoord lengths[] = {-1, 0, 0, 0};
    const FFCoord largest = spring.compute_maxForce(forces, 3, lengths);
    const FFCoord expected[] = {-1, 5 * scale, 12 * scale, 13 * scale};
    require(lengths[0] == -1, "force lengths must remain one-indexed");
    for (int i = 1; i <= 3; ++i) {
      require(std::isfinite(lengths[i]) && lengths[i] > 0, "nonfinite or lost force length");
      require(std::abs(lengths[i] / expected[i] - 1) < 2.0e-6f, "wrong analytic force length");
    }
    require(largest == lengths[3], "wrong maximum force");
  }
  FFPoint forces[] = {FFPoint(), FFPoint(0, 0, 0)};
  FFCoord lengths[] = {-1, -1};
  require(spring.compute_maxForce(forces, 1, lengths) == 0 && lengths[1] == 0,
          "zero force length changed");
}
void coincidentStar(int dimension, bool weighted) {
  std::cout << "coincident star: dimension=" << dimension << " weighted=" << weighted << '\n';
  FFGraph graph(YES);
  graph->_setDirected(NO);
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  constexpr int count = 1000;
  std::vector<FFNode> nodes;
  std::vector<FFEdge> edges;
  for (int i = 0; i < count; ++i) nodes.push_back(graph->addNode());
  for (int i = 1; i < count; ++i) edges.push_back(graph->addEdge(nodes[0], nodes[i]));
  auto* spring = new HeadlessSpring;
  graph->addGraphLabel(spring);
  spring->setDimension(dimension);
  spring->setUseEdgeWeight(weighted ? YES : NO);
  spring->setAnimation(NO);
  require(spring->setQuality(2), "quality rejected"); // Ceiling max(30,10000/1000) = 30.
  spring->layout();
  require(graph->getNumNodes() == count && graph->getNumEdges() == count - 1,
          "layout changed graph size");
  require(!graph->getDirected(), "layout changed directedness");
  for (auto* node : nodes) {
    require(graph->allNodes().contains(node), "layout replaced a node");
    require(node->getGraphicNode(), "missing node graphics");
    for (auto v : node->getGraphicNode()->getCoords().v)
      require(std::isfinite(v), "star has nonfinite node coordinates");
    const auto& position = node->getGraphicNode()->getCoords();
    // At most 30 iterations, moving no farther than the ideal length (100)
    // per iteration. Catch finite-but-runaway layouts as well as NaNs.
    require(std::hypot(position.v[0] - 100, position.v[1] - 100, position.v[2]) <= 3000.01f,
            "star displacement exceeds the iteration movement budget");
  }
  for (int i = 1; i < count; ++i) {
    auto* edge = edges[i - 1];
    require(graph->allEdges().contains(edge) && edge->source() == nodes[0] &&
            edge->target() == nodes[i], "layout changed star incidence");
    auto* graphic = edge->getGraphicEdge();
    require(graphic && graphic->anzPoints() == 2, "layout changed straight edge route");
    for (int k = 0; k < graphic->anzPoints(); ++k)
      for (auto v : graphic->getPointAt(k).v)
        require(std::isfinite(v), "star has nonfinite edge coordinates");
  }
}
}
int main() {
  try {
    forceNorms();
    for (int dimension : {2, 3})
      for (bool weighted : {true, false}) coincidentStar(dimension, weighted);
  } catch (const std::exception& error) {
    std::cerr << "Spring numerics: " << error.what() << '\n';
    return 1;
  }
}
