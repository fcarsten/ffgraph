// Real Tk lifecycle test; run with xvfb-run to avoid dependence on a desktop.
#include <ffgraph/FFWindow.h>
#include <tcl.h>
#include <tk.h>
#include <iostream>
#include <cstdlib>
#include <string>

class TestWindow final : public FFWindow {
public:
  bool start() {
    initFromFile("");
    return getInterpreter() && Tk_MainWindow(getInterpreter()) != nullptr;
  }
};

static void interpDeleted(ClientData opaque, Tcl_Interp *) {
  ++*static_cast<int *>(opaque);
}

static int closeFromCallback(ClientData opaque, Tcl_Interp*, int, const char**) {
  auto **window = static_cast<TestWindow **>(opaque);
  delete *window;
  *window = nullptr;
  return TCL_OK;
}

static bool evaluate(Tcl_Interp* i, const char* script) {
  if (Tcl_Eval(i, script) == TCL_OK) return true;
  std::cerr << script << ": " << Tcl_GetStringResult(i) << '\n';
  return false;
}

int main(int argc, char** argv) {
  Tcl_FindExecutable(argc > 0 ? argv[0] : "test_ffwindow_tk_teardown");
  int removed = 0;
  const int initial = Tk_GetNumMainWindows();
  {
    TestWindow window;
    if (!window.start()) return 1;
    Tcl_Interp *i = window.getInterpreter();
    Tcl_CallWhenDeleted(i, interpDeleted, &removed);
    if (Tk_GetNumMainWindows() != initial + 1) return 2;
    if (!evaluate(i, "toplevel .extra; update idletasks")) return 3;
  }
  FFWindow::collectPendingInterpreters();
  if (removed != 1) { std::cerr << "Tcl deletion callback missed: " << removed << '\n'; return 4; }
  if (Tk_GetNumMainWindows() != initial) {
    std::cerr << "Tk root leaked: " << Tk_GetNumMainWindows() << '\n'; return 5;
  }
  {
    TestWindow window;
    if (!window.start()) return 6;
    Tcl_Interp *i = window.getInterpreter();
    Tcl_CallWhenDeleted(i, interpDeleted, &removed);
    if (!evaluate(i, "destroy .")) return 7;
  }
  FFWindow::collectPendingInterpreters();
  if (removed != 2 || Tk_GetNumMainWindows() != initial) return 8;
  {
    TestWindow first, second;
    if (!first.start() || !second.start()) return 9;
    Tcl_Interp *other = second.getInterpreter();
    if (Tk_GetNumMainWindows() != initial + 2) return 10;
    Tcl_CallWhenDeleted(first.getInterpreter(), interpDeleted, &removed);
    Tcl_CallWhenDeleted(other, interpDeleted, &removed);
  }
  FFWindow::collectPendingInterpreters();
  if (removed != 4 || Tk_GetNumMainWindows() != initial) return 11;

  // Repeated allocation/deletion must not accumulate Tk main windows.
  for (int cycle = 0; cycle < 20; ++cycle) {
    TestWindow window;
    if (!window.start()) return 12;
    Tcl_CallWhenDeleted(window.getInterpreter(), interpDeleted, &removed);
  }
  FFWindow::collectPendingInterpreters();
  if (removed != 24 || Tk_GetNumMainWindows() != initial) return 13;

  // The real application can remove its graphical label from a Tcl
  // command callback, deleting the C++ window while Tcl is evaluating it.
  // Tcl must be allowed to finish the active command without accessing a
  // prematurely freed interpreter.
  {
    auto *window = new TestWindow;
    if (!window->start()) return 14;
    Tcl_Interp *i = window->getInterpreter();
    Tcl_CallWhenDeleted(i, interpDeleted, &removed);
    Tcl_CreateCommand(i, "closeFromCallback", closeFromCallback, &window, nullptr);
    // Exercise re-entrant Tcl update; interpreter deletion MUST wait until
    // this outer Tcl_Eval has returned, even if nested idle events run.
    int result = Tcl_Eval(i, "closeFromCallback; update idletasks");
    if (result != TCL_OK || window != nullptr)
      return 15;
  }
  FFWindow::collectPendingInterpreters();
  if (removed != 25 || Tk_GetNumMainWindows() != initial) return 16;

  // Test the real application event-loop wrapper, including self-destruction
  // from an event callback and cleanup after Tcl_DoOneEvent returns.
  {
    auto *window = new TestWindow;
    if (!window->start()) return 17;
    Tcl_CallWhenDeleted(window->getInterpreter(), interpDeleted, &removed);
    Tcl_Interp *i = window->getInterpreter();
    Tcl_CreateCommand(i, "closeFromCallback", closeFromCallback, &window, nullptr);
    struct TimerData { Tcl_Interp* interp; int result; } timer{i, TCL_ERROR};
    Tcl_CreateTimerHandler(1, [](ClientData data) {
      auto *timer = static_cast<TimerData *>(data);
      timer->result = Tcl_Eval(timer->interp,
                               "closeFromCallback; update idletasks");
    }, &timer);
    FFWindow::mainLoop();
    if (window != nullptr || removed != 26 || timer.result != TCL_OK)
      return 18;
  }
  if (Tk_GetNumMainWindows() != initial) return 19;

  Tcl_Finalize();
  return 0;
}
