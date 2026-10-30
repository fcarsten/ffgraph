#!/public/bin/pixmap-wish -f
# Program: naiv
# Tcl version: 7.3 (Tcl/Tk/XF)
# Tk version: 3.6
# XF version: 2.3
#


proc Naiv_SetupWindow {w} {
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

proc Naiv_MakePositionItems {} {
  proc Naiv_MakeMyEntry {name text recall_text} {
    frame $name
    label $name.l -text $text -anchor w
    entry $name.e -width 8 -relief sunken -bg white
    $name.e insert 1 "100"
    bind $name.e <KeyPress> {
      if {("[expr %N -48]" != "%A") && ("%A" != "\{\}") && ("%K" != "BackSpace") && ("%K" != "Delete")} {
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
  Naiv_MakeMyEntry .positionitems.xoffset "X Distance" yoff
  Naiv_MakeMyEntry .positionitems.yoffset "Y Distance" xoff
  pack append .positionitems \
    .positionitems.l {top frame nw expand fillx} \
    .positionitems.xoffset {top frame nw expand fillx} \
    .positionitems.yoffset {top frame nw expand fillx} 
}

proc Naiv_MakeActionButtons {} {
  frame .actionbuttons 
  button .actionbuttons.ok -text "Just do it" -anchor nw -command {recall startLayout}
  button .actionbuttons.dismiss -text Dismiss -anchor nw -command {recall deleteWindow}
  pack append .actionbuttons \
    .actionbuttons.ok {left} \
    .actionbuttons.dismiss {right}
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
  
  wm title . {Naiv Inspector}
  Naiv_MakePositionItems
  Naiv_MakeActionButtons
  pack append . \
    .positionitems {top frame nw expand fillx padx 4 pady 4} \
    .actionbuttons {top frame center expand} 

  Naiv_SetupWindow .
  
}

StartupSrc

# initialize global variables
proc InitGlobals {} {
  global 3dlayout
  set 3dlayout 0
  global edgeweight
  set edgeweight 0
  global animate
  set animate 0
}

# initialize global variables
InitGlobals

# display/remove toplevel windows.
option add *HighlightThickness 0
option add *BorderWidth 2
ShowWindow.

