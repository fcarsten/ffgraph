// Exercising shared-interpreter callback deregistration requires no X display.
#include <cassert>
#include <tcl.h>
#include <ffgraph/FFTclSubWindow.h>

int main() {
  Tcl_FindExecutable("test_tcl_command_lifetime");
  Tcl_Interp* parent = Tcl_CreateInterp();
  assert(parent != nullptr);
  assert(Tcl_FindCommand(parent, "proj", nullptr, 0) == nullptr);

  {
    auto* child = new FFTclSubWindow(nullptr, parent, nullptr, ".test.canvas", "node");
    assert(Tcl_FindCommand(parent, "SubRecall..test.canvas", nullptr, 0) != nullptr);
    // Tcl may delete a command before its C++ owner is destroyed.
    assert(Tcl_DeleteCommand(parent, "SubRecall..test.canvas") == TCL_OK);
    delete child;
    assert(Tcl_FindCommand(parent, "SubRecall..test.canvas", nullptr, 0) == nullptr);
    assert(!Tcl_InterpDeleted(parent));
  }
  {
    auto* child = new FFTclSubWindow(nullptr, parent, nullptr, ".next.canvas", "node");
    assert(Tcl_FindCommand(parent, "SubRecall..next.canvas", nullptr, 0) != nullptr);
    delete child;
    assert(Tcl_FindCommand(parent, "SubRecall..next.canvas", nullptr, 0) == nullptr);
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
