// Pass 10: exercise real Tcl/Tk interpreter and subwindow lifetimes under Xvfb.
// Uses only Tk's public APIs; no WSLg inspector/window-manager assumptions.
#include <ffgraph/FFWindow.h>
#include <ffgraph/FFTclSubWindow.h>
#include <tcl.h>
#include <tk.h>

#include <cstdlib>
#include <iostream>

namespace {
void check(bool condition, const char* message) {
  if (!condition) { std::cerr << "Tk lifecycle: " << message << '\n'; std::abort(); }
}
class Pass10Window : public FFWindow {
public:
  bool start() {
    Tcl_Interp* i = getInterpreter();
    if (Tcl_Init(i) != TCL_OK || Tk_Init(i) != TCL_OK) {
      std::cerr << "Tk initialise: " << Tcl_GetStringResult(i) << '\n';
      return false;
    }
    w = Tk_MainWindow(i);
    return w != nullptr;
  }
};
void deleted(ClientData data, Tcl_Interp*) {
  ++*static_cast<int*>(data);
}
int closeOwner(ClientData data, Tcl_Interp*, int, const char**) {
  Pass10Window** owner = static_cast<Pass10Window**>(data);
  delete *owner;
  *owner = nullptr;
  return TCL_OK;
}
void checkTcl(Tcl_Interp* i, const char* script) {
  if (Tcl_Eval(i, script) != TCL_OK) {
    std::cerr << script << ": " << Tcl_GetStringResult(i) << '\n';
    std::abort();
  }
}
}
int main(int argc, char** argv) {
  Tcl_FindExecutable(argc ? argv[0] : "test_tk_lifecycle_pass10");
  const int initial = Tk_GetNumMainWindows();
  int countDeleted = 0;
  for (int cycle = 0; cycle < 12; ++cycle) {
    {
      Pass10Window first, second;
      check(first.start() && second.start(), "create two Tk interpreters");
      check(Tk_GetNumMainWindows() == initial + 2, "two Tk roots");
      Tcl_Interp* i = first.getInterpreter();
      Tcl_CallWhenDeleted(i, deleted, &countDeleted);
      Tcl_CallWhenDeleted(second.getInterpreter(), deleted, &countDeleted);
      checkTcl(i, "toplevel .aux; update idletasks");
      {
        FFTclSubWindow child(nullptr, i, nullptr, ".pass10.canvas", "child");
        check(Tcl_FindCommand(i, "SubRecall..pass10.canvas", nullptr, 0) != nullptr,
              "borrowed interpreter command registered");
        // A subwindow must not take ownership of its parent's Tcl interpreter.
        checkTcl(i, "destroy .aux; update idletasks");
      }
      check(Tcl_FindCommand(i, "SubRecall..pass10.canvas", nullptr, 0) == nullptr,
            "borrowed interpreter command deregistered");
      checkTcl(i, "destroy .");
    }
    FFWindow::collectPendingInterpreters();
    check(Tk_GetNumMainWindows() == initial, "no Tk roots left after each cycle");
  }
  FFWindow::collectPendingInterpreters();
  check(countDeleted == 24, "deleted both interpreters per cycle");
  check(Tk_GetNumMainWindows() == initial, "no Tk roots remain");

  // Close the window in an idle callback nested inside Tcl_Eval(). This
  // exercises the deferred Tcl_DeleteInterp boundary, with a second live root.
  for (int cycle = 0; cycle < 8; ++cycle) {
    auto* first = new Pass10Window;
    Pass10Window second;
    check(first->start() && second.start(), "callback fixture Tk roots");
    Tcl_Interp* active = first->getInterpreter();
    Tcl_CallWhenDeleted(active, deleted, &countDeleted);
    Tcl_CreateCommand(active, "close_owner", closeOwner, &first, nullptr);
    checkTcl(active, "after idle close_owner; update idletasks");
    check(first == nullptr, "window deleted inside Tcl callback");
    // The other interpreter/root must remain available and functional.
    checkTcl(second.getInterpreter(), "set pass10_ok 1; update idletasks");
    FFWindow::collectPendingInterpreters();
    check(Tk_GetNumMainWindows() == initial + 1, "second root survives cleanup");
  }
  FFWindow::collectPendingInterpreters();
  check(countDeleted == 32, "callback interpreter cleanup");
  check(Tk_GetNumMainWindows() == initial, "all roots reclaimed");
  Tcl_Finalize();
}
