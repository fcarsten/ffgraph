#!/public/bin/wish -f
# Program: graph

global FF_LIBDIR
source $FF_LIBDIR/tcl/xfmisc.tcl
source $FF_LIBDIR/tcl/stdGraph.tcl
source $FF_LIBDIR/tcl/fileselect.tcl
source $FF_LIBDIR/tcl/povexp.tcl

proc userAddNodeMenu { menu } {
  $menu add separator
  
  $menu add command \
    -command {global nodeMenuCommand; $nodeMenuCommand showCluster} \
    -label {Show Cluster}

  $menu add command \
    -command {global nodeMenuCommand; $nodeMenuCommand hideCluster} \
    -label {Hide Cluster}
}

ShowWindow.
