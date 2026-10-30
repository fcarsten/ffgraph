#!/public/bin/pixmap-wish -f
# Program: ortho
# Tcl version: 7.3 (Tcl/Tk/XF)
# Tk version: 3.6
# XF version: 2.3
#


proc Ortho_SetupWindow {w} {
  set tmplist [winfo children $w]
  while {[llength $tmplist] != 0} {
    switch [winfo class [lindex $tmplist 0]] {
      Radiobutton {[lindex $tmplist 0] configure -relief raised
	bind [lindex $tmplist 0] <1> {
	  %W configure -relief sunken
	}
	bind [lindex $tmplist 0] <ButtonRelease> {
	  %W configure -relief raised
	}
      }
      Checkbutton {[lindex $tmplist 0] configure -relief raised
	bind [lindex $tmplist 0] <1> {
	  %W configure -relief sunken
	}
	bind [lindex $tmplist 0] <ButtonRelease> {
	  %W configure -relief raised
	}
      }
    }
    set tmplist [concat $tmplist [winfo children [lindex $tmplist 0]]]
    set tmplist [lreplace $tmplist 0 0]
  }
}


global FF_LIBDIR
source $FF_LIBDIR/tcl/xfmisc.tcl

proc Ortho_MakePositionItems {} {
  proc Ortho_MakeMyEntry {name text recall_text} {
    frame $name
    label $name.l -text $text -anchor w
    entry $name.e -width 8 -relief sunken -bg white
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
  frame .positionitems -bd 3 -relief ridge
  label .positionitems.l -text "Positioning:" -anchor nw
  Ortho_MakeMyEntry .positionitems.xoffset "X Distance" yoff
  Ortho_MakeMyEntry .positionitems.yoffset "Y Distance" xoff
  pack append .positionitems \
    .positionitems.l {top frame nw expand fillx} \
    .positionitems.xoffset {top frame nw expand fillx} \
    .positionitems.yoffset {top frame nw expand fillx} 
}

proc Ortho_MakeActionButtons {} {
  frame .actionbuttons 
  button .actionbuttons.ok -text "Just do it" -anchor nw -command {recall startLayout}
  button .actionbuttons.dismiss -text Dismiss -anchor nw -command {recall deleteWindow}
  pack append .actionbuttons \
    .actionbuttons.ok {left} \
    .actionbuttons.dismiss {right}
}

proc Ortho_MakeCheckBox {} {
  frame .checkBox -relief ridge
  checkbutton .checkBox.3d -text "3D Layout" -anchor nw -command \
    {recall toggle3D}

  checkbutton .checkBox.anim -text "Animated" -anchor nw -command \
    {recall toggleAnim}

  pack append .checkBox \
    .checkBox.3d {top expand fillx} \
    .checkBox.anim {bottom expand fillx} 
}

proc Ortho_MakeStepBox {} {
  global initPlacement

  frame .stepBox -relief ridge
  frame .stepBox.initBox -relief ridge
  
  label .stepBox.initBox.text -text {Initial Placement}

  radiobutton .stepBox.initBox.random -text "Random" -anchor nw -command \
    {recall setRandom} -variable initPlacement -value "random"

  radiobutton .stepBox.initBox.adjust -text "Adjust" -anchor nw -command \
    {recall setAdjust} -variable initPlacement -value "adjust"

  radiobutton .stepBox.initBox.none -text "None" -anchor nw -command \
    {recall setNone} -variable initPlacement -value "none"

  checkbutton .stepBox.swapping -text "Swapping" -anchor nw -command \
    {recall toggleSwapping}

  checkbutton .stepBox.swapbad -text "Swapping Badies" -anchor nw -command \
    {recall toggleSwapBadies}

  checkbutton .stepBox.edgeadjust -text "Edge Adjustment" -anchor nw -command \
    {recall toggleEdgeAdjust}

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

