// Replacement is an ownership transfer, but replacing an object with itself
// must never free the object that remains in the graph.
#include "test_check.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFNodeLabel.h>
#include <ffgraph/FFEdgeLabel.h>
#include <ffgraph/FFGraphLabel.h>

struct TestNodeLabel : FFNodeLabel {
  static int destroyed;
  TestNodeLabel() { _setType("ownership_test_node"); }
  FFNodeLabel* proto() const override { return new TestNodeLabel; }
  ~TestNodeLabel() override { ++destroyed; }
};
int TestNodeLabel::destroyed = 0;

struct TestEdgeLabel : FFEdgeLabel {
  static int destroyed;
  TestEdgeLabel() { _setType("ownership_test_edge"); }
  FFEdgeLabel* proto() const override { return new TestEdgeLabel; }
  ~TestEdgeLabel() override { ++destroyed; }
};
int TestEdgeLabel::destroyed = 0;

struct TestGraphLabel : FFGraphLabel {
  static int destroyed;
  TestGraphLabel() { _setType("ownership_test_graph"); }
  FFGraphLabel* proto() const override { return new TestGraphLabel; }
  // This test label intentionally does not retain its owner graph; testing
  // the legacy FFGraphLabel reference cycle is separate from alias safety.
  void init(const FFGraph&) override {}
  ~TestGraphLabel() override { ++destroyed; }
};
int TestGraphLabel::destroyed = 0;

int main() {
  {
    FFGraph graph(YES); // bare: no GUI or global default labels
    const int ni = graph->addNodeLabel(new TestNodeLabel);
    const int ei = graph->addEdgeLabel(new TestEdgeLabel);
    const int gi = graph->addGraphLabel(new TestGraphLabel);
    FFNode a = graph->addNode();
    FFNode b = graph->addNode();
    FFEdge e = graph->addEdge(a, b);

    FFNodeLabel* nl = a->getLabel(ni);
    FFEdgeLabel* el = e->getLabel(ei);
    FFGraphLabel* gl = graph->getGraphLabel(gi);
    FF_CHECK(nl && el && gl);
    const int nd = TestNodeLabel::destroyed;
    const int ed = TestEdgeLabel::destroyed;
    const int gd = TestGraphLabel::destroyed;

    a->setLabel(nl, ni);                 // explicit index
    a->setLabel(nl);                     // lookup by type
    a->setLabel(*a->getLabels());        // self-container
    e->setLabel(el, ei);
    e->setLabel(el);
    e->setLabel(*e->getLabels());
    graph->setGraphLabel(gl, gi);
    graph->setGraphLabel(gl);
    graph->setGraphLabel(graph->getGraphLabels());

    FF_CHECK(a->getLabel(ni) == nl);
    FF_CHECK(e->getLabel(ei) == el);
    FF_CHECK(graph->getGraphLabel(gi) == gl);
    FF_CHECK(TestNodeLabel::destroyed == nd);
    FF_CHECK(TestEdgeLabel::destroyed == ed);
    FF_CHECK(TestGraphLabel::destroyed == gd);

    a->setLabel(new TestNodeLabel, ni);
    e->setLabel(new TestEdgeLabel, ei);
    graph->setGraphLabel(new TestGraphLabel, gi);
    FF_CHECK(TestNodeLabel::destroyed == nd + 1);
    FF_CHECK(TestEdgeLabel::destroyed == ed + 1);
    FF_CHECK(TestGraphLabel::destroyed == gd + 1);
  }
  // All graph-owned labels and their prototypes must be destroyed.
  FF_CHECK(TestNodeLabel::destroyed == 4); // prototype and 2 node clones + replacement
  FF_CHECK(TestEdgeLabel::destroyed == 3); // prototype, old edge label, replacement
  FF_CHECK(TestGraphLabel::destroyed == 2);
}
