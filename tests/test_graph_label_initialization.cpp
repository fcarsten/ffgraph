// Replacing a graph label must initialize its observer just like adding one.
#include <cassert>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraphLabel.h>

namespace {
struct InitializedLabel : FFGraphLabel {
  static int destroyed;
  static int initialized;
  InitializedLabel() { _setType("initialization_test"); }
  FFGraphLabel* proto() const override { return new InitializedLabel; }
  void init(const FFGraph& graph) override {
    FFGraphLabel::init(graph);
    ++initialized;
    // Replacement labels must already occupy their destination slot.
    assert(getGraph().operator->() == graph.operator->());
    assert(graph->getGraphLabel(index) == this);
  }
  ~InitializedLabel() override { ++destroyed; }
};
int InitializedLabel::initialized = 0;
int InitializedLabel::destroyed = 0;
}

int main() {
  {
    FFGraph graph(YES);
    const int index = graph->addGraphLabel(new InitializedLabel);
    assert(InitializedLabel::initialized == 1);
    assert(graph->getGraphLabel(index)->getGraph().operator->() == graph.operator->());

    graph->setGraphLabel(new InitializedLabel, index);
    assert(InitializedLabel::initialized == 2);
    assert(InitializedLabel::destroyed == 1);
    assert(graph->getGraphLabel(index)->getGraph().operator->() == graph.operator->());

    graph->setGraphLabel(new InitializedLabel); // type-based replacement
    assert(InitializedLabel::initialized == 3);
    assert(InitializedLabel::destroyed == 2);
    auto* current = graph->getGraphLabel(index);
    assert(current->getGraph().operator->() == graph.operator->());

    graph->setGraphLabel(current); // alias, must not reinitialize
    graph->setGraphLabel(current, index);
    graph->setGraphLabel(graph->getGraphLabels()); // own container
    assert(InitializedLabel::initialized == 3);
    assert(InitializedLabel::destroyed == 2);
  }
  assert(InitializedLabel::destroyed == 3);
}
