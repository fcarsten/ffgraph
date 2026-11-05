#include "test_check.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFTcl3DWindow.h>

namespace {
void exercise(bool threeD) {
  FFGraph graph(YES);
  auto* window = threeD ? static_cast<FFTclBasicWindow*>(new FFTcl3DWindow)
                        : static_cast<FFTclBasicWindow*>(new FFTclWindow);
  graph->createGraphic(window);
  const int status = Tcl_Eval(window->getInterpreter(), R"tcl(
    proc selectorCheck {condition message} {
      if {![uplevel 1 [list expr $condition]]} {error $message}
    }
    # Reject any construction-time post before Windows enters its native menu
    # loop. This catches even a popup that is immediately hidden again on X11.
    set selectorAllowPost 0
    proc selectorMenuTrace {command operation} {
      if {[lindex $command 1] ne "post"} {return}
      if {!$::selectorAllowPost} {error {Navigation menu posted during dialog construction}}
      set ::selectorPost $command
      # Windows uses a native modal menu; Linux exercises actual Tk events.
      if {!$::selectorRealMenu} {error SELECTOR_TEST_POST}
    }
    proc selectorCreatedMenu {command code result operation} {
      if {$code == 0 && $result eq ".fileselect.menu_begin_dir"} {
        trace add execution $result enter selectorMenuTrace
      }
    }
    trace add execution menu leave selectorCreatedMenu
    set selectorPreparing 0
    set selectorMapStates {}
    set selectorOpening 0
    proc selectorMapEvent {widget} {
      if {$widget ne ".fileselect"} {return}
      set state unset
      if {[info exists ::fileselect(state)]} {set state $::fileselect(state)}
      lappend ::selectorMapStates $state
    }
    bind all <Map> {+selectorMapEvent %W}
    proc selectorScanTrace {command operation} {
      if {$::selectorPreparing && [winfo ismapped .fileselect]} {
        error {File selector became visible before directory initialization finished}
      }
    }
    trace add execution FileSelectorBox_GetDirStruct enter selectorScanTrace
    set channel [file tempfile selectorFile]
    close $channel
    set selectorFailure ""
    set selectorRealMenu 0
    proc bgerror {message} {set ::selectorFailure $message; puts stderr $::errorInfo}
    proc selectorNavigate {} {
      set ::selectorAllowPost 1
      set ::selectorRealMenu 1
      set menu .fileselect.menu_begin_dir
      set button .fileselect.misc.left.begin_dir
      # Give the legacy layout enough width to display its arrow subwidget.
      wm geometry .fileselect 600x500
      update
      # Exercise the frame, label and arrow, not a direct menu-command invoke.
      foreach suffix {{} .l .knopf} choice {Home Root Current} expected [list ~ / $::fileselect(start_dir)] {
        set control $button$suffix
        focus -force .fileselect.choose.current.e
        grab set .fileselect
        event generate $control <Motion> -warp 1 -x 3 -y 3
        update
        selectorCheck {[winfo viewable $control]} {Navigation control is not visible}
        event generate $control <ButtonPress-1> -x 5 -y 5
        event generate $control <ButtonRelease-1> -x 5 -y 5
        update
        selectorCheck {[winfo ismapped $menu]} {Releasing the navigation button hid the menu}
        selectorCheck {[grab current] eq $menu && [grab status $menu] eq "global"} {Popup does not own the pointer grab}
        set entry [lsearch -exact [FileSelectorBox_GetMenuEntries $menu] $choice]
        set y [expr {[$menu yposition $entry] + 5}]
        event generate $menu <Motion> -warp 1 -x 10 -y $y
        update
        selectorCheck {[$menu index active] eq "$entry"} {Pointer did not activate the menu entry}
        event generate $menu <ButtonPress-1> -x 10 -y $y
        event generate $menu <ButtonRelease-1> -x 10 -y $y
        update
        selectorCheck {$::fileselect(currentmenu) eq $choice} {Menu click did not invoke its directory command}
        selectorCheck {[file normalize $::fileselect(cur_dir)] eq [file normalize $expected]} {Navigation chose the wrong directory}
        selectorCheck {![winfo ismapped $menu]} {Menu stayed open after selection}
        selectorCheck {[grab current] eq ".fileselect"} {Popup did not restore the previous grab}
        selectorCheck {[focus] eq ".fileselect.choose.current.e"} {Popup did not restore focus}
        grab release .fileselect
      }
      # Dismissing a popup must also restore the existing local grab.
      foreach dismissal {escape outside} {
        grab set .fileselect
        event generate $button <Motion> -warp 1 -x 3 -y 3
        update
        event generate $button <ButtonPress-1> -x 5 -y 5
        event generate $button <ButtonRelease-1> -x 5 -y 5
        update
        if {$dismissal eq "escape"} {
          event generate $menu <Escape>
        } else {
          event generate $menu <Motion> -warp 1 -x -20 -y -20
          update
          event generate $menu <ButtonPress-1> -x -20 -y -20
          event generate $menu <ButtonRelease-1> -x -20 -y -20
        }
        update
        selectorCheck {![winfo ismapped $menu]} {Dismissal left the popup visible}
        selectorCheck {[grab current] eq ".fileselect"} {Dismissal did not restore the previous grab}
        grab release .fileselect
      }
      set ::selectorRealMenu 0
      set ::selectorAllowPost 0
    }
    proc selectorRespond {accept} {
      if {![info exists ::fileselect(state)] || $::fileselect(state) ne "wait"} {
        set ::selectorResponse [after 10 [list selectorRespond $accept]]
        return
      }
      if {[catch {
        update
        set ::selectorPreparing 0
        selectorCheck {[winfo ismapped .fileselect]} {Ready file selector is hidden}
        selectorCheck {[llength $::selectorMapStates] == $::selectorOpening} {File selector was mapped more than once per opening}
        foreach state $::selectorMapStates {
          selectorCheck {$state eq "wait"} {File selector was mapped before it was ready}
        }
        selectorCheck {[winfo exists .fileselect.action.ok]} {Incomplete file selector}
        selectorCheck {![winfo ismapped .fileselect.menu_begin_dir]} {Navigation menu is visible at startup}
        selectorCheck {[FileSelectorBox_GetMenuEntries .fileselect.menu_begin_dir] eq {Home Root Current}} {Missing navigation entries}
        if {!$accept && [tk windowingsystem] eq "x11"} {selectorNavigate}
        if {$accept} {
          FileSelectorBox_FillEntry .fileselect.choose.current.e $::selectorFile
          .fileselect.action.ok invoke
        } else {
          .fileselect.action.dismiss invoke
        }
      } message]} {
        set ::selectorFailure $message
        set ::fileselect(state) dismiss
      }
    }
    set selectorWatchdog [after 5000 {
      set selectorFailure {File selector did not reach its response loop}
      set fileselect(state) dismiss
    }]
    try {
      # A first opening must reach the complete dialog without a menu popup.
      set selectorPreparing 1
      incr selectorOpening
      set selectorResponse [after 0 {selectorRespond 0}]
      set selected [FileSelectorBox {Open graph} * [file dirname $selectorFile]]
      selectorCheck {$selectorFailure eq ""} $selectorFailure
      selectorCheck {$selected eq ""} {Cancel returned a filename}
      selectorCheck {[wm state .fileselect] eq "withdrawn"} {Cancel left the dialog visible}

      # The original button binding must still request the navigation popup.
      set selectorAllowPost 1
      set binding [bind .fileselect.misc.left.begin_dir <ButtonRelease-1>]
      set outcome [catch {eval [string map {%X 400 %Y 300 %y 10} $binding]} message]
      selectorCheck {$outcome == 1 && $message eq "SELECTOR_TEST_POST"} {Navigation binding failed to post its menu}
      selectorCheck {[lrange $selectorPost 0 1] eq {.fileselect.menu_begin_dir post}} {Wrong navigation menu posted}
      set selectorAllowPost 0

      # Reopening the existing selector must also work, including acceptance.
      set selectorPreparing 1
      incr selectorOpening
      set selectorResponse [after 0 {selectorRespond 1}]
      set selected [FileSelectorBox {Open graph again} * [file dirname $selectorFile]]
      selectorCheck {$selectorFailure eq ""} $selectorFailure
      selectorCheck {[file normalize $selected] eq [file normalize $selectorFile]} {Wrong selected filename}
      selectorCheck {[wm state .fileselect] eq "withdrawn"} {Accept left the dialog visible}
    } finally {
      after cancel $selectorWatchdog
      after cancel $selectorResponse
      trace remove execution menu leave selectorCreatedMenu
      trace remove execution FileSelectorBox_GetDirStruct enter selectorScanTrace
      file delete $selectorFile
      catch {destroy .fileselect}
    }
  )tcl");
  if (status != TCL_OK) std::cerr << Tcl_GetStringResult(window->getInterpreter()) << '\n';
  FF_CHECK(status == TCL_OK);
  graph->removeGraphic();
}
}

int main(int argc, char** argv) {
  Tcl_FindExecutable(argc ? argv[0] : "test_file_selector");
  exercise(false); FFWindow::collectPendingInterpreters();
  exercise(true); FFWindow::collectPendingInterpreters();
  std::cout << "File selector startup, navigation binding, cancel and accept verified in 2D/3D\n";
}
