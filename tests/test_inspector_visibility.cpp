// Display-backed test. Run using xvfb-run (or a real X display).
#include <ffgraph/FFTclInspector.h>
#include <tk.h>
#include <iostream>
#include <string>

class TestInspector : public FFTclInspector {
public:
  TestInspector() : FFTclInspector(nullptr) {}
  void inspect() override {}
  bool initialiseTk() {
    Tcl_Interp* i = getInterpreter();
    if (Tcl_Init(i) != TCL_OK || Tk_Init(i) != TCL_OK)
      return false;
    w = Tk_MainWindow(i);
    return w != nullptr;
  }
};

static bool evaluate(Tcl_Interp* i, const char* command, const char* expected) {
  if (Tcl_Eval(i, command) != TCL_OK) {
    std::cerr << command << ": " << Tcl_GetStringResult(i) << '\n';
    return false;
  }
  const std::string result = Tcl_GetStringResult(i);
  if (result != expected) {
    std::cerr << command << ": got '" << result << "', expected '" << expected << "'\n";
    return false;
  }
  return true;
}

int main() {
  auto* inspector = new TestInspector;
  Tcl_Interp* interp = inspector->getInterpreter();
  if (!inspector->initialiseTk()) {
    std::cerr << "Could not initialise Tcl/Tk: " << Tcl_GetStringResult(interp) << '\n';
    delete inspector;
    Tcl_DeleteInterp(interp);
    return 1;
  }

  bool ok = inspector->hasLiveWindow();
  ok = evaluate(interp, "wm withdraw .", "") && ok;
  ok = evaluate(interp, "wm state .", "withdrawn") && ok;
  inspector->show();
  ok = evaluate(interp, "wm state .", "normal") && ok;
  ok = inspector->hasLiveWindow() && ok;
  // The interpreter remains valid after the Tk root is destroyed; show()
  // must not treat a destroyed window as a restorable withdrawn window.
  ok = evaluate(interp, "destroy .", "") && ok;
  ok = !inspector->hasLiveWindow() && ok;

  delete inspector;
  Tcl_DeleteInterp(interp);
  return ok ? 0 : 1;
}
