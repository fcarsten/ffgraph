// Pass 9 geometry regression: exercise the old MFPlex<FFPoint>/straightenEdges
// path without requiring a Tk window or running the full spring embedder.
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {
void check(bool ok, const char* message) {
  if (!ok) { std::cerr << "geometry invariant: " << message << '\n'; std::abort(); }
}
void finiteEdge(FFGraphicEdge* edge) {
  check(edge && edge->anzPoints() >= 2, "route needs endpoints");
  for (int i = 0; i < edge->anzPoints(); ++i) {
    const FFPoint& point = edge->getPointAt(i);
    for (int d = 0; d != 3; ++d)
      check(std::isfinite(point.v[d]), "route point finite");
  }
}
}
int main() {
  FFGraph graph(YES);
  const int nodeIndex = graph->addNodeLabel(new FFGraphicNode);
  const int edgeIndex = graph->addEdgeLabel(new FFGraphicEdge);
  graph->setNodeIndex(nodeIndex);
  graph->setEdgeIndex(edgeIndex);
  FFNode a = graph->addNode(FFPoint(0, 0));
  FFNode b = graph->addNode(FFPoint(0, 0)); // intentionally coincident
  FFNode c = graph->addNode(FFPoint(100, 10));
  FFEdge loop = graph->addEdge(a, a);
  FFEdge ab = graph->addEdge(a, b);
  FFEdge bc = graph->addEdge(b, c);
  FFGraphicEdge* routes[] = {(*loop)(), (*ab)(), (*bc)()};
  for (auto* route : routes) finiteEdge(route);
  for (int repeat = 0; repeat < 60; ++repeat) {
    for (auto* route : routes) {
      for (int j = 0; j != 9; ++j)
        check(route->addPointAt(1, FFPoint(j, repeat, 0)), "insert point");
      check(route->anzPoints() == 11, "inserted points count");
      finiteEdge(route);
      check(route->deletePointAt(1), "delete point");
      finiteEdge(route);
    }
    graph->straightenEdges();
    for (auto* route : routes) {
      check(route->anzPoints() == 2, "straightened edge has endpoints");
      finiteEdge(route);
    }
    graph->rev();
    for (auto* route : routes) finiteEdge(route);
  }
  graph->clear();
  check(graph->getNumEdges() == 0 && graph->getNumNodes() == 0, "clear with labels");
}
