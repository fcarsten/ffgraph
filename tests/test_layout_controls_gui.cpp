#include "test_check.h"
#include <ffgraph/FFSugi.h>
#include <ffgraph/FFSpring.h>
#include <ffgraph/FFSugiInsp.h>
#include <ffgraph/FFSpringInsp.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFTcl3DWindow.h>
#include <ffgraph/FFTclGraphInsp.h>
#include <ffgraph/FFTclCompat.h>
#include <ffgraph/FFOrthoInsp.h>
#include <ffgraph/ffError.h>
#include <ffgraph/FFGraphicEdge.h>
#include <tk.h>
#include <string>

extern charErrorHandler* userCharErrorHandler;
namespace {
int errors = 0;
void unexpected(errGrade, const char* message, const char*, int) {++errors; std::cerr << message << '\n';}
struct Sugi : FFSugi { FFSugiInsp* controls() { return static_cast<FFSugiInsp*>(inspector); } };
struct Spring : FFSpring { FFSpringInsp* controls() { return static_cast<FFSpringInsp*>(inspector); } };
struct Ortho : FFOrtho { FFOrthoInsp* controls() { return static_cast<FFOrthoInsp*>(inspector); } };
void eval(Tcl_Interp* i, const char* script, const char* expected = nullptr) {
  if (Tcl_Eval(i,script) != TCL_OK) { std::cerr << Tcl_GetStringResult(i) << '\n'; FF_CHECK(false); }
  if (expected) FF_CHECK(std::string(Tcl_GetStringResult(i)) == expected);
}
void exercise(bool threeD) {
  FFGraph graph(YES);
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  auto* spring = new Spring; graph->addGraphLabel(spring);
  auto* sugi = new Sugi; graph->addGraphLabel(sugi);
  auto* ortho = new Ortho; graph->addGraphLabel(ortho);
  auto a = graph->addNode(FFPoint(0,0,0)), b = graph->addNode(FFPoint(1000,500,0));
  a->setValue(1); b->setValue(2); graph->addEdge(a,b);
  auto* window = threeD ? static_cast<FFTclBasicWindow*>(new FFTcl3DWindow)
                        : static_cast<FFTclBasicWindow*>(new FFTclWindow);
  graph->createGraphic(window);
  {
    FFTclGraphInsp choices(window);
    auto* chooser = choices.getInterpreter();
    eval(chooser, R"tcl(
      proc findLayoutEntry {menu name} {
        for {set i 0} {$i <= [$menu index end]} {incr i} {
          if {[$menu type $i] eq "cascade"} {
            set found [findLayoutEntry [$menu entrycget $i -menu] $name]
            if {$found ne ""} {return $found}
          } elseif {[$menu type $i] eq "command" && [$menu entrycget $i -label] eq $name} {
            return [list $menu $i]
          }
        }
        return ""
      }
      proc selectLayoutInspector {name stale} {
        lassign [findLayoutEntry .insp_graph.type.graph.menubutton2.m $name] menu entry
        if {$menu eq ""} {error "Missing layout menu entry: $name"}
        # Native menus can clear the highlight before invoking the command.
        # Also test a different highlighted item: the invoked entry must win.
        if {$stale} {$menu activate [expr {$entry == 0 ? 1 : 0}]} else {$menu activate none}
        $menu invoke $entry
        if {[OptionButtonGet .insp_graph.type.graph] ne $name} {
          error "Layout selection lost its entry label: $name"
        }
      }
    )tcl");
    struct Layout {FFGraphLabel* model; FFTclInspector* controls;};
    const Layout layouts[] = {{sugi, sugi->controls()}, {spring, spring->controls()}, {ortho, ortho->controls()}};
    for (const auto& layout : layouts) {
      auto* controls = layout.controls->getInterpreter();
      eval(controls, "wm state .", "withdrawn");
      for (const char* stale : {"0", "1"}) {
        const std::string name = layout.model->baseType().c_str();
        const int status = FF_TclEvalObjv(chooser, {"selectLayoutInspector", name.c_str(), stale});
        if (status != TCL_OK) std::cerr << Tcl_GetStringResult(chooser) << '\n';
        FF_CHECK(status == TCL_OK);
        eval(controls, "update; wm state .", "normal");
        eval(controls, "winfo ismapped .", "1");
        eval(controls, "recall deleteWindow; update; wm state .", "withdrawn");
      }
    }
  }
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
  userCharErrorHandler = unexpected;
  const int initial = Tk_GetNumMainWindows();
  exercise(false); FFWindow::collectPendingInterpreters();
  exercise(true); FFWindow::collectPendingInterpreters();
  FF_CHECK(errors == 0);
  FF_CHECK(Tk_GetNumMainWindows() == initial);
  std::cout << "actual Spring/Sugiyama inspector controls and resource lookup verified\n";
}
