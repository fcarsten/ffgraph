#include "test_check.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFTcl3DWindow.h>
#include <ffgraph/ffError.h>
#include <cmath>

extern charErrorHandler* userCharErrorHandler;
namespace {
int errors = 0;
void unexpected(errGrade, const char* message, const char*, int) {++errors; std::cerr << message << '\n';}
void eval(Tcl_Interp* interp, const char* script) {
  const int status = Tcl_Eval(interp, script);
  if (status != TCL_OK) std::cerr << Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY) << '\n';
  FF_CHECK(status == TCL_OK);
}
void exercise(bool threeD) {
  FFGraph graph(YES);
  auto* window = threeD ? static_cast<FFTclBasicWindow*>(new FFTcl3DWindow)
                        : static_cast<FFTclBasicWindow*>(new FFTclWindow);
  graph->createGraphic(window);
  auto* interp = window->getInterpreter();
  eval(interp, R"tcl(
    proc check {condition message} {if {![uplevel 1 [list expr $condition]]} {error $message}}
    proc zoomDrag {x1 y1 x2 y2} {
      set ::workingMode Zoom
      button1 .frame0.canvas2 $x1 $y1
      check {$::draging == 1} {Test did not begin a background drag}
      button1Motion recall .frame0.canvas2 $x2 $y2
      button1Release recall .frame0.canvas2 $x2 $y2
      check {$::draging == 0 && $::workingMode eq "Create"} {Zoom left the drag active}
      check {$::dragBoxX == 0 && $::dragBoxY == 0} {Zoom left stale drag coordinates}
    }
    update
    check {[.frame0.canvas2 cget -scrollregion] eq ""} {Expected an initially empty scroll region}
    # A fresh empty graph has no bounds, even while the drag rectangle exists.
    zoomDrag 40 50 240 190
    check {[.frame0.canvas2 cget -scrollregion] eq ""} {Empty zoom invented graph bounds}
    centerCanvas .frame0.canvas2 100 100
    recall resetScales
  )tcl");
  {
    const FFPoint before = window->proj(FFPoint(10,20,0));
    eval(interp, "zoomDrag 40 50 40 50");
    const FFPoint after = window->proj(FFPoint(10,20,0));
    for (int i=0; i<3; ++i) FF_CHECK(before.v[i] == after.v[i]);
  }
  for (const char* gesture : {"zoomDrag 40 50 240 50", "zoomDrag 40 50 40 190"}) {
    const FFPoint before = window->proj(FFPoint(10,20,0));
    eval(interp, gesture);
    const FFPoint after = window->proj(FFPoint(10,20,0));
    FF_CHECK(before.v[0] != after.v[0] || before.v[1] != after.v[1]);
    for (int i=0; i<3; ++i) FF_CHECK(std::isfinite(after.v[i]));
    eval(interp, "recall resetScales");
  }
  auto first = graph->addNode(FFPoint(0,0,0));
  auto second = graph->addNode(FFPoint(100,80,0));
  const FFPoint firstPosition = first->getGraphicNode()->getCoords();
  const FFPoint secondPosition = second->getGraphicNode()->getCoords();
  const FFPoint projection = window->proj(secondPosition);
  eval(interp, "zoomDrag 20 20 220 160");
  const FFPoint zoomed = window->proj(secondPosition);
  FF_CHECK(projection.v[0] != zoomed.v[0] || projection.v[1] != zoomed.v[1]);
  for (int i=0; i<3; ++i) {
    FF_CHECK(firstPosition.v[i] == first->getGraphicNode()->getCoords().v[i]);
    FF_CHECK(secondPosition.v[i] == second->getGraphicNode()->getCoords().v[i]);
  }
  eval(interp, R"tcl(
    # Remove decorative insets for the numerical centering oracle.
    .frame0.canvas2 configure -borderwidth 0 -highlightthickness 0
    update idletasks
    .frame0.canvas2 configure -scrollregion {-100.5 -200.25 4000.75 5000.25}
    centerCanvas .frame0.canvas2 1200.25 1600.5
    set cx [.frame0.canvas2 canvasx [expr {[winfo width .frame0.canvas2]/2.0}]]
    set cy [.frame0.canvas2 canvasy [expr {[winfo height .frame0.canvas2]/2.0}]]
    check {abs($cx-1200.25) <= 2 && abs($cy-1600.5) <= 2} "Fractional region did not center correctly: $cx $cy"
    .frame0.canvas2 configure -scrollregion {0 0 40i 40i}
    centerCanvas .frame0.canvas2 1200 1600
    check {abs([.frame0.canvas2 canvasx [expr {[winfo width .frame0.canvas2]/2.0}]]-1200) <= 2} {Screen-unit region did not center horizontally}
    check {abs([.frame0.canvas2 canvasy [expr {[winfo height .frame0.canvas2]/2.0}]]-1600) <= 2} {Screen-unit region did not center vertically}
    .frame0.canvas2 configure -scrollregion {0 0 0 5000}
    centerCanvas .frame0.canvas2 0 2000
    check {abs([.frame0.canvas2 canvasy [expr {[winfo height .frame0.canvas2]/2.0}]]-2000) <= 2} {Zero-width region prevented vertical centering}
    .frame0.canvas2 configure -scrollregion {0 0 5000 0}
    centerCanvas .frame0.canvas2 2000 0
    check {abs([.frame0.canvas2 canvasx [expr {[winfo width .frame0.canvas2]/2.0}]]-2000) <= 2} {Zero-height region prevented horizontal centering}
    .frame0.canvas2 configure -scrollregion {0 0 0 0}
    centerCanvas .frame0.canvas2 0 0
  )tcl");
  graph->removeGraphic();
}
}
int main(int argc, char** argv) {
  Tcl_FindExecutable(argc ? argv[0] : "test_canvas_zoom");
  userCharErrorHandler = unexpected;
  exercise(false); FFWindow::collectPendingInterpreters();
  exercise(true); FFWindow::collectPendingInterpreters();
  FF_CHECK(errors == 0);
  std::cout << "Empty, degenerate and normal zoom gestures verified in 2D/3D\n";
}
