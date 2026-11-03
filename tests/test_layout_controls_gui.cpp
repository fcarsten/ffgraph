#include "test_check.h"
#include <ffgraph/FFSugi.h>
#include <ffgraph/FFSpring.h>
#include <ffgraph/FFSugiInsp.h>
#include <ffgraph/FFSpringInsp.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFGraphicEdge.h>
#include <tk.h>
#include <string>

namespace {
struct Sugi : FFSugi { FFSugiInsp* controls() { return static_cast<FFSugiInsp*>(inspector); } };
struct Spring : FFSpring { FFSpringInsp* controls() { return static_cast<FFSpringInsp*>(inspector); } };
void eval(Tcl_Interp* i, const char* script, const char* expected = nullptr) {
  if (Tcl_Eval(i,script) != TCL_OK) { std::cerr << Tcl_GetStringResult(i) << '\n'; FF_CHECK(false); }
  if (expected) FF_CHECK(std::string(Tcl_GetStringResult(i)) == expected);
}
void exercise() {
  FFGraph graph(YES);
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  auto* spring = new Spring; graph->addGraphLabel(spring);
  auto* sugi = new Sugi; graph->addGraphLabel(sugi);
  auto a = graph->addNode(FFPoint(0,0,0)), b = graph->addNode(FFPoint(1000,500,0));
  a->setValue(1); b->setValue(2); graph->addEdge(a,b);
  auto* window = new FFTclWindow; graph->createGraphic(window);
  auto* si = sugi->controls()->getInterpreter();
  eval(si,"info commands Sugi_EntryInsert","Sugi_EntryInsert");
  eval(si,"winfo exists .middleframe.reduce_items.scale.scale","1");
  eval(si,"recall noneRed"); FF_CHECK(sugi->getCrossingAlg() == 2);
  eval(si,"set reduce","noneRed");
  eval(si,"recall startLayout");
  eval(si,"recall adjazenz"); FF_CHECK(sugi->getCrossingAlg() == 0);
  eval(si,"set reduce","adjazenzmatrix");
  eval(si,"recall toggleDecycle"); FF_CHECK(!sugi->getUseStartNode());
  eval(si,"recall toggleDecycle"); FF_CHECK(sugi->getUseStartNode());
  auto* sp = spring->controls()->getInterpreter();
  eval(sp,"winfo exists .quality.scale.scale","1");
  eval(sp,"recall setEdgeWeight"); FF_CHECK(!spring->getUseEdgeWeight());
  FF_CHECK(spring->setMaxIteration(1)); eval(sp,"recall quality 2; recall startLayout");
  eval(sp,"recall setEdgeWeight"); FF_CHECK(spring->getUseEdgeWeight());
  FF_CHECK(graph->getNumNodes() == 2 && graph->getNumEdges() == 1);
  graph->removeGraphic();
}
}
int main(int argc, char** argv) {
  Tcl_FindExecutable(argc ? argv[0] : "test_layout_controls_gui");
  const int initial = Tk_GetNumMainWindows();
  exercise(); FFWindow::collectPendingInterpreters();
  FF_CHECK(Tk_GetNumMainWindows() == initial);
  std::cout << "actual Spring/Sugiyama inspector controls and resource lookup verified\n";
}
