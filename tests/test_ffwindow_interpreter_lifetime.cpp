// A standalone FFWindow constructs an interpreter before any Tk window exists.
// Destruction must release that interpreter, even when initFromFile wasn't run.
#include "test_check.h"
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
    FF_CHECK(interp != nullptr);
    FF_CHECK(Tcl_Eval(interp, "set ffgraph_lifetime_test 42") == TCL_OK);
    Tcl_CallWhenDeleted(interp, interpreterDeleted, &deletions);
  }
  FF_CHECK(deletions == 1);
}
