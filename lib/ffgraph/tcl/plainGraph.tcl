#!/public/bin/wish -f
# Program: graph

global FF_LIBDIR
source $FF_LIBDIR/tcl/xfmisc.tcl
source $FF_LIBDIR/tcl/fileselect.tcl
source $FF_LIBDIR/tcl/stdGraph.tcl

# The shared Info page is defined by stdGraph.tcl.

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
