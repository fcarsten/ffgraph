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
