#include "test_check.h"
#include "sugiyama_layout_snapshot.h"
#include <ffgraph/FFSugi.h>
#include <ffgraph/FFSpring.h>
#include <ffgraph/FFInspector.h>
#include <ffgraph/FFreduce_crossing.h>
#include <algorithm>
#include <cmath>
#include <vector>

namespace {
template<class Base> struct Headless : Base {
  void init(const FFGraph& g) override { FFLayoutLabel::init(g); }
  void controls(FFInspector* value) { this->inspector = value; }
};
void graphics(FFGraph& graph) {
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
}
ffgraph_test::LayoutSnapshot sugi(bool start, bool useStart, int orientation) {
  FFGraph graph(YES); graphics(graph); graph->_setDirected(YES);
  std::vector<FFNode> nodes;
  for (int i = 0; i < 3; ++i) { auto n = graph->addNode(); n->setValue(i+1); nodes.push_back(n); }
  for (int i = 0; i < 3; ++i) graph->addEdge(nodes[i],nodes[(i+1)%3])->setValue(i+1);
  if (start) graph->setStartNode(nodes[2]);
  auto* layout = new Headless<FFSugi>; graph->addGraphLabel(layout);
  layout->setUseStartNode(useStart); layout->setOrient(orientation);
  const auto ids = ffgraph_test::layoutIds(*graph.operator->());
  layout->layout();
  FF_CHECK(graph->getStartNode() == (start ? nodes[2] : nullptr));
  return ffgraph_test::captureLayout(*graph.operator->(),ids);
}
void sugiyamaControls() {
  Headless<FFSugi> layout;
  FF_CHECK(layout.getCrossingAlg() == 0 && layout.setCrossingAlg(2) && layout.getCrossingAlg() == 2);
  FF_CHECK(!layout.setCrossingAlg(-1) && !layout.setCrossingAlg(3));
  FF_CHECK(layout.getCrossingAlg() == 2 && layout.setCrossingAlg(1)); // Retained legacy bypass.
  const std::vector<std::pair<int,int>> edges = {{1,6},{2,5},{2,7},{3,7},{4,5}};
  const std::vector<int> original = {4,1,2,3,4,3,5,6,7,0};
  for (int algorithm : {1,2}) {
    std::vector<SNode> nodes(8); for (auto [a,b] : edges) nodes[a].succ.append(b);
    auto levels = original;
    FF_CHECK(reduce_crossings(nodes.data(),1,4,4,7,algorithm,levels.data(),8,3) == 0 && levels == original);
  }
  for (int orientation : {1,2}) {
    FF_CHECK(ffgraph_test::sameLayout(sugi(true,false,orientation),sugi(false,false,orientation)));
    FF_CHECK(!ffgraph_test::sameLayout(sugi(true,true,orientation),sugi(false,true,orientation)));
  }
}
struct Counter : FFInspector {
  Headless<FFSpring>& spring;
  int count = 0, stopAt;
  Counter(Headless<FFSpring>& s, int stop) : FFInspector(&s), spring(s), stopAt(stop) {}
  void inspect() override {}
  void update() override { if (++count == stopAt) spring.stop(); }
};
void springControls() {
  for (int dimension : {2,3}) {
    FFGraph graph(YES); graphics(graph);
    auto a = graph->addNode(FFPoint(0,0,17)), b = graph->addNode(FFPoint(1000,500,-13));
    graph->addEdge(a,b);
    auto* spring = new Headless<FFSpring>; graph->addGraphLabel(spring); spring->setDimension(dimension);
    auto* counter = new Counter(*spring,10); spring->controls(counter);
    FF_CHECK(spring->setQuality(2) && spring->setMaxIteration(1));
    spring->layout(); FF_CHECK(counter->count == 1);
    counter->count = 0; FF_CHECK(spring->setQuality(0));
    spring->layout(); FF_CHECK(counter->count == 10); // Unlimited stays explicit.
    FFSet<FFNode> pinned; pinned.add(a);
    a->getGraphicNode()->moveTo(FFPoint(321,456,17));
    const auto original = a->getGraphicNode()->getCoords();
    FF_CHECK(spring->setQuality(2)); spring->layout(pinned);
    FF_CHECK(a->getGraphicNode()->getCoords() == original);
    graph->removeNode(b);
    spring->layout(pinned); FF_CHECK(a->getGraphicNode()->getCoords() == original);
    spring->layout();
    const auto p = a->getGraphicNode()->getCoords();
    FF_CHECK(p.v[0] == spring->getxoff() && p.v[1] == spring->getyoff());
    FF_CHECK(p.v[2] == (dimension == 3 ? spring->getzoff() : 0));
  }
}
struct ForceOracle : FFInspector {
  Headless<FFSpring>& spring;
  FFNode a,b;
  FFCoord expected;
  ForceOracle(Headless<FFSpring>& s, FFNode first, FFNode second, FFCoord value)
      : FFInspector(&s), spring(s), a(first), b(second), expected(value) {}
  void inspect() override {}
  void update() override {
    FFPoint force[3]; spring.init_forces(force,2); spring.compute_forces_edges(100,force,2);
    FF_CHECK(force[a->getValue()].v[0] == expected && force[b->getValue()].v[0] == -expected);
    for (int k : {1,2}) FF_CHECK(force[a->getValue()].v[k] == 0 && force[b->getValue()].v[k] == 0);
    spring.stop();
  }
};
void legacyWeightLaws() {
  for (int dimension : {2,3}) for (bool reversed : {false,true}) for (int multiplicity : {1,2}) {
    FFGraph graph(YES); graphics(graph);
    auto a = graph->addNode(FFPoint(0,0,0)), b = graph->addNode(FFPoint(100,0,0));
    for (int i = 0; i < multiplicity; ++i) graph->addEdge(reversed ? b : a,reversed ? a : b)->setWeight(2);
    auto* spring = new Headless<FFSpring>; graph->addGraphLabel(spring);
    spring->setDimension(dimension); FF_CHECK(spring->setQuality(0));
    spring->controls(new ForceOracle(*spring,a,b,multiplicity*(dimension == 2 ? 150 : 350)));
    FFSet<FFNode> fixed; fixed.add(a); fixed.add(b); spring->layout(fixed);
  }
}
ffgraph_test::LayoutSnapshot offsetDrawing(int orientation, bool grid, int size, bool shifted) {
  FFGraph graph(YES); graphics(graph);
  std::vector<FFNode> nodes;
  for (int i = 0; i < 5; ++i) { auto n = graph->addNode(); n->setValue(i+1); nodes.push_back(n); }
  int id = 0;
  for (auto [a,b] : {std::pair<int,int>{0,1},{1,2},{2,3},{3,4},{0,4},{0,4},{2,2}})
    graph->addEdge(nodes[a],nodes[b])->setValue(++id);
  const auto ids = ffgraph_test::layoutIds(*graph.operator->());
  auto* layout = new Headless<FFSugi>; graph->addGraphLabel(layout);
  layout->setOrient(orientation); layout->setUseGrid(grid); layout->setSameSize(size);
  if (shifted) { FF_CHECK(layout->setxoff(31)); FF_CHECK(layout->setyoff(47)); }
  layout->layout(); return ffgraph_test::captureLayout(*graph.operator->(),ids);
}
void offsets() {
  for (int orientation : {1,2}) for (bool grid : {false,true}) for (int size : {1,2}) {
    auto expected = offsetDrawing(orientation,grid,size,false);
    for (auto& [id,name,point] : expected.nodes) { point[0] += 31; point[1] += 47; }
    for (auto& [a,b,name,weight,route] : expected.edges)
      for (auto& point : route) { point[0] += 31; point[1] += 47; }
    FF_CHECK(ffgraph_test::sameLayout(expected,offsetDrawing(orientation,grid,size,true)));
  }
}
}
int main() {
  sugiyamaControls(); springControls(); legacyWeightLaws(); offsets();
  std::cout << "layout selectors, start-node flag, iteration cap, pinned nodes and legacy weight laws verified\n";
}
