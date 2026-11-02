// Exercising shared-interpreter callback deregistration requires no X display.
#include "test_check.h"
#include <tcl.h>
#include <ffgraph/FFTclSubWindow.h>

int main() {
  Tcl_FindExecutable("test_tcl_command_lifetime");
  Tcl_Interp* parent = Tcl_CreateInterp();
  FF_CHECK(parent != nullptr);
  FF_CHECK(Tcl_FindCommand(parent, "proj", nullptr, 0) == nullptr);

  {
    auto* child = new FFTclSubWindow(nullptr, parent, nullptr, ".test.canvas", "node");
    FF_CHECK(Tcl_FindCommand(parent, "SubRecall..test.canvas", nullptr, 0) != nullptr);
    // Tcl may delete a command before its C++ owner is destroyed.
    FF_CHECK(Tcl_DeleteCommand(parent, "SubRecall..test.canvas") == TCL_OK);
    delete child;
    FF_CHECK(Tcl_FindCommand(parent, "SubRecall..test.canvas", nullptr, 0) == nullptr);
    FF_CHECK(!Tcl_InterpDeleted(parent));
  }
  {
    auto* child = new FFTclSubWindow(nullptr, parent, nullptr, ".next.canvas", "node");
    FF_CHECK(Tcl_FindCommand(parent, "SubRecall..next.canvas", nullptr, 0) != nullptr);
    delete child;
    FF_CHECK(Tcl_FindCommand(parent, "SubRecall..next.canvas", nullptr, 0) == nullptr);
  }

  // A subwindow may outlive interpreter deletion. Tcl_Preserve prevents its
  // Tcl_Interp pointer from becoming dangling before child destruction.
  {
    auto* child = new FFTclSubWindow(nullptr, parent, nullptr, ".other.canvas", "node");
    Tcl_DeleteInterp(parent);
    delete child;
  }
  Tcl_Finalize();
}
