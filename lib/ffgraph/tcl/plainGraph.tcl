#!/public/bin/wish -f
# Program: graph

global FF_LIBDIR
source $FF_LIBDIR/tcl/xfmisc.tcl
source $FF_LIBDIR/tcl/fileselect.tcl
source $FF_LIBDIR/tcl/stdGraph.tcl

# Procedure: showUsageBox
proc addUserUsage {fenster} {
  wm title $fenster {Graph Usage: }

  # build widget $fenster.button4
  button $fenster.button4 -command "destroy $fenster"  -text {OK}

  # build widget $fenster.message3
  message $fenster.message3 -aspect {1331} -borderwidth {4} -justify {left}\
    -padx {5} -pady {2} -relief {raised} -text {
    Shift-XX, Ctrl-XX, Alt-XX etc := press key Shift, Ctrl, ... while doing XX
    Mn := Press Mouse Button n
    
    Add Node: Ctrl-M1 on background
    Add Edge: Ctrl-M1 on source Node then Ctrl-M1 on target Node
    Add Edgepoint: Select Edge then Shift-Ctrl-M1 on background

    Move Node: M1 on Node then drag
    Move Edge: M1 on Edgepoint then drag
    Move Edgepoint: Shift-M1 on Edgepoint then drag

    Delete Node: M2 on Node
    Delete Edge: M2 on Edge
    Delete Edgepoint: Select Edge, Shift-Ctrl-M2 near point

    Bring up Node Menu: M3 on Node
    Bring up Edge Menu: M3 on Edge
      
    Zoom to a rectangle: Mod-M1 on brackground then drag
  }

  # pack widget $fenster
  pack append $fenster     \
    $fenster.message3 {top frame center}     \
    $fenster.button4 {bottom frame center fillx}
}

# end source

# display/remove toplevel windows.
ShowWindow.

# load default bindings.
# if {[info exists env(XF_BIND_FILE)] &&
#     "[info procs XFShowHelp]" == ""} {
#   source $env(XF_BIND_FILE)
# }

# parse and apply application defaults.
XFLocalLoadAppDefs Graph
XFLocalSetAppDefs

# eof
#
