// Replacing a graph label must initialize its observer just like adding one.
#include "test_check.h"
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
    FF_CHECK(getGraph().operator->() == graph.operator->());
    FF_CHECK(graph->getGraphLabel(index) == this);
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
    FF_CHECK(InitializedLabel::initialized == 1);
    FF_CHECK(graph->getGraphLabel(index)->getGraph().operator->() == graph.operator->());

    graph->setGraphLabel(new InitializedLabel, index);
    FF_CHECK(InitializedLabel::initialized == 2);
    FF_CHECK(InitializedLabel::destroyed == 1);
    FF_CHECK(graph->getGraphLabel(index)->getGraph().operator->() == graph.operator->());

    graph->setGraphLabel(new InitializedLabel); // type-based replacement
    FF_CHECK(InitializedLabel::initialized == 3);
    FF_CHECK(InitializedLabel::destroyed == 2);
    auto* current = graph->getGraphLabel(index);
    FF_CHECK(current->getGraph().operator->() == graph.operator->());

    graph->setGraphLabel(current); // alias, must not reinitialize
    graph->setGraphLabel(current, index);
    graph->setGraphLabel(graph->getGraphLabels()); // own container
    FF_CHECK(InitializedLabel::initialized == 3);
    FF_CHECK(InitializedLabel::destroyed == 2);
  }
  FF_CHECK(InitializedLabel::destroyed == 3);
}
