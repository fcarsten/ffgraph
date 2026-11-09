#include "test_check.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFTcl3DWindow.h>
#include <ffgraph/FFTclGraphInsp.h>
#include <ffgraph/FFOrthoInsp.h>
#include <ffgraph/FFNaivInsp.h>
#include <ffgraph/FFTclCompat.h>
#include <ffgraph/ffError.h>
#include <string>

extern charErrorHandler* userCharErrorHandler;
namespace {
int errors = 0;
void unexpected(errGrade, const char* message, const char*, int) {++errors; std::cerr << message << '\n';}
void eval(Tcl_Interp* interp, const char* script) {
  const int status = Tcl_Eval(interp, script);
  if (status != TCL_OK) std::cerr << Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY) << '\n';
  FF_CHECK(status == TCL_OK);
}
void infoDialog(Tcl_Interp* interp) {
  eval(interp, R"tcl(
    set infoFonts [lsort [font names]]
    .frame1.menubutton2.m invoke Usage
    set t .topA.message3
    check {[wm title .topA] eq "Info" && [winfo class $t] eq "Text"} {Info is not a formatted text view}
    check {[$t cget -state] eq "disabled" && [$t cget -wrap] eq "word"} {Info is editable or does not wrap}
    check {[winfo class .topA.scroll] eq "TScrollbar"} {Info has no themed scrollbar}
    set content [$t get 1.0 end]
    foreach text [list {Carsten Friedrich} {Falk Schreiber} "Dirk Kosch\u00fctzki" "Barbara Z\u00fcndorf" {Bernhard Steffen} {Usage (historic text)}] {
      check {[string first $text $content] >= 0} "Missing Info content: $text"
    }
    check {[font actual [$t tag cget heading -font] -weight] eq "bold"} {Info headings are not emphasized}
    check {[llength [$t tag ranges heading]] == 8} {Info section headings are missing}
    $t insert 1.0 EDITED
    check {[$t get 1.0 end] eq $content} {Info text can be modified}
    $t tag add sel 1.0 1.7
    check {[$t get sel.first sel.last] eq "ffgraph"} {Info text cannot be selected for copying}
    $t tag remove sel 1.0 end
    wm geometry .topA 450x300
    update idletasks
    $t yview moveto 1
    check {[lindex [$t yview] 0] > 0} {Info does not scroll in a small window}
    rename exec infoOriginalExec
    rename auto_execok infoOriginalAutoExecok
    rename tk_messageBox infoOriginalMessageBox
    set infoLaunches {}
    set infoExecFailure 0
    proc exec {args} {
      if {$::infoExecFailure} {error {Test launcher unavailable}}
      lappend ::infoLaunches $args
      return 1234
    }
    proc auto_execok {name} {return [list {fake browser with spaces}]}
    proc tk_messageBox {args} {set ::infoLinkError $args; return ok}
    try {
      uplevel #0 [$t tag bind url1 <ButtonRelease-1>]
      check {[llength $infoLaunches] == 1} {Clicking the project link did not launch it}
      set launch [lindex $infoLaunches 0]
      check {[lindex $launch 0] eq {fake browser with spaces}} {URL launcher arguments lost quoting}
      check {[lindex $launch end-1] eq {https://github.com/fcarsten/ffgraph} && [lindex $launch end] eq "&"} {Project URL launch is incorrect or blocks the GUI}
      if {[tk windowingsystem] eq "win32"} {
        check {[lindex $launch 1] eq "url.dll,FileProtocolHandler"} {Windows URL handler is missing}
      }
      $t tag add sel {*}[$t tag ranges url1]
      uplevel #0 [$t tag bind url1 <ButtonRelease-1>]
      check {[llength $infoLaunches] == 1} {Selecting a link launched it instead of allowing copying}
      $t tag remove sel 1.0 end
      $t mark set insert 1.0
      ::ffgraph_info::focusLink $t 1
      check {[llength [$t tag ranges linkFocus]] == 2} {Keyboard link focus is invisible}
      ::ffgraph_info::activateLink $t
      check {[lindex [lindex $infoLaunches end] end-1] eq {https://www.gnu.org/licenses/gpl-3.0.html}} {Keyboard activation lost the license URL}
      if {[tk windowingsystem] eq "x11"} {
        set savedWsl {}
        foreach name {WSL_INTEROP WSL_DISTRO_NAME} {
          if {[info exists env($name)]} {dict set savedWsl $name $env($name)}
        }
        set infoExecutables {rundll32.exe gio}
        proc auto_execok {name} {
          if {$name in $::infoExecutables} {return [list $name]}
          return {}
        }
        try {
          set env(WSL_INTEROP) test-interop
          ::ffgraph_info::openLink .topA https://www.gnu.org/licenses/gpl-3.0.html
          set launch [lindex $infoLaunches end]
          check {[lrange $launch 0 1] eq {rundll32.exe url.dll,FileProtocolHandler}} {WSL sent the URL to gio instead of Windows}
          lappend infoExecutables wslview
          unset -nocomplain env(WSL_INTEROP)
          set env(WSL_DISTRO_NAME) test-distro
          check {[lindex [::ffgraph_info::linuxURLCommand https://example.invalid] 0] eq "wslview"} {WSL did not prefer the installed wslview launcher}
          unset env(WSL_DISTRO_NAME)
          lappend infoExecutables xdg-open
          check {[lindex [::ffgraph_info::linuxURLCommand https://example.invalid] 0] eq "xdg-open"} {Native Linux did not use its desktop browser launcher}
          set infoExecutables {rundll32.exe gio}
          check {[lrange [::ffgraph_info::linuxURLCommand https://example.invalid] 0 1] eq {gio open}} {Native Linux lost its gio fallback}
          set infoExecutables {}
          check {[catch {::ffgraph_info::linuxURLCommand https://example.invalid}]} {Missing Linux launchers did not report an error}
          set infoExecutables {rundll32.exe gio}
        } finally {
          foreach name {WSL_INTEROP WSL_DISTRO_NAME} {
            unset -nocomplain env($name)
            if {[dict exists $savedWsl $name]} {set env($name) [dict get $savedWsl $name]}
          }
        }
      }
      set infoExecFailure 1
      ::ffgraph_info::openLink .topA https://github.com/fcarsten/ffgraph
      check {[lindex $infoLinkError 1] eq ".topA" && [string first {Test launcher unavailable} $infoLinkError] >= 0} {Link launch failure did not give a parented error}
      set count [llength $infoLaunches]
      ::ffgraph_info::openLink .topA javascript:test
      check {[llength $infoLaunches] == $count && [string first {Unsupported link type} $infoLinkError] >= 0} {Info launched an unsupported URI scheme}
    } finally {
      foreach {command original} {exec infoOriginalExec auto_execok infoOriginalAutoExecok tk_messageBox infoOriginalMessageBox} {
        rename $command {}
        rename $original $command
      }
    }
    .topA.button4 invoke
    check {![winfo exists .topA]} {Info OK did not close the dialog}
    showUsageBox
    uplevel #0 [bind .topA <Escape>]
    check {![winfo exists .topA] && [lsort [font names]] eq $infoFonts} {Info reopen/close left a window or named fonts}
  )tcl");
}
void colorList(Tcl_Interp* interp, bool legacy) {
  Tcl_SetVar(interp, "colorListLegacy", legacy ? "1" : "0", TCL_GLOBAL_ONLY);
  eval(interp, R"tcl(
    set colorListFailure {}
    proc chooseListColor {} {
      if {![winfo exists .colorBox.colors.colors]} {after 10 chooseListColor; return}
      if {[catch {
        wm geometry .colorBox 600x500
        .colorBox.frame2.list invoke
        set list .colorBox.colors.colors
        # Use deterministic rows on Windows too, without an X11 rgb.txt file.
        $list delete 0 end
        $list insert end {red #ff0000} {ghost white #f8f8ff} {black #000000}
        foreach index {1 2 0} name {{ghost white} black red} rgb {{248 248 255} {0 0 0} {255 0 0}} {
          $list see $index
          update idletasks
          set box [$list bbox $index]
          check {[llength $box] == 4} {Colour-list row is not visible}
          set y [expr {[lindex $box 1]+[lindex $box 3]/2}]
          # Execute the actual click binding; propagate callback errors instead
          # of letting Tk send them to an asynchronous background-error popup.
          uplevel #0 [string map [list %W $list %y $y] [bind $list <ButtonPress-1>]]
          check {[$list curselection] eq "$index"} {Colour-list selection is stale}
          check {[.colorBox.current.current get] eq $name} {Colour name was not preserved}
          check {[.colorBox.demo cget -background] eq $name} {Colour preview was not updated}
          check {[list [.colorBox.red get] [.colorBox.green get] [.colorBox.blue get]] eq $rgb} {Colour-list RGB channels differ}
          set sample [.colorBox.palette.palette$::colorBox(paletteNr) cget -image]
          check {[$sample get 0 0] eq $rgb} {Colour list did not update the palette sample}
        }
        if {$::colorListLegacy} {
          .colorBox.frame1.ok invoke
        } else {
          uplevel #0 [string map [list %W $list %y $y] [bind $list <Double-1>]]
        }
      } message]} {
        set ::colorListFailure $::errorInfo
        set ::colorBox(colorName) {}
        if {$::colorListLegacy} {destroy .colorBox}
      }
    }
    set colorListWatchdog [after 10000 {puts stderr {colour-list regression timed out}; exit 1}]
    try {
      after 0 chooseListColor
      if {$colorListLegacy} {set result [ColorBox {} Color]} else {set result [ColorBox blue]}
      check {$colorListFailure eq ""} $colorListFailure
      check {$result eq "red"} {Colour-list acceptance returned the wrong colour}
      check {$backgroundErrors eq ""} $backgroundErrors
    } finally {
      after cancel $colorListWatchdog
      catch {destroy .colorBox}
    }
    check {[array size ::ffgraph_ui::colors] == 0} {Colour list left sample images behind}
  )tcl");
}
void exercise(bool threeD) {
  FFGraph graph(YES);
  auto* window = threeD ? static_cast<FFTclBasicWindow*>(new FFTcl3DWindow)
                        : static_cast<FFTclBasicWindow*>(new FFTclWindow);
  graph->createGraphic(window);
  eval(window->getInterpreter(), R"tcl(
    proc check {condition message} {if {![uplevel 1 [list expr $condition]]} {error $message}}
    set backgroundErrors {}
    set stage controls
    set watchdog [after 10000 {puts stderr "ttk regression timed out: $stage"; exit 1}]
    proc bgerror {message} {lappend ::backgroundErrors $message}
    proc themed {root} {
      foreach child [winfo children $root] {
        check {[winfo class $child] ni {Button Checkbutton Radiobutton Scrollbar Entry Label Menubutton Scale Frame}} "Classic control remains: $child ([winfo class $child])"
        themed $child
      }
    }
    themed .
    check {[winfo class .frame1.menubutton1] eq "TMenubutton"} {File menu is not themed}
    check {[winfo class .frame2.frame8.scale16.slider] eq "TScale"} {Rotation slider is not themed}
    # Integer rounding, callback scripts and a working variable/command prefix.
    set received {}
    ::ffgraph_ui::scale .testScale -from -10 -to 10 -label Value -command {set marker 1; lappend received}
    pack .testScale
    update
    .testScale.slider set 2.6
    update idletasks
    check {$received eq "3" && [.testScale get] == 3} {Slider callback lost integer semantics}
    .testScale set 100
    update idletasks
    check {[.testScale get] == 10} {Slider did not clamp its range}
    check {$received eq {3 10}} {Programmatic slider update lost its callback}
    .testScale configure -state disabled
    .testScale set 5
    check {[.testScale get] == 10} {Disabled slider changed its value}
    check {[.testScale.slider instate disabled]} {Disabled state did not reach slider}
    destroy .testScale
    check {[info commands .testScale] eq ""} {Destroyed slider left a command alias}
    # Color samples must survive native button themes that ignore backgrounds.
    ::ffgraph_ui::swatch button .sample -command {set chosen 1}
    .sample configure -background #123456
    set sampleImage [.sample cget -image]
    check {[$sampleImage get 0 0] eq {18 52 86}} {Color swatch pixels differ}
    .sample invoke
    check {$chosen == 1} {Color swatch command failed}
    destroy .sample
    check {$sampleImage ni [image names]} {Destroyed swatch leaked its image}
    # Exercise auxiliary windows with real callbacks, not just construction.
    if {![llength [info commands ShowPovWindow]]} {source [file join $FF_LIBDIR tcl povexp.tcl]}
    ShowPovWindow
    themed .pov
    destroy .pov
    check {$backgroundErrors eq ""} $backgroundErrors
    proc answerInput {} {
      if {![winfo exists .inputOneBox.entry1] || ![winfo viewable .inputOneBox.entry1]} {after 10 answerInput; return}
      .inputOneBox.entry1 insert 0 {themed input}
      focus -force .inputOneBox.entry1
      update
      event generate .inputOneBox.entry1 <Return> -when tail
    }
    set stage input-dialog
    after 0 answerInput
    check {[InputOneBox] eq {themed input}} {Themed input box lost its value}
    AlertBox {Themed alert}
    themed .alertBox
    .alertBox.frame1.button0 invoke
    after cancel $watchdog
  )tcl");
  infoDialog(window->getInterpreter());
  {
    FFTclGraphInsp inspector(window);
    auto* interp = inspector.getInterpreter();
    eval(interp, R"tcl(
      proc check {condition message} {if {![uplevel 1 [list expr $condition]]} {error $message}}
      set backgroundErrors {}
      set stage color-dialog
      set watchdog [after 10000 {puts stderr "ttk regression timed out: $stage"; exit 1}]
      proc bgerror {message} {lappend ::backgroundErrors $message}
      check {[winfo class .insp_graph.number.nodes.entry] eq "TEntry"} {Inspector entry is not themed}
      check {[winfo class .insp_graph.type.graph.value] eq "TLabel"} {Inspector value is not themed}
      check {[OptionButtonGet .insp_graph.type.graph] ne ""} {Themed inspector value cannot be read}
      set colorTestFailure {}
      proc chooseColor {} {
        if {![winfo exists .colorBox.palette.palette4]} {after 10 chooseColor; return}
        if {[catch {
          .colorBox.palette.palette4 invoke
          check {[.colorBox.demo cget -background] eq "red"} {Palette did not update preview}
          .colorBox.frame1.ok invoke
        } message]} {set ::colorTestFailure $message; set ::colorBox(colorName) {}}
      }
      after 0 chooseColor
      set result [ColorBox blue]
      check {$colorTestFailure eq ""} $colorTestFailure
      check {$result eq "red"} {Color chooser returned the wrong color}
      check {$backgroundErrors eq ""} $backgroundErrors
      destroy .colorBox
      check {[array size ::ffgraph_ui::colors] == 0} {Color chooser left image owners behind}
      after cancel $watchdog
    )tcl");
    colorList(interp, false);
  }
  colorList(window->getInterpreter(), true);
  graph->removeGraphic();
}
void otherLayouts() {
  FFGraph graph(YES);
  auto* ortho = new FFOrtho;
  auto* naive = new FFNaiv;
  graph->addGraphLabel(ortho); graph->addGraphLabel(naive);
  {
    FFOrthoInsp inspector(ortho);
    inspector.inspect();
    eval(inspector.getInterpreter(), ".checkBox.3d invoke");
    const bool selected = ortho->get3D();
    inspector.inspect();
    eval(inspector.getInterpreter(), ".checkBox.3d invoke");
    FF_CHECK(bool(ortho->get3D()) != selected);
  }
  {
    FFNaivInsp inspector(naive);
    inspector.inspect();
    eval(inspector.getInterpreter(), "if {[winfo class .positionitems.xoffset.e] ne {TEntry}} {error {Naive inspector is not themed}}");
  }
}
}
int main(int argc, char** argv) {
  Tcl_FindExecutable(argc ? argv[0] : "test_ttk_widgets");
  userCharErrorHandler = unexpected;
  exercise(false); FFWindow::collectPendingInterpreters();
  exercise(true); FFWindow::collectPendingInterpreters();
  otherLayouts(); FFWindow::collectPendingInterpreters();
  FF_CHECK(errors == 0);
  std::cout << "Themed controls, integer sliders, colors and callbacks verified in 2D/3D\n";
}
