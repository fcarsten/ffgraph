# Formatted, selectable Info content shared by the 2D and 3D applications.
if {[namespace exists ::ffgraph_info]} {return}
namespace eval ::ffgraph_info {}

proc ::ffgraph_info::linuxURLCommand {url} {
  # WSL may have gio but no Linux browser/HTTP handler. Use Windows' browser
  # through interop instead of treating gio's presence as a working desktop.
  if {[info exists ::env(WSL_INTEROP)] || [info exists ::env(WSL_DISTRO_NAME)]} {
    set command [auto_execok wslview]
    if {$command ne ""} {return [list {*}$command $url]}
    set command [auto_execok rundll32.exe]
    if {$command ne ""} {return [list {*}$command url.dll,FileProtocolHandler $url]}
  }
  set command [auto_execok xdg-open]
  if {$command ne ""} {return [list {*}$command $url]}
  set command [auto_execok gio]
  if {$command ne ""} {return [list {*}$command open $url]}
  error {No desktop URL launcher was found.}
}

proc ::ffgraph_info::openLink {window url} {
  if {[catch {
    if {![regexp {^https?://} $url]} {error {Unsupported link type}}
    switch -- [tk windowingsystem] {
      win32 {
        set command [auto_execok rundll32.exe]
        if {$command eq ""} {error {The Windows URL launcher was not found.}}
        exec {*}$command url.dll,FileProtocolHandler $url &
      }
      aqua {exec open $url &}
      default {
        exec {*}[linuxURLCommand $url] &
      }
    }
  } message]} {
    tk_messageBox -parent $window -title {Open link} -icon error -type ok \
      -message "Could not open $url.\n\n$message\n\nYou can select and copy the link into your browser."
  }
}

proc ::ffgraph_info::clickLink {text window url} {
  # Keep dragging/selecting a URL available for copying without launching it.
  if {[$text tag ranges sel] eq ""} {openLink $window $url}
}

proc ::ffgraph_info::link {text window label url} {
  set tag url[llength [lsearch -all -inline -glob [$text tag names] url*]]
  $text insert end $label [list hyperlink $tag]
  $text tag bind $tag <Enter> [list $text configure -cursor hand2]
  $text tag bind $tag <Leave> [list $text configure -cursor xterm]
  $text tag bind $tag <ButtonRelease-1> [list ::ffgraph_info::clickLink $text $window $url]
  $text tag bind $tag <Return> [list ::ffgraph_info::openLink $window $url]
}

proc ::ffgraph_info::focusLink {text direction} {
  set links [lsearch -all -inline -glob [$text tag names] url*]
  set current [lsearch -inline -glob [$text tag names insert] url*]
  set index [lsearch -exact $links $current]
  if {$index < 0 && $direction < 0} {set index [llength $links]}
  incr index $direction
  $text tag remove linkFocus 1.0 end
  if {$index < 0 || $index >= [llength $links]} {
    if {$direction > 0} {focus [tk_focusNext $text]} else {focus [tk_focusPrev $text]}
    return
  }
  lassign [$text tag ranges [lindex $links $index]] first last
  $text mark set insert $first
  $text tag add linkFocus $first $last
  $text see $first
}

proc ::ffgraph_info::activateLink {text} {
  foreach tag [$text tag names insert] {
    if {[string match url* $tag]} {
      uplevel #0 [$text tag bind $tag <Return>]
      return
    }
  }
}

proc ::ffgraph_info::build {window} {
  wm title $window {Info}
  wm transient $window .
  wm minsize $window 360 240
  set text $window.message3
  set lines [expr {min(28, max(10, int([winfo screenheight $window]*0.7/[font metrics TkDefaultFont -linespace])-4))}]
  set columns [expr {min(86, max(40, int([winfo screenwidth $window]*0.8/[font measure TkDefaultFont 0])))}]
  text $text -wrap word -width $columns -height $lines -font TkDefaultFont \
    -borderwidth 0 -highlightthickness 1 -padx 16 -pady 12 -takefocus 1 \
    -yscrollcommand [list $window.scroll set]
  ttk::scrollbar $window.scroll -orient vertical -command [list $text yview]
  ttk::button $window.button4 -text OK -command [list destroy $window]
  grid $text -row 0 -column 0 -sticky news -padx {12 0} -pady {12 0}
  grid $window.scroll -row 0 -column 1 -sticky ns -padx {0 12} -pady {12 0}
  grid $window.button4 -row 1 -column 0 -columnspan 2 -sticky e -padx 12 -pady 12
  grid columnconfigure $window 0 -weight 1
  grid rowconfigure $window 0 -weight 1
  ::ffgraph_ui::surface $window
  set font [font actual TkDefaultFont]
  set size [dict get $font -size]
  $text tag configure title -font [dict replace $font -weight bold -size [expr {$size > 0 ? $size+6 : $size-8}]] -spacing3 8
  $text tag configure heading -font [dict replace $font -weight bold] -spacing1 14 -spacing3 5
  $text tag configure strong -font [dict replace $font -weight bold]
  $text tag configure hyperlink -foreground #005ea8 -underline 1
  $text tag configure linkFocus -relief solid -borderwidth 1
  $text insert end "ffgraph\n" title
  $text insert end "Authors and contributors to the historic code\n" heading
  foreach {label content} {
    {Main authors} {Carsten Friedrich, Falk Schreiber}
    Contributors {Dirk Koschützki, Andreas Holzmann, Achim Dannecker, Andreas Claßen, Volker Braun, Barbara Zündorf, others at the Lehrstuhl für Programmiersysteme University of Passau}
    {Project lead} {Bernhard Steffen, Head of Lehrstuhl für Programmiersysteme at the University of Passau}
  } {
    $text insert end "$label: " strong "$content\n\n"
  }
  $text insert end "License\n" heading
  link $text $window {GNU General Public License, version 3} https://www.gnu.org/licenses/gpl-3.0.html
  $text insert end "\nProject\n" heading
  $text insert end "GitHub: " strong
  link $text $window https://github.com/fcarsten/ffgraph https://github.com/fcarsten/ffgraph
  $text insert end "\nUsage (historic text)\n" heading
  $text insert end "Shift-XX, Ctrl-XX, Alt-XX etc := press key Shift, Ctrl, ... while doing XX\nMn := Press Mouse Button n\n\n"
  foreach {label action} {
    {Add Node} {Ctrl-M1 on background}
    {Add Edge} {Ctrl-M1 on source Node then Ctrl-M1 on target Node}
    {Add Edgepoint} {Select Edge then Shift-Ctrl-M1 on background}
    {Move Node} {M1 on Node then drag}
    {Move Edge} {M1 on Edgepoint then drag}
    {Move Edgepoint} {Shift-M1 on Edgepoint then drag}
    {Delete Node} {M2 on Node}
    {Delete Edge} {M2 on Edge}
    {Delete Edgepoint} {Select Edge, Shift-Ctrl-M2 near point}
    {Bring up Node Menu} {M3 on Node}
    {Bring up Edge Menu} {M3 on Edge}
    {Zoom to a rectangle} {Alt-M1 on background then drag}
  } {
    $text insert end "$label: " strong "$action\n"
  }
  $text configure -state disabled
  $text mark set insert 1.0
  foreach {key command} {<Tab> {focusLink 1} <Shift-Tab> {focusLink -1} <Return> activateLink <space> activateLink} {
    set handler [linsert $command 1 $text]
    lset handler 0 ::ffgraph_info::[lindex $handler 0]
    bind $text $key "[list {*}$handler]; break"
  }
  bind $window <Escape> [list destroy $window]
}
