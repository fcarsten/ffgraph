#!/usr/bin/env wish
# Program: spring
# Tcl version: 7.3 (Tcl/Tk/XF)
# Tk version: 3.6
# XF version: 2.3
#


proc Spring_SetupWindow {w} {::ffgraph_ui::surface $w}


global FF_LIBDIR
source $FF_LIBDIR/tcl/xfmisc.tcl

proc Spring_MakeParamButtons {} {
  ttk::frame .parambuttons
  ttk::label .parambuttons.l -text "Parameters:" -anchor nw
  ttk::checkbutton .parambuttons.3d\
    -text "3D Layout"  \
    -command {recall toggle3D}\
    -variable {3dlayout}

  ttk::checkbutton .parambuttons.useedgeweight -text "Use Edgeweight"  \
    -command {recall setEdgeWeight} -variable {edgeweight}
  pack append .parambuttons \
    .parambuttons.l {top frame nw expand fillx} \
    .parambuttons.3d {top frame nw expand fillx} \
    .parambuttons.useedgeweight {top frame nw expand fillx}
}
 

proc Spring_MakePositionItems {} {
  proc Spring_MakeMyEntry {name text recall_text} {
    ttk::frame $name
    ttk::label $name.l -text $text -anchor w
    ttk::entry $name.e -width 8
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
  ttk::frame .positionitems
  ttk::label .positionitems.l -text "Positioning:" -anchor nw
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
  ttk::frame .animation
  ttk::label .animation.l -text "Animation:" -anchor nw
  ttk::checkbutton .animation.animation -text "Animate"  \
     -command {recall animate} -variable {animate}
  ttk::label .animation.l2 -text "Iterations per Animation" -anchor nw
  ::ffgraph_ui::scale .animation.scale  \
    -command {recall animIters} \
    -orient {horizontal} \
    -length {100} \
    -from {2} \
    -showvalue {1} \
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
    .quality.scale.l.low configure -state normal
    .quality.scale.l.medium configure -state normal
    .quality.scale.l.high configure -state normal
    .quality.f.check configure -command Sugi_QualityUnlimited
    recall quality [.quality.scale.scale get]
  }

  proc Sugi_QualityUnlimited { } {
    .quality.f.l configure -text "Quality: oo"
    .quality.scale.scale configure -state disabled
    .quality.scale.l.low configure -state disabled
    .quality.scale.l.medium configure -state disabled
    .quality.scale.l.high configure -state disabled
    .quality.f.check configure -command Sugi_QualityLimited
    recall quality 0
  }
  
  ttk::frame .quality
  ttk::frame .quality.f
  ttk::label .quality.f.l -text "Quality: " -anchor nw
  ttk::checkbutton .quality.f.check -text "oo"  \
    -command {Sugi_QualityUnlimited}
  
  ttk::frame .quality.scale
  ttk::frame .quality.scale.l
  ttk::label .quality.scale.l.low -text "Low" -anchor nw
  ttk::label .quality.scale.l.medium -text "Medium" -anchor center
  ttk::label .quality.scale.l.high -text "High" -anchor ne
  pack append .quality.scale.l \
    .quality.scale.l.low {left frame center expand fillx} \
    .quality.scale.l.medium {left frame center expand fillx} \
    .quality.scale.l.high {left frame ne expand fillx} 
  ::ffgraph_ui::scale .quality.scale.scale \
    -command {Sugi_QualityScaleTo} \
    -orient {horizontal} \
    -length {100} \
    -from {1} \
    -showvalue {0} \
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
  
  ttk::frame .actionbuttons
  ttk::button .actionbuttons.ok -text "Just do it"  \
    -command {Spring_DoIt}
  ttk::button .actionbuttons.dismiss -text Dismiss  \
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

