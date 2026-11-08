#include "test_check.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFTcl3DWindow.h>
#include <ffgraph/FFTclCompat.h>
#include <ffgraph/ffError.h>
#include <cmath>

extern charErrorHandler* userCharErrorHandler;
namespace {
int errors = 0;
void unexpected(errGrade, const char* message, const char*, int) {
  ++errors; std::cerr << message << '\n';
}
void checked(Tcl_Interp* interp, int status) {
  if (status != TCL_OK) std::cerr << Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY) << '\n';
  FF_CHECK(status == TCL_OK);
}
bool same(const FFPoint& a, const FFPoint& b) {
  for (int i = 0; i < 3; ++i) if (std::abs(a.v[i] - b.v[i]) > 1e-6) return false;
  return true;
}
void exercise(bool threeD) {
  FFGraph graph(YES);
  auto* window = threeD ? static_cast<FFTclBasicWindow*>(new FFTcl3DWindow)
                        : static_cast<FFTclBasicWindow*>(new FFTclWindow);
  graph->createGraphic(window);
  auto* interp = window->getInterpreter();
  checked(interp, Tcl_Eval(interp, R"tcl(
    set editClock 10000
    set editError {}
    proc bgerror {message} {set ::editError $::errorInfo}
    proc editPress {x y modifiers} {
      # Explicit times avoid accidentally opening the double-click inspector.
      incr ::editClock 1000
      event generate .frame0.canvas2 <Motion> -x $x -y $y -state 0 -time $::editClock
      event generate .frame0.canvas2 <${modifiers}ButtonPress-1> -x $x -y $y -time $::editClock
    }
    proc editRelease {x y modifiers} {
      event generate .frame0.canvas2 <${modifiers}ButtonRelease-1> -x $x -y $y -time [incr ::editClock 20]
      update idletasks
      if {$::editError ne ""} {error $::editError}
    }
    proc editCenter {id} {
      set c .frame0.canvas2
      lassign [$c coords $id] x1 y1 x2 y2
      return [list [expr {round(($x1+$x2)/2 - [$c canvasx 0])}] \
                   [expr {round(($y1+$y2)/2 - [$c canvasy 0])}]]
    }
    proc editClick {id modifiers} {
      lassign [editCenter $id] x y
      editPress $x $y $modifiers
      editRelease $x $y $modifiers
    }
    proc editDrag {id modifiers} {
      lassign [editCenter $id] x y
      editPress $x $y $modifiers
      incr x 35
      incr y 15
      event generate .frame0.canvas2 <${modifiers}B1-Motion> -x $x -y $y -time [incr ::editClock 20]
      editRelease $x $y $modifiers
    }
    proc editCreatePair {locks} {
      foreach {x y} {120 120 300 210} {
        editPress $x $y ${locks}Control-
        editRelease $x $y ${locks}Control-
      }
    }
    proc editKeyTrace {command operation} {
      if {[lindex $command 1] in {keyPress modPress}} {
        lappend ::editKeyCommands [lindex $command 1]
      }
    }
    proc editKeys {locks alt} {
      focus -force .frame0.canvas2
      update
      set ::editKeyCommands {}
      trace add execution recall enter editKeyTrace
      try {
        foreach modifiers [list $locks ${locks}${alt}] {
          event generate .frame0.canvas2 <${modifiers}KeyPress> -keysym F12
          event generate .frame0.canvas2 <${modifiers}KeyRelease> -keysym F12
        }
      } finally {
        trace remove execution recall enter editKeyTrace
      }
      if {$::editKeyCommands ne {keyPress modPress}} {
        error "Lock/Alt keyboard routing is incorrect: $::editKeyCommands"
      }
    }
    update
  )tcl"));
  // Windows Tk maps Num Lock to Mod1 and Scroll Lock to Mod3. On X11,
  // exercise the usual Num Lock Mod2 mapping, without treating Alt as a lock.
  const bool windows = std::string(Tcl_GetVar(interp, "tcl_platform(platform)", TCL_GLOBAL_ONLY)) == "windows";
  const char* numLock = windows ? "Mod1-" : "Mod2-";
  const char* physicalAlt = windows ? "Mod2-" : "Mod1-";
  const std::string allLocks = std::string("Lock-Mod3-") + numLock;
  for (const char* locks : {numLock, "", "Lock-", allLocks.c_str()}) {
    std::cout << (threeD ? "3D" : "2D") << " lock modifiers: " << locks << std::endl;
    graph->clear();
    checked(interp, FF_TclEvalObjv(interp, {"editCreatePair", locks}));
    FF_CHECK(graph->getNumNodes() == 2 && graph->getNumEdges() == 0);
    Pix p = graph->allNodes().first();
    FFNode source = graph->allNodes()(p);
    graph->allNodes().next(p);
    FFNode target = graph->allNodes()(p);
    auto* first = source->getGraphicNode();
    auto* second = target->getGraphicNode();
    const FFPoint position = first->getCoords(), range = first->getRange();
    checked(interp, FF_TclEvalObjv(interp, {"editClick", first->getId().c_str(), locks}));
    checked(interp, FF_TclEvalObjv(interp, {"editDrag", first->getId().c_str(), locks}));
    FF_CHECK(same(range, first->getRange()));
    FF_CHECK(!same(position, first->getCoords()));
    const FFPoint moved = first->getCoords(), other = second->getCoords();
    const std::string control = std::string(locks) + "Control-";
    checked(interp, FF_TclEvalObjv(interp, {"editClick", second->getId().c_str(), control.c_str()}));
    FF_CHECK(graph->getNumEdges() == 1);
    auto edge = graph->allEdges()(graph->allEdges().first());
    FF_CHECK(edge->source() == source && edge->target() == target);
    FF_CHECK(same(moved, first->getCoords()) && same(other, second->getCoords()));
    const std::string alt = std::string(locks) + physicalAlt;
    checked(interp, FF_TclEvalObjv(interp, {"editDrag", first->getId().c_str(), alt.c_str()}));
    FF_CHECK(same(moved, first->getCoords()));
    FF_CHECK(!same(range, first->getRange()));
    checked(interp, FF_TclEvalObjv(interp, {"editKeys", locks, physicalAlt}));
  }
  graph->removeGraphic();
}
}
int main(int argc, char** argv) {
  Tcl_FindExecutable(argc ? argv[0] : "test_canvas_editing");
  userCharErrorHandler = unexpected;
  exercise(false); FFWindow::collectPendingInterpreters();
  exercise(true); FFWindow::collectPendingInterpreters();
  FF_CHECK(errors == 0);
  std::cout << "Canvas node creation, movement, edges and Alt-resize verified with lock modifiers in 2D/3D\n";
}
