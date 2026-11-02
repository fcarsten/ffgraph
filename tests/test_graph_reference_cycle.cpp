// A graph owns its FFGraphLabel instances, so labels and their inspectors
// must observe the graph without incrementing its intrusive refcount.
#include "test_check.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraphLabel.h>

namespace {
struct CountingGraph : FFGraphClass {
  static int destroyed;
  explicit CountingGraph(BOOL bare) : FFGraphClass(bare) {}
  ~CountingGraph() override { ++destroyed; }
};
int CountingGraph::destroyed = 0;

struct CountingLabel : FFGraphLabel {
  static int destroyed;
  String labelType;
  explicit CountingLabel(const String& type) : labelType(type) { _setType(type); }
  FFGraphLabel* proto() const override { return new CountingLabel(labelType); }
  void event(labelEvent what, labelEventStruct*) override {
    if (what == DEL_GRAPH) {
      // This temporary owning handle must not re-enter graph deletion.
      FFGraph temporary = getGraph();
      FF_CHECK(temporary->getGraphLabelIndex(labelType) >= 0);
    }
  }
  ~CountingLabel() override { ++destroyed; }
};
int CountingLabel::destroyed = 0;
}

int main()
{
  // With the original owning FFGraphLabel::graph, this label would keep the
  // graph alive after its last external handle was released.
  {
    FFGraph owner(new CountingGraph(YES));
    FFGraphClass* const raw = owner.operator->();
    const int index = owner->addGraphLabel(new CountingLabel("cycle_direct"));
    auto* label = owner->getGraphLabel(index);
    FF_CHECK(label != nullptr);
    {
      FFGraph alias = label->getGraph();
      FF_CHECK(alias.operator->() == raw);
      FF_CHECK(CountingGraph::destroyed == 0);
    }
    FF_CHECK(CountingGraph::destroyed == 0);
  }
  FF_CHECK(CountingGraph::destroyed == 1);
  FF_CHECK(CountingLabel::destroyed == 1);

  // A copied-out FFGraph is an ordinary owning handle: the graph must stay
  // alive until both external owners are released.
  {
    FFGraph retained(YES);
    {
      FFGraph owner(new CountingGraph(YES));
      const int index = owner->addGraphLabel(new CountingLabel("cycle_alias"));
      retained = owner->getGraphLabel(index)->getGraph();
    }
    FF_CHECK(CountingGraph::destroyed == 1);
    FF_CHECK(retained->getGraphLabelIndex("cycle_alias") >= 0);
  }
  FF_CHECK(CountingGraph::destroyed == 2);
  FF_CHECK(CountingLabel::destroyed == 2);

  // Default labels are initialized *inside* FFGraphClass construction.
  // Temporary FFGraph(this) handles must not delete a partially constructed
  // graph before the caller has acquired its first owning handle.
  static CountingLabel defaultLabel("cycle_default");
  addDefaultGraphLabel(&defaultLabel);
  {
    FFGraph owner(NO);
    FF_CHECK(owner->getGraphLabelIndex("cycle_default") >= 0);
    FF_CHECK(owner->getGraphLabel("cycle_default")->getGraph().operator->()
           == owner.operator->());
  }
  FF_CHECK(CountingLabel::destroyed == 3);

  // Copy constructs a graph with reference-count 0 until the returned graph
  // is adopted by an owning FFGraph. Both graphs must be independently freed.
  {
    FFGraph original(new CountingGraph(YES));
    original->addGraphLabel(new CountingLabel("cycle_copy"));
    FFGraph copy(original->copy());
    FF_CHECK(copy.operator->() != original.operator->());
    FF_CHECK(copy->getGraphLabelIndex("cycle_copy") >= 0);
  }
  // Copying into a non-bare graph also constructs its default label.
  FF_CHECK(CountingLabel::destroyed == 6);
}
