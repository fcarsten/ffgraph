#!/public/bin/pixmap-wish -f
# Program: sugi
# Tcl version: 7.3 (Tcl/Tk/XF)
# Tk version: 3.6
# XF version: 2.3
#

proc Sugi_SetupWindow {w} {
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
set FF_LIBDIR "./lib/ffgraph"
source $FF_LIBDIR/tcl/xfmisc.tcl

# procedure to show window .
proc Sugi_MakeMiscButtons {} {
  frame .topframe.misc_buttons -relief ridge -bd 3
  label .topframe.misc_buttons.l -text "Parameters:" -anchor nw
  frame .topframe.misc_buttons.global -relief ridge -bd 2
  label .topframe.misc_buttons.global.l -text "Global:" -anchor nw
  frame .topframe.misc_buttons.global.left
  checkbutton .topframe.misc_buttons.global.left.startnode -text "Use startnode" -anchor nw \
    -command {recall toggleDecycle} -variable {toggleDecycle}
  checkbutton .topframe.misc_buttons.global.left.grid -text "Use Grid" -anchor nw \
    -command {recall useGrid} -variable {useGrid}
  checkbutton .topframe.misc_buttons.global.left.leafs_down -text "Leafs down" -anchor nw \
    -command {recall leavesDown} -variable {leavesDown}
  checkbutton .topframe.misc_buttons.global.left.deterministic -text "Deterministic" -anchor nw \
    -command {recall toggleDeterministic} -variable {deterministic}
  
  pack append .topframe.misc_buttons.global.left \
    .topframe.misc_buttons.global.left.startnode {top frame nw expand fillx} \
    .topframe.misc_buttons.global.left.grid {top frame nw expand fillx} \
    .topframe.misc_buttons.global.left.leafs_down {top frame nw expand fillx} \
    .topframe.misc_buttons.global.left.deterministic {top frame nw expand fillx} 
  
  frame .topframe.misc_buttons.global.right
  radiobutton .topframe.misc_buttons.global.right.3d -text "3D Layout" -anchor nw \
    -command {recall set3D} -variable {dimension} -value {3}
  radiobutton .topframe.misc_buttons.global.right.2d -text "2D Layout" -anchor nw \
    -command {recall set2D} -variable {dimension} -value {2}
  radiobutton .topframe.misc_buttons.global.right.horizontal -text "Horizontal Layout" -anchor nw \
    -command {recall orientHorizontal} -variable {orientation} -value {1}
  radiobutton .topframe.misc_buttons.global.right.vertical -text "Vertival Layout" -anchor nw \
    -command {recall orientVertical} -variable {orientation} -value {2}
  pack append .topframe.misc_buttons.global.right \
    .topframe.misc_buttons.global.right.3d {top frame nw expand fillx} \
    .topframe.misc_buttons.global.right.2d {top frame nw expand fillx} \
    .topframe.misc_buttons.global.right.horizontal {top frame nw expand fillx} \
    .topframe.misc_buttons.global.right.vertical {top frame nw expand fillx} 

  pack append .topframe.misc_buttons.global \
    .topframe.misc_buttons.global.l {top frame nw expand fillx} \
    .topframe.misc_buttons.global.left {left frame nw expand fillx} \
    .topframe.misc_buttons.global.right {right frame nw expand fillx} 
    
  pack append .topframe.misc_buttons \
    .topframe.misc_buttons.l {top frame nw expand fillx} \
    .topframe.misc_buttons.global {top frame nw expand fillx}
    
  pack append .topframe \
    .topframe.misc_buttons {left frame n padx 15}
}

proc Sugi_EntryInsert {entryName value} {
  $entryName delete 0 end
  $entryName insert 0 $value 
}

proc Sugi_MakePositionItems {} {
  proc Sugi_MakeEntry {name text recall_text} {
    frame $name
    label $name.l -text $text -anchor nw
    entry $name.e -width 8 -bg white
    $name.e insert 0 "0"
    bind $name.e <KeyPress> {
      if {("[expr %N -48]" != "%A") && ("%A" != "\{\}") && ("%K" != "BackSpace") && ("%K" != "Delete")} {
	break
      }
    }
    bind $name.e <KeyRelease> "recall $recall_text \[%W get\]"
    pack append $name $name.l {left} $name.e {right frame nw expand fillx}
  }
  proc Sugi_OffsetScaleTo {args} {
    .topframe.position.scale.l configure -text "Edgepoint Offset: $args%"
    recall edgePointOff $args
  }
  frame .topframe.position -relief ridge -bd 3
  label .topframe.position.l -text "Positioning:" -anchor nw
  frame .topframe.position.top
  frame .topframe.position.top.offset -relief ridge -bd 2
  label .topframe.position.top.offset.l -text "Offset" -anchor nw
  Sugi_MakeEntry .topframe.position.top.offset.x "X:" "xoff"
  Sugi_MakeEntry .topframe.position.top.offset.y "Y:" "yoff"
  Sugi_MakeEntry .topframe.position.top.offset.z "Z:" "zoff"
  pack append .topframe.position.top.offset \
    .topframe.position.top.offset.l {top frame nw expand fillx} \
    .topframe.position.top.offset.x {top frame nw expand fillx} \
    .topframe.position.top.offset.y {top frame nw expand fillx} \
    .topframe.position.top.offset.z {top frame nw expand fillx} 
  frame .topframe.position.top.distance -relief ridge -bd 2
  label .topframe.position.top.distance.l -text "Distance" -anchor nw
  Sugi_MakeEntry .topframe.position.top.distance.x "X:" "xdist"
  Sugi_MakeEntry .topframe.position.top.distance.y "Y:" "ydist"
  Sugi_MakeEntry .topframe.position.top.distance.z "Z:" "zdist"
  pack append .topframe.position.top.distance \
    .topframe.position.top.distance.l {top frame nw expand fillx} \
    .topframe.position.top.distance.x {top frame nw expand fillx} \
    .topframe.position.top.distance.y {top frame nw expand fillx} \
    .topframe.position.top.distance.z {top frame nw expand fillx} 
  pack append .topframe.position.top \
    .topframe.position.top.offset {left frame nw expand fillx} \
    .topframe.position.top.distance {left frame nw expand fillx}
  
  frame .topframe.position.radiobuttons
  label .topframe.position.radiobuttons.l -text "Use node size:" -anchor nw 
  radiobutton .topframe.position.radiobuttons.max -text "Max" -anchor nw -padx 3 \
    -command {recall setPositionMax} -variable {position} -value {1}
  radiobutton .topframe.position.radiobuttons.relative -text "Relative" -anchor nw -padx 3 \
    -command {recall setPositionRelative} -variable {position} -value {2}
  pack append .topframe.position.radiobuttons \
    .topframe.position.radiobuttons.l {top frame nw expand fillx} \
    .topframe.position.radiobuttons.max {left frame nw expand fillx} \
    .topframe.position.radiobuttons.relative {left frame nw expand fillx} 

  
  pack append .topframe.position \
    .topframe.position.l {top frame nw expand fillx} \
    .topframe.position.top {top frame nw expand fillx} \
    .topframe.position.radiobuttons {top frame nw} 
    
  pack append .topframe \
    .topframe.position {right frame center padx 15}

}

proc Sugi_MakeReduceItems {} {
  proc Sugi_ReduceScaleTo {args} {
    .middleframe.reduce_items.scale_l configure -text "Quality: $args"
    recall redCrossIters $args
  }
  frame .middleframe.reduce_items -relief ridge -bd 3
  label .middleframe.reduce_items.l -text "Reduce Crossing:" -anchor nw
  frame .middleframe.reduce_items.radiobuttons
  radiobutton .middleframe.reduce_items.radiobuttons.noneRed -text "None" -anchor nw \
    -padx 3 -command {recall noneRed} -variable {reduce} -value {noneRed}
  radiobutton .middleframe.reduce_items.radiobuttons.adjazenzmatrix -text "Adjazenzmatrix" -anchor nw \
    -padx 3 -command {recall adjazenz} -variable {reduce} -value {adjazenzmatrix}
  pack append .middleframe.reduce_items.radiobuttons \
    .middleframe.reduce_items.radiobuttons.noneRed {top frame nw expand fillx} \
  .middleframe.reduce_items.radiobuttons.adjazenzmatrix {top frame nw expand fillx}
  
  label .middleframe.reduce_items.scale_l -text "Quality: " -anchor nw
  frame .middleframe.reduce_items.scale -bd 2 -relief ridge
  frame .middleframe.reduce_items.scale.l
  label .middleframe.reduce_items.scale.l.low -text "Low" -anchor nw
  label .middleframe.reduce_items.scale.l.medium -text "Medium" -anchor center
  label .middleframe.reduce_items.scale.l.high -text "High" -anchor ne
  pack append .middleframe.reduce_items.scale.l \
    .middleframe.reduce_items.scale.l.low {left frame center expand fillx} \
    .middleframe.reduce_items.scale.l.medium {left frame center expand fillx} \
    .middleframe.reduce_items.scale.l.high {left frame ne expand fillx} 
  scale .middleframe.reduce_items.scale.scale \
    -command {Sugi_ReduceScaleTo} \
    -orient {horizontal} \
    -relief {sunken} \
    -length {250} \
    -bd 1 \
    -from {2} \
    -showvalue {0} \
    -width 10 \
    -to {15}

  pack append .middleframe.reduce_items.scale \
    .middleframe.reduce_items.scale.l {top expand fillx} \
    .middleframe.reduce_items.scale.scale {bottom} 
  
  pack append .middleframe.reduce_items \
    .middleframe.reduce_items.l {top frame nw expand fillx} \
    .middleframe.reduce_items.radiobuttons {top frame nw} \
    .middleframe.reduce_items.scale_l {top frame nw fillx} \
    .middleframe.reduce_items.scale {top frame nw fillx} 

  pack append .middleframe .middleframe.reduce_items {left frame center filly padx 15}
}

proc Sugi_MakeActionButtons {} {
  frame .middleframe.action_buttons 
  button .middleframe.action_buttons.ok -text "Just do it" -command {recall startLayout}
  button .middleframe.action_buttons.help -text "Help" -state disabled
  button .middleframe.action_buttons.dismiss -text "Dismiss" -command {recall deleteWindow}
  button .middleframe.action_buttons.save -text "Save" -state disabled
  button .middleframe.action_buttons.load -text "Load" -state disabled
  pack append .middleframe.action_buttons \
    .middleframe.action_buttons.ok {top frame center expand fillx} \
    .middleframe.action_buttons.help {top frame center expand fillx} \
    .middleframe.action_buttons.dismiss {top frame center expand fillx} \
    .middleframe.action_buttons.save {top frame center expand fillx} \
    .middleframe.action_buttons.load {top frame center expand fillx}
    
  pack append .middleframe \
    .middleframe.action_buttons {right frame center padx 15}



}

proc ShowWindow. {args} {# xf ignore me 7

  # Window manager configurations
  global tk_version
  wm positionfrom . program
  wm sizefrom . program
  #wm maxsize . 130 700
  if {$tk_version >= 3.0} {
    wm protocol . WM_DELETE_WINDOW { recall deleteWindow;}
  }
  wm title . {Sugiyama Inspector}

  frame .topframe
  frame .middleframe
  Sugi_MakeMiscButtons
  Sugi_MakePositionItems
  Sugi_MakeReduceItems
  Sugi_MakeActionButtons
  pack append . \
    .topframe {expand fillx} \
    .middleframe {expand fillx} 

  Sugi_SetupWindow .
}




# startup source
StartupSrc

# initialize global variables
proc InitGlobals {} {
  global position
  set position 0
  global reduce
  set reduce adjazenzmatrix
  global orientation
  set orientation 1
  global dimension
  set dimension 2
  global toggleDecycle
  global toggleEndNodes
  global useGrid
  global leavesDown
  global keepEdge
  global toggleEndNodes
  global deterministic
}

# initialize global variables
InitGlobals

# display/remove toplevel windows.
option add *HighlightThickness 0
option add *BorderWidth 2
ShowWindow.
