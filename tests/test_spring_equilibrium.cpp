// Exercise the real continuous layout and its public displacement kernel.
#include <ffgraph/FFSpring.h>
#include <ffgraph/FFInspector.h>
#include <ffgraph/FFGraphicEdge.h>
#include <algorithm>
#include <cfenv>
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
void samePosition(const FFPoint& actual, const FFPoint& expected) {
  for (int i = 0; i < 4; ++i)
    require(std::isfinite(actual.v[i]) && std::abs(actual.v[i] - expected.v[i]) < 1e-5f,
            "unexpected or nonfinite equilibrium position");
}
struct HeadlessSpring : FFSpring {
  void init(const FFGraph& graph) override { FFLayoutLabel::init(graph); }
  void setTestInspector(FFInspector* value) { inspector = value; }
};
struct EquilibriumInspector : FFInspector {
  HeadlessSpring& spring;
  FFNode first, second;
  FFPoint originalFirst, originalSecond;
  FFNode fixed;
  int dimension, calls = 0;
  EquilibriumInspector(HeadlessSpring& layout, FFNode a, FFNode b, FFNode pinned, int dim)
      : FFInspector(&layout), spring(layout), first(a), second(b),
        originalFirst(a->getGraphicNode()->getCoords()),
        originalSecond(b->getGraphicNode()->getCoords()), fixed(pinned), dimension(dim) {}
  void inspect() override {}
  void update() override {
    // The workspace remains live here, without a Tk window or private access.
    spring.write_nodePosition(2);
    samePosition(first->getGraphicNode()->getCoords(), originalFirst);
    samePosition(second->getGraphicNode()->getCoords(), originalSecond);
    if (++calls != 101) return;

    FFPoint forces[] = {FFPoint(), FFPoint(), FFPoint()};
    FFCoord lengths[3] = {};
    // Zero force is stationary at zero and positive temperature, including -0.
    for (FFCoord temperature : {0.0f, 1.0f}) {
      forces[1] = FFPoint(-0.0f, 0, 0); forces[2] = FFPoint();
      spring.compute_maxForce(forces, 2, lengths);
      spring.compute_nodePosition(forces, 2, temperature, lengths);
      spring.write_nodePosition(2);
      samePosition(first->getGraphicNode()->getCoords(), originalFirst);
      samePosition(second->getGraphicNode()->getCoords(), originalSecond);
    }
    // Nonzero forces with zero temperature must also leave both vertices put.
    forces[1] = forces[2] = FFPoint(3, 4, dimension == 3 ? 12 : 0);
    spring.compute_maxForce(forces, 2, lengths);
    spring.compute_nodePosition(forces, 2, 0, lengths);
    spring.write_nodePosition(2);
    samePosition(first->getGraphicNode()->getCoords(), originalFirst);
    samePosition(second->getGraphicNode()->getCoords(), originalSecond);

    // Keep the nonzero displacement/clipping path and fixed-node rule covered.
    for (FFCoord temperature : {1.0f, 20.0f}) {
      spring.compute_nodePosition(forces, 2, temperature, lengths);
      const FFCoord factor = std::min(temperature / lengths[1], 1.0f);
      if (first != fixed)
        for (int i = 0; i < 3; ++i) originalFirst.v[i] += forces[1].v[i] * factor;
      if (second != fixed)
        for (int i = 0; i < 3; ++i) originalSecond.v[i] += forces[2].v[i] * factor;
      spring.write_nodePosition(2);
      samePosition(first->getGraphicNode()->getCoords(), originalFirst);
      samePosition(second->getGraphicNode()->getCoords(), originalSecond);
    }
    spring.stop();
  }
};
void exercise(int dimension, bool weighted) {
  FFGraph graph(YES);
  graph->_setDirected(YES);
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  auto first = graph->addNode(FFPoint(0, 0, 0)); first->setValue(1);
  auto second = graph->addNode(FFPoint(100, 0, 0)); second->setValue(2);
  auto edge = graph->addEdge(first, second);
  auto* spring = new HeadlessSpring;
  graph->addGraphLabel(spring);
  spring->setDimension(dimension);
  spring->setUseEdgeWeight(weighted ? YES : NO);
  require(spring->setQuality(0), "continuous mode rejected");

  // The initializer perturbs the last visited movable vertex; pin it so this
  // exact ideal-distance pair starts at zero force in either container order.
  FFNode last = nullptr;
  const auto& all = graph->allNodes();
  for (Pix p = all.first(); p; all.next(p)) last = all(p);
  FFSet<FFNode> fixed; fixed.add(last);
  auto* inspector = new EquilibriumInspector(*spring, first, second, last, dimension);
  spring->setTestInspector(inspector); // The graph label owns this headless inspector.
  std::feclearexcept(FE_INVALID | FE_DIVBYZERO);
  spring->layout(fixed);
  require(std::fetestexcept(FE_INVALID | FE_DIVBYZERO) == 0, "invalid force normalization");
  require(inspector->calls == 101, "continuous mode stopped or skipped updates at equilibrium");
  samePosition(first->getGraphicNode()->getCoords(), inspector->originalFirst);
  samePosition(second->getGraphicNode()->getCoords(), inspector->originalSecond);
  require(graph->getNumNodes() == 2 && graph->getNumEdges() == 1 && graph->getDirected(),
          "layout changed graph size/direction");
  require(graph->allNodes().contains(first) && graph->allNodes().contains(second) &&
          graph->allEdges().contains(edge) && edge->source() == first && edge->target() == second,
          "layout changed graph objects/incidence");
  for (int i = 0; i < edge->getGraphicEdge()->anzPoints(); ++i)
    for (auto value : edge->getGraphicEdge()->getPointAt(i).v)
      require(std::isfinite(value), "nonfinite edge geometry");
}
}
int main() {
  try {
    for (int dimension : {2, 3})
      for (bool weighted : {false, true}) exercise(dimension, weighted);
    std::cout << "verified continuous equilibrium, stop updates and displacement in 2D/3D\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n'; return 1;
  }
}
