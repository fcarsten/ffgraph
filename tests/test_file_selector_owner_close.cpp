#include "test_check.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFTcl3DWindow.h>
#include <ffgraph/FFTclCompat.h>
#include <ffgraph/ffError.h>
#include <string>

extern charErrorHandler* userCharErrorHandler;
namespace {
int errors = 0;
void unexpected(errGrade, const char* message, const char*, int) {
  ++errors; std::cerr << message << '\n';
}
template<class Base> struct Tracked : Base {
  bool& alive;
  explicit Tracked(bool& flag) : alive(flag) { alive = true; }
  ~Tracked() override { alive = false; }
};
void eval(Tcl_Interp* interp, const char* script) {
  const int code = Tcl_Eval(interp, script);
  if (code != TCL_OK) std::cerr << Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY) << '\n';
  FF_CHECK(code == TCL_OK);
}
int checkpoint(ClientData data, Tcl_Interp* interp, int, const char**) {
  const char* error = Tcl_GetVar(interp, "ownerCloseError", TCL_GLOBAL_ONLY);
  if (error && *error) std::cerr << error << '\n';
  // Stop here before the old code returns into its deleted C++ fsBox caller.
  FF_CHECK(*static_cast<bool*>(data));
  FF_CHECK(!error || !*error);
  return TCL_OK;
}
template<class Base> void exercise() {
  const int initial = Tk_GetNumMainWindows();
  for (const char* phase : {"wait", "prepare", "confirm"}) {
    for (const char* label : {"Load (M-l)", "Save As (M-S)"}) {
      if (std::string(phase) == "confirm" && std::string(label) != "Save As (M-S)") continue;
      FFGraph other(YES), graph(YES);
      if (std::string(phase) == "wait") other->createGraphic(new Base);
      const int others = Tk_GetNumMainWindows();
      bool alive = false;
      auto* window = new Tracked<Base>(alive);
      graph->createGraphic(window);
      auto* interp = window->getInterpreter();
      Tcl_CreateCommand(interp, "ownerCheckpoint", checkpoint, &alive, nullptr);
      Tcl_SetVar(interp, "ownerClosePhase", phase, TCL_GLOBAL_ONLY);
      Tcl_SetVar(interp, "ownerCloseLabel", label, TCL_GLOBAL_ONLY);
      eval(interp, R"tcl(
        set ownerCloseError {}
        set ownerCloseDone 0
        set ownerMissing [file join [pwd] ffgraph-missing-[pid]-[clock clicks] test.g]
        proc bgerror {message} {set ::ownerCloseError $::errorInfo}
        proc closeOwner {} {
          if {$::ownerCloseDone} {return}
          set ::ownerCloseDone 1
          # Simulate repeated WM requests while a nested dialog is waiting.
          catch {
            uplevel #0 [wm protocol . WM_DELETE_WINDOW]
            uplevel #0 [wm protocol . WM_DELETE_WINDOW]
            # File > Close and the keyboard shortcut share this C++ entry.
            recall modPress w
          } ::ownerCloseError
          ownerCheckpoint
          update idletasks
          ownerCheckpoint
          if {[info commands recall] eq ""} {error {Close removed recall before the file operation returned}}
        }
        proc ownerScan {args} {
          trace remove execution FileSelectorBox_GetDirStruct enter ownerScan
          closeOwner
        }
        proc ownerRespond {} {
          if {![info exists ::fileselect(state)] || $::fileselect(state) ne "wait"} {
            after 10 ownerRespond
            return
          }
          FileSelectorBox_FillEntry .fileselect.choose.current.e $::ownerMissing
          if {$::ownerClosePhase eq "confirm"} {
            set ::fileselect(state) ok
            after 0 ownerConfirm
          } else {
            closeOwner
          }
        }
        proc ownerConfirm {} {
          if {![winfo exists .htmessagebox]} {after 10 ownerConfirm; return}
          closeOwner
        }
        after 10000 {set ownerCloseError {Owner-close regression timed out}; ownerCheckpoint; exit 99}
        if {$ownerClosePhase eq "prepare"} {
          trace add execution FileSelectorBox_GetDirStruct enter ownerScan
        } else {
          after 0 ownerRespond
        }
        .frame1.menubutton1.m invoke $ownerCloseLabel
        if {!$ownerCloseDone} {error {Owner Close callback was not exercised}}
        if {$ownerCloseError ne ""} {error $ownerCloseError}
        if {[file exists [file dirname $ownerMissing]]} {error {Cancellation created a directory}}
      )tcl");
      FF_CHECK(alive); // Destruction must wait for the C++ file callback.
      eval(interp, "update idletasks; closeGraphWindow; closeGraphWindow");
      FF_CHECK(!alive && graph->getWindow() == nullptr);
      FFWindow::collectPendingInterpreters();
      FF_CHECK(Tk_GetNumMainWindows() == others);
      if (other->getWindow()) {
        eval(static_cast<Base*>(other->getWindow())->getInterpreter(), "update idletasks");
        other->removeGraphic();
        FFWindow::collectPendingInterpreters();
      }
      FF_CHECK(Tk_GetNumMainWindows() == initial);
    }
  }
}
}
int main(int argc, char** argv) {
  Tcl_FindExecutable(argc ? argv[0] : "test_file_selector_owner_close");
  userCharErrorHandler = unexpected;
  exercise<FFTclWindow>();
  exercise<FFTcl3DWindow>();
  FF_CHECK(errors == 0);
  std::cout << "Owner Close cancels and unwinds file dialogs before destroying 2D/3D graphs\n";
}
