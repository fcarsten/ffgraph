// Actual Rome production imports in 2D/3D windows, including update suppression,
// rejection, merge recovery and deferred interpreter cleanup.
#include "test_check.h"
#include "graph_corpus/graph_snapshot.h"
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFTcl3DWindow.h>
#include <ffgraph/FFGraphLabel.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/ffError.h>
#include <vector>

extern charErrorHandler* userCharErrorHandler;
namespace {
int errors = 0;
void error(errGrade grade, const char* message, const char*, int) {
  FF_CHECK(grade == USER_ERROR); std::cerr << message << '\n'; ++errors;
}
template<class Base> struct Window final : Base {
  std::vector<double> state() const {
    std::vector<double> result{this->scale,this->alpha,this->beta,this->gamma,this->labelXoff,this->labelYoff};
    for (const auto* matrix : {&this->ctm,&this->invCtm})
      for (const auto& row : matrix->row) for (double x : row.v) result.push_back(x);
    return result;
  }
};
struct Events final : FFGraphLabel {
  int starts = 0, ends = 0;
  Events() { _setType("RomeWindowEvents"); }
  Events* proto() const override { return new Events; }
  void event(labelEvent kind, labelEventStruct*) override {
    if (kind == START_LOAD) ++starts;
    if (kind == END_LOAD) ++ends;
  }
};
void oracle(FFGraph& graph, FFNodeClass* original, FFEdgeClass* oldEdge) {
  FF_CHECK(!graph->getDirected() && graph->getName() == "Rome window");
  FF_CHECK(graph->getNumNodes() == 4 && graph->getNumEdges() == 4);
  FF_CHECK(graph->allNodes().contains(original) && original->getValue() == -1 && original->getName() == "original");
  FF_CHECK(original->getGraphicNode()->getx() == 7 && original->getGraphicNode()->gety() == 8 && original->getGraphicNode()->getz() == 9);
  FF_CHECK(graph->getStartNode() == original && graph->getEndNodes().contains(original));
  FF_CHECK(graph->allEdges().contains(oldEdge) && oldEdge->getValue() == 456 && oldEdge->getWeight() == 2.5);
  auto* a = graph->nodeWithValue(1); auto* b = graph->nodeWithValue(2);
  FF_CHECK(a && b && graph->nodeWithValue(9));
  int parallels = 0, loops = 0;
  for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
    auto* edge = graph->allEdges()(p);
    FF_CHECK(graph->allNodes().contains(edge->source()) && graph->allNodes().contains(edge->target()));
    if (edge == oldEdge) continue;
    FF_CHECK(edge->getWeight() == 1 && edge->getGraphicEdge());
    if (edge->source() == a && edge->target() == b) ++parallels;
    else { FF_CHECK(edge->source() == a && edge->target() == a); ++loops; }
  }
  FF_CHECK(parallels == 2 && loops == 1);
}
template<class Base> void exercise(const char* good, const char* bad) {
  const int initial = Tk_GetNumMainWindows();
  for (bool update : {false,true}) {
    {
      FFGraph graph(YES); auto* window = new Window<Base>; graph->createGraphic(window);
      graph->setName("Rome window"); graph->_setDirected(YES);
      auto* original = graph->addNode(FFPoint(7,8,9)); original->setValue(123); original->setName("original");
      auto* oldEdge = graph->addEdge(original,original); oldEdge->setValue(456); oldEdge->setWeight(2.5);
      graph->setStartNode(original); graph->addEndNode(original);
      auto* events = new Events; graph->addGraphLabel(events); window->setUpdate(update);
      const int before = errors;
      graph->load(String(good),YES);
      FF_CHECK(errors == before && window->getUpdate() == update && Tk_GetNumMainWindows() == initial+1);
      FF_CHECK(events->starts == 1 && events->ends == 1); oracle(graph,original,oldEdge);
      const auto state = window->state();
      const auto snapshot = ffgraph_test::capture(*graph.operator->());
      auto* first = graph->nodeWithValue(1);
      graph->load(String(bad),YES);
      FF_CHECK(errors == before+1 && window->getUpdate() == update);
      FF_CHECK(events->starts == 2 && events->ends == 2 && window->state() == state);
      FF_CHECK(ffgraph_test::difference(snapshot,ffgraph_test::capture(*graph.operator->())).empty());
      FF_CHECK(graph->nodeWithValue(1) == first); oracle(graph,original,oldEdge);
      graph->load(String(good),YES);
      FF_CHECK(errors == before+1 && window->getUpdate() == update);
      FF_CHECK(events->starts == 3 && events->ends == 3);
      FF_CHECK(graph->getNumNodes() == 7 && graph->getNumEdges() == 7);
      FF_CHECK(graph->allNodes().contains(first) && first->getValue() == -1 && graph->nodeWithValue(1) != first);
      graph->removeGraphic();
    }
    FFWindow::collectPendingInterpreters(); FF_CHECK(Tk_GetNumMainWindows() == initial);
  }
}
}
int main(int argc, char** argv) {
  FF_CHECK(argc == 3); Tcl_FindExecutable(argv[0]);
  const auto previous = userCharErrorHandler; userCharErrorHandler = error;
  exercise<FFTclWindow>(argv[1],argv[2]); exercise<FFTcl3DWindow>(argv[1],argv[2]);
  FFWindow::collectPendingInterpreters(); Tcl_Finalize(); userCharErrorHandler = previous;
}
