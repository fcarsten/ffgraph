// A standalone FFWindow constructs an interpreter before any Tk window exists.
// Destruction must release that interpreter, even when initFromFile wasn't run.
#include <cassert>
#include <ffgraph/FFWindow.h>

namespace {
void interpreterDeleted(ClientData data, Tcl_Interp*) {
  ++*static_cast<int*>(data);
}
}

int main() {
  int deletions = 0;
  {
    FFWindow window;
    Tcl_Interp* interp = window.getInterpreter();
    assert(interp != nullptr);
    assert(Tcl_Eval(interp, "set ffgraph_lifetime_test 42") == TCL_OK);
    Tcl_CallWhenDeleted(interp, interpreterDeleted, &deletions);
  }
  assert(deletions == 1);
}
