#!/usr/bin/env wish
# Program: ortho
# Tcl version: 7.3 (Tcl/Tk/XF)
# Tk version: 3.6
# XF version: 2.3
#


proc Ortho_SetupWindow {w} {::ffgraph_ui::surface $w}


global FF_LIBDIR
source $FF_LIBDIR/tcl/xfmisc.tcl

proc Ortho_MakePositionItems {} {
  proc Ortho_MakeMyEntry {name text recall_text} {
    ttk::frame $name
    ttk::label $name.l -text $text -anchor w
    ttk::entry $name.e -width 8
    $name.e insert 1 "100"
    bind $name.e <KeyPress> {
      if {("[expr %N -48]" != "%A") && ("%A" != "\{\}") && \
            ("%K" != "BackSpace") && ("%K" != "Delete")} {
	break
      }
    }
    bind $name.e <KeyRelease> "recall $recall_text \[%W get\]"
    pack append $name \
      $name.l {left frame w expand fillx} \
      $name.e {right frame w}
  }
  ttk::frame .positionitems
  ttk::label .positionitems.l -text "Positioning:" -anchor nw
  Ortho_MakeMyEntry .positionitems.xoffset "X Distance" yoff
  Ortho_MakeMyEntry .positionitems.yoffset "Y Distance" xoff
  Ortho_MakeMyEntry .positionitems.zoffset "Z Distance (3D)" zoff
  pack append .positionitems \
    .positionitems.l {top frame nw expand fillx} \
    .positionitems.xoffset {top frame nw expand fillx} \
    .positionitems.yoffset {top frame nw expand fillx} \
    .positionitems.zoffset {top frame nw expand fillx} 
}

proc Ortho_MakeActionButtons {} {
  ttk::frame .actionbuttons
  ttk::button .actionbuttons.ok -text "Just do it"  -command {recall startLayout}
  ttk::button .actionbuttons.dismiss -text Dismiss  -command {recall deleteWindow}
  pack append .actionbuttons \
    .actionbuttons.ok {left} \
    .actionbuttons.dismiss {right}
}

proc Ortho_MakeCheckBox {} {
  ttk::frame .checkBox
  ttk::checkbutton .checkBox.3d -text "3D Layout"  -command \
    {recall toggle3D} -variable ortho3D

  ttk::checkbutton .checkBox.anim -text "Animated"  -command \
    {recall toggleAnim} -variable orthoAnimate

  pack append .checkBox \
    .checkBox.3d {top expand fillx} \
    .checkBox.anim {bottom expand fillx} 
}

proc Ortho_MakeStepBox {} {
  global initPlacement

  ttk::frame .stepBox
  ttk::frame .stepBox.initBox
  
  ttk::label .stepBox.initBox.text -text {Initial Placement}

  ttk::radiobutton .stepBox.initBox.random -text "Random"  -command \
    {recall setRandom} -variable initPlacement -value "random"

  ttk::radiobutton .stepBox.initBox.adjust -text "Adjust"  -command \
    {recall setAdjust} -variable initPlacement -value "adjust"

  ttk::radiobutton .stepBox.initBox.none -text "None"  -command \
    {recall setNone} -variable initPlacement -value "none"

  ttk::checkbutton .stepBox.swapping -text "Swapping"  -command \
    {recall toggleSwapping} -variable orthoSwapping

  ttk::checkbutton .stepBox.swapbad -text "Swapping Badies"  -command \
    {recall toggleSwapBadies} -variable orthoSwapBad

  ttk::checkbutton .stepBox.edgeadjust -text "Edge Adjustment"  -command \
    {recall toggleEdgeAdjust} -variable orthoEdgeAdjust

  pack append .stepBox.initBox \
    .stepBox.initBox.text {top expand fillx} \
    .stepBox.initBox.random {top expand fillx} \
    .stepBox.initBox.adjust {top expand fillx} 
#    .stepBox.initBox.none {top expand fillx} 

  pack append .stepBox \
    .stepBox.initBox {top expand fillx} \
    .stepBox.swapping {top expand fillx} \
    .stepBox.swapbad {top expand fillx} \
    .stepBox.edgeadjust {top expand fillx} 
}

# procedure to show window .
proc ShowWindow. {args} {# xf ignore me 7

  # Window manager configurations
  global tk_version
  wm positionfrom . program
  wm sizefrom . program
  #wm maxsize . 130 500
  if {$tk_version >= 3.0} {
    wm protocol . WM_DELETE_WINDOW { recall deleteWindow;}
  }
  
  wm title . {Ortho Inspector}
  Ortho_MakePositionItems
  Ortho_MakeActionButtons
  Ortho_MakeCheckBox
  Ortho_MakeStepBox

  pack append . \
    .positionitems {top frame nw expand fillx padx 4 pady 4} \
    .checkBox {top frame nw expand fillx padx 4 pady 4} \
    .stepBox {top frame nw expand fillx padx 4 pady 4} \
    .actionbuttons {bottom frame center expand} 

  Ortho_SetupWindow .
  
}

StartupSrc

# initialize global variables
proc InitGlobals {} {
}

# initialize global variables
InitGlobals

# display/remove toplevel windows.
option add *HighlightThickness 0
option add *BorderWidth 2
ShowWindow.

