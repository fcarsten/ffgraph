#include "test_check.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFTcl3DWindow.h>
#include <ffgraph/FFTclCompat.h>
#include <string>

namespace {
void eval(Tcl_Interp* interp, const char* script) {
  if (Tcl_Eval(interp, script) != TCL_OK) {
    std::cerr << Tcl_GetStringResult(interp) << '\n';
    FF_CHECK(false);
  }
}
void exercise(bool threeD) {
  FFGraph source(YES), destination(YES);
  auto* window = threeD ? static_cast<FFTclBasicWindow*>(new FFTcl3DWindow)
                        : static_cast<FFTclBasicWindow*>(new FFTclWindow);
  source->createGraphic(window);
  auto a = source->addNode(FFPoint(30,40)), b = source->addNode(FFPoint(100,120));
  a->setName("first"); b->setName("second"); source->addEdge(a,b);
  window->updateGraph();
  auto* interp = window->getInterpreter();
  eval(interp, "update idletasks; if {![winfo exists .frame0.canvas2]} {error {missing canvas}}");
  eval(interp, "setBusy; unsetBusy");
#if defined(_WIN32) || defined(FFGRAPH_NO_XPM)
  eval(interp, "if {[lsearch -exact [image types] pixmap] >= 0} {error {unexpected pixmap handler}};"
               "if {[createIcon deprecated.xpm] ne {}} {error {Xpm should be omitted}}");
#endif
  window->setCurrent(a->getGraphicNode()); window->addToCurrent(b->getGraphicNode());
  eval(interp, "recall modPress c");
  auto* target = new FFTclWindow;
  destination->createGraphic(target);
  eval(target->getInterpreter(), "recall modPress v; update idletasks");
  FF_CHECK(destination->getNumNodes() == 2 && destination->getNumEdges() == 1);
  destination->removeGraphic(); source->removeGraphic();
}
}
int main(int argc, char** argv) {
  Tcl_FindExecutable(argc ? argv[0] : "test_native_gui_smoke");
  const int before = Tk_GetNumMainWindows();
  exercise(false); FFWindow::collectPendingInterpreters();
  exercise(true); FFWindow::collectPendingInterpreters();
  FF_CHECK(Tk_GetNumMainWindows() == before);
}
