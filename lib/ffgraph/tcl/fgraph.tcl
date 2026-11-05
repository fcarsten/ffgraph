#!/public/bin/wish -f
# Program: graph

global FF_LIBDIR
source $FF_LIBDIR/tcl/xfmisc.tcl
source $FF_LIBDIR/tcl/stdGraph.tcl
source $FF_LIBDIR/tcl/fileselect.tcl
source $FF_LIBDIR/tcl/povexp.tcl

proc createButtonRow { frame text befehl {gr 50} {r 10} {l -10} {gl -50}} {
  # build widget .frame2.frame7.frame14.label4
  ttk::label $frame.label \
    -text $text

  # build widget .frame2.frame7.frame14.button1
  createButton $frame.button1 { >> }  $befehl $gr

  # build widget .frame2.frame7.frame14.button2
  createButton $frame.button2  { >  } $befehl $r

  # build widget .frame2.frame7.frame14.button3
  createButton $frame.button3  {  < } $befehl $l

  # build widget .frame2.frame7.frame14.button4
  createButton $frame.button4  { << } $befehl $gl

  # pack widget .frame2.frame7.frame14
  pack append $frame \
    $frame.label {left frame center} \
    $frame.button1 {right frame center} \
    $frame.button2 {right frame center} \
    $frame.button3 {right frame center} \
    $frame.button4 {right frame center} 
}

proc buildToolBar {} {
  global workingMode
  
  ttk::frame .toolFrame
   set name .toolFrame
   set workingMode Create
 
   ttk::button $name.b1 \
     -text "Create" \
     -command "setWorkingMode Create $name.e1"
   
   ttk::button $name.b2 \
     -text "Navigate" \
     -command "setWorkingMode Navigate $name.e1"


   ttk::button $name.b3 \
     -text "Zoom" \
     -command "setWorkingMode Zoom $name.e1"

   ttk::entry $name.e1

   $name.e1 insert 0 "Create" 

   $name.e1 configure -state disabled

   pack append .toolFrame \
     .toolFrame.b1 {left frame center} \
     .toolFrame.b2 {left frame center} \
     .toolFrame.b3 {left frame center} \
     .toolFrame.e1 {right frame center filly}
}

proc buildButtonFrame {} {
  # build widget .frame2
  ttk::frame .frame2

  # build widget .frame2.frame7
  ttk::frame .frame2.frame7
    
  ttk::frame .frame2.frame7.row1

  # build widget .frame2.frame7.row1.frame11
  ttk::frame .frame2.frame7.row1.frame11

  createButtonRow .frame2.frame7.row1.frame11 "Move Y:" "setY"

  # build widget .frame2.frame7.row1.frame12
  ttk::frame .frame2.frame7.row1.frame12

  createButtonRow .frame2.frame7.row1.frame12 "Zoom:" "zoom" 0.7 0.95 1.05 1.5

  # build widget .frame2.frame7.row1.frame13
  ttk::frame .frame2.frame7.row1.frame13

  createButtonRow .frame2.frame7.row1.frame13 "Move X:" "setX"

  # build widget .frame2.frame7.row1.frame14
  ttk::frame .frame2.frame7.row1.frame14

  createButtonRow .frame2.frame7.row1.frame14 "Move Z:" "setZ"

#  button .frame2.frame7.row1.pov -text "Export POV2" -command ShowPovWindow
  ttk::button .frame2.frame7.row1.pov -text "Export POV2" -command {recall exportPOV}
#     {if {![string match "*.pov*" [info commands]]}\
#        {showPovWindow} }
    
  # pack widget .frame2.frame7.row1
  pack append .frame2.frame7.row1 \
    .frame2.frame7.row1.pov {left frame center fill} \
    .frame2.frame7.row1.frame13 {left frame center fill} \
    .frame2.frame7.row1.frame14 {left frame center fill} \
    .frame2.frame7.row1.frame11 {left frame center fill} \
    .frame2.frame7.row1.frame12 {left frame center fill}


  ttk::frame .frame2.frame7.row2

  # build widget .frame2.frame7.row1.frame11
  ttk::frame .frame2.frame7.row2.frame11

  pack append .frame2.frame7.row2 \
  .frame2.frame7.row2.frame11 {left frame center fill} 

  pack append .frame2.frame7 \
      .frame2.frame7.row1 {top frame center fill} \
      .frame2.frame7.row2 {top frame center fill}

  # build widget .frame2.frame8
  ttk::frame .frame2.frame8

  # build widget .frame2.frame8.scale14
  ::ffgraph_ui::scale .frame2.frame8.scale14 \
    -command {global aktuCommand; $aktuCommand setXWinkel } \
    -orient {horizontal} \
    -from {-360} \
    -showvalue {0} \
    -to {360}

  .frame2.frame8.scale14 set {0}

  # build widget .frame2.frame8.scale15
  ::ffgraph_ui::scale .frame2.frame8.scale15 \
    -command {global aktuCommand; $aktuCommand setYWinkel} \
    -orient {horizontal} \
    -showvalue {0} \
    -from {-360} \
    -to {360}

  .frame2.frame8.scale15 set {0}

  # build widget .frame2.frame8.scale16
  ::ffgraph_ui::scale .frame2.frame8.scale16 \
    -command {global aktuCommand; $aktuCommand setZWinkel} \
    -orient {horizontal} \
    -showvalue {0} \
    -from {-360} \
    -to {360}

  .frame2.frame8.scale16 set {0}

#   # build widget .frame2.frame8.scale17

  ::ffgraph_ui::scale .frame2.frame8.scale17
   
#      -width 10 \
#      -background {gainsboro} \
#     -command {global aktuCommand; $aktuCommand setZvalue} \
#     -orient {horizontal} \
#     -relief {raised} \
#     -showvalue {0} \
#     -from {-1500} \
#     -to {-100}

#   .frame2.frame8.scale17 set {-1000}

  ::ffgraph_ui::scale .frame2.frame8.scale18 \
    -command {global aktuCommand; $aktuCommand viewDist} \
    -orient {horizontal} \
    -showvalue {0} \
    -from {100} \
    -to {2000}

  .frame2.frame8.scale18 set {1000}

# pack widget .frame2.frame8

   pack append .frame2.frame8 \
    .frame2.frame8.scale14 {top frame center fillx} \
    .frame2.frame8.scale15 {top frame center fillx} \
    .frame2.frame8.scale16 {top frame center fillx} \
    .frame2.frame8.scale18 {top frame center fillx} 

#    .frame2.frame8.scale17 {top frame center fillx}

  # pack widget .frame2
  pack append .frame2 \
    .frame2.frame7 {top frame center expand fillx} \
    .frame2.frame8 {bottom frame center expand fillx}
}

ShowWindow.
