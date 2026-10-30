#!/public/bin/pixmap-wish -f
# Program: spring
# Tcl version: 7.3 (Tcl/Tk/XF)
# Tk version: 3.6
# XF version: 2.3
#


proc Spring_SetupWindow {w} {
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

proc Spring_MakeParamButtons {} {
  frame .parambuttons -bd 3 -relief ridge
  label .parambuttons.l -text "Parameters:" -anchor nw 
  checkbutton .parambuttons.3d\
    -text "3D Layout" -anchor nw \
    -command {recall toggle3D}\
    -variable {3dlayout}

  checkbutton .parambuttons.useedgeweight -text "Use Edgeweight" -anchor nw \
    -command {recall setEdgeWeight} -variable {edgeweight}
  pack append .parambuttons \
    .parambuttons.l {top frame nw expand fillx} \
    .parambuttons.3d {top frame nw expand fillx} \
    .parambuttons.useedgeweight {top frame nw expand fillx}
}
 

proc Spring_MakePositionItems {} {
  proc Spring_MakeMyEntry {name text recall_text} {
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
  Spring_MakeMyEntry .positionitems.xoffset "X Offset" xoff
  Spring_MakeMyEntry .positionitems.yoffset "Y Offset" yoff
  Spring_MakeMyEntry .positionitems.zoffset "Z Offset" zoff
  Spring_MakeMyEntry .positionitems.distance "Distance" dist
  pack append .positionitems \
    .positionitems.l {top frame nw expand fillx} \
    .positionitems.distance {top frame nw expand fillx} 

#     .positionitems.xoffset {top frame nw expand fillx} 
#     .positionitems.yoffset {top frame nw expand fillx} 
#     .positionitems.zoffset {top frame nw expand fillx} 
}

proc Spring_MakeAnimation {} {
  frame .animation -bd 3 -relief ridge
  label .animation.l -text "Animation:" -anchor nw
  checkbutton .animation.animation -text "Animate" -anchor nw \
     -command {recall animate} -variable {animate}
  label .animation.l2 -text "Iterations per Animation" -anchor nw
  scale .animation.scale  \
    -command {recall animIters} \
    -orient {horizontal} \
    -relief {sunken} \
    -length {100} \
    -bd 1 \
    -from {2} \
    -showvalue {1} \
    -width 10 \
    -to {30}
  pack append .animation \
    .animation.l {top frame nw expand fillx} \
    .animation.animation {top frame nw} \
    .animation.l2 {top frame nw expand fillx} \
    .animation.scale {top frame nw expand fillx} 
}

proc Spring_MakeQuality {} {
  proc Sugi_QualityScaleTo {args} {
    .quality.f.l configure -text "Quality: $args"
    recall quality $args
  }
  
  proc Sugi_QualityLimited { } {
    .quality.f.l configure -text "Quality: [.quality.scale.scale get]"
    .quality.scale.scale configure -state normal
    .quality.scale.l.low configure -fg "black"
    .quality.scale.l.medium configure -fg "black"
    .quality.scale.l.high configure -fg "black"
    .quality.f.check configure -command Sugi_QualityUnlimited
    recall quality [.quality.scale.scale get]
  }

  proc Sugi_QualityUnlimited { } {
    .quality.f.l configure -text "Quality: oo"
    .quality.scale.scale configure -state disabled
    .quality.scale.l.low configure -fg "grey"
    .quality.scale.l.medium configure -fg "grey"
    .quality.scale.l.high configure -fg "grey"
    .quality.f.check configure -command Sugi_QualityLimited
    recall quality 0
  }
  
  frame .quality -relief ridge -bd 3
  frame .quality.f -relief ridge -bd 3
  label .quality.f.l -text "Quality: " -anchor nw
  checkbutton .quality.f.check -text "oo" -anchor nw \
    -command {Sugi_QualityUnlimited}
  
  frame .quality.scale -bd 2 -relief ridge
  frame .quality.scale.l
  label .quality.scale.l.low -text "Low" -anchor nw
  label .quality.scale.l.medium -text "Medium" -anchor center
  label .quality.scale.l.high -text "High" -anchor ne
  pack append .quality.scale.l \
    .quality.scale.l.low {left frame center expand fillx} \
    .quality.scale.l.medium {left frame center expand fillx} \
    .quality.scale.l.high {left frame ne expand fillx} 
  scale .quality.scale.scale \
    -command {Sugi_QualityScaleTo} \
    -orient {horizontal} \
    -relief {sunken} \
    -length {100} \
    -bd 1 \
    -from {1} \
    -showvalue {0} \
    -width 10 \
    -to {10}
  pack append .quality.scale \
    .quality.scale.l  {top frame nw expand fillx} \
    .quality.scale.scale {top frame nw expand fillx}

  pack append .quality.f \
    .quality.f.l {left frame w expand fillx} \
    .quality.f.check {right frame e expand fillx}
    
  pack append .quality \
    .quality.f {top frame nw expand fillx} \
    .quality.scale {top frame center expand fillx}
  
}

proc Spring_MakeActionButtons {} {
  proc Spring_DoIt {} {
    .actionbuttons.ok configure -text "  Stop   "
    .actionbuttons.ok configure -command "recall stop"
    recall startLayout
    .actionbuttons.ok configure -text "Just do it"
    .actionbuttons.ok configure -command "Spring_DoIt"
  }
  
  frame .actionbuttons 
  button .actionbuttons.ok -text "Just do it" -anchor nw \
    -command {Spring_DoIt}
  button .actionbuttons.dismiss -text Dismiss -anchor nw \
    -command {recall deleteWindow}

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
  
  wm title . {Springembedder Inspector}
  Spring_MakeParamButtons
  Spring_MakePositionItems
  Spring_MakeAnimation
  Spring_MakeQuality
  Spring_MakeActionButtons
  pack append . \
    .parambuttons {top frame nw expand fillx padx 4 pady 4} \
    .positionitems {top frame nw expand fillx padx 4 pady 4} \
    .animation {top frame nw expand fillx padx 4 pady 4} \
    .quality {top frame nw expand fillx padx 4 pady 4} \
    .actionbuttons {top frame center expand} 

  Spring_SetupWindow .
  
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

