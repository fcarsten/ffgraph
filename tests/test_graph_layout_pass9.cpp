// Headless Pass 9 layout smoke/regression test: spring's straightenEdges path.
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFSpring.h>
#include <ffgraph/FFRandom.h>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace {
void check(bool condition, const char* why) {
  if (!condition) { std::cerr << "layout invariant: " << why << '\n'; std::abort(); }
}
// Spring's default inspector creates a Tcl/Tk window during init. Keep the
// real layout implementation but use a headless graph observer for this test.
struct HeadlessSpring : FFSpring {
  void init(const FFGraph& g) override { FFLayoutLabel::init(g); }
  FFGraphLabel* proto() const override { return new HeadlessSpring; }
};

void fixture(int size, bool addLoop, bool addParallel) {
  FFGraph graph(YES);
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  std::vector<FFNode> nodes;
  for (int i = 0; i < size; ++i)
    nodes.push_back(graph->addNode(FFPoint(i * 10, i % 2 * 20)));
  for (int i = 1; i < size; ++i)
    graph->addEdge(nodes[i-1], nodes[i]);
  if (size > 2) graph->addEdge(nodes.back(), nodes.front());
  if (addLoop && size) graph->addEdge(nodes[0], nodes[0]);
  if (addParallel && size > 1) graph->addEdge(nodes[0], nodes[1]);
  for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
    FFGraphicEdge* ge = (*graph->allEdges()(p))();
    check(ge && ge->anzPoints() == 2, "initial edge route");
    ge->addPointAt(1, FFPoint(12, 13));
  }
  auto* spring = new HeadlessSpring;
  graph->addGraphLabel(spring);
  check(spring->setMaxIteration(3), "set bounded iterations");
  spring->layout();
  for (FFNode n : nodes) {
    FFGraphicNode* gn = (*n)();
    check(gn, "graphic node exists");
    const FFPoint& p = gn->getCoords();
    for (int j = 0; j < 3; ++j) check(std::isfinite(p.v[j]), "finite node coordinate");
  }
  for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
    FFGraphicEdge* ge = (*graph->allEdges()(p))();
    check(ge && ge->anzPoints() == 2, "spring straightens edge routing");
  }
  graph->clear();
}
}
int main() {
  fixture(0, false, false);
  fixture(1, false, false);
  fixture(2, false, false);
  fixture(4, false, false);
  fixture(6, true, true);
}
