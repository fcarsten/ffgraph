// Dependency-only reproducer for the Ubuntu Tk 8.6.14/Xft shutdown leak.
// This does not link ffgraph. See FFGRAPH-PASS12-FF-BASELINE-FIXES.md for
// standalone build/run commands; the existing GUI CTests retain leak checking.
#include <tcl.h>
#include <tk.h>
#include <iostream>

int main(int, char** argv) {
  Tcl_FindExecutable(argv[0]);
  Tcl_Interp* interp = Tcl_CreateInterp();
  if (Tcl_Init(interp) != TCL_OK || Tk_Init(interp) != TCL_OK) {
    std::cerr << Tcl_GetStringResult(interp) << '\n';
    Tcl_DeleteInterp(interp);
    Tcl_Finalize();
    return 1;
  }
  const int result = Tcl_Eval(interp,
    "menu .m; .m add command -label test; "
    "menubutton .b -text file -menu .m; pack .b; update idletasks; "
    "destroy .; update idletasks");
  if (result != TCL_OK) std::cerr << Tcl_GetStringResult(interp) << '\n';
  int deletions = 0;
  Tcl_CallWhenDeleted(interp, [](ClientData data, Tcl_Interp*) {
    ++*static_cast<int*>(data);
  }, &deletions);
  Tcl_DeleteInterp(interp);
  Tcl_Finalize();
  std::cout << "Tk menu destroyed; interpreter deletions: " << deletions << std::endl;
  if (result != TCL_OK || deletions != 1) return 2;
  return 0;
}
