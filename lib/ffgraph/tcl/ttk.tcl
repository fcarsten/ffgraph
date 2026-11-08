# Shared themed controls. Keep Tk's own widget commands/bindings untouched.
if {[namespace exists ::ffgraph_ui]} {return}
package require Ttk 8.6
namespace eval ::ffgraph_ui {
  variable scales
  variable colors
}
if {[tk windowingsystem] eq "win32"} {
  foreach theme {vista xpnative winnative} {
    if {$theme in [ttk::style theme names]} {ttk::style theme use $theme; break}
  }
} elseif {[tk windowingsystem] eq "x11"} {
  ttk::style theme use clam
}

# Theme the few Tk-only display widgets without recoloring graph canvases.
proc ::ffgraph_ui::themeColor {style option fallback {state {}}} {
  set color [ttk::style lookup $style $option $state]
  if {$color eq ""} {return $fallback}
  return $color
}
proc ::ffgraph_ui::surface {window} {
  set background [themeColor TFrame -background #d9d9d9]
  $window configure -background $background
  set pending [winfo children $window]
  while {[llength $pending]} {
    set child [lindex $pending 0]
    set pending [concat [lrange $pending 1 end] [winfo children $child]]
    switch -- [winfo class $child] {
      Listbox - Text {
        $child configure -background [themeColor TEntry -fieldbackground white] \
          -foreground [themeColor TLabel -foreground black] \
          -selectbackground [themeColor Treeview -background #4a6984 selected] \
          -selectforeground [themeColor Treeview -foreground white selected]
      }
      Message {$child configure -background $background -foreground [themeColor TLabel -foreground black]}
    }
  }
}

# Composite ttk slider preserving integer values, captions and value displays
# used by the graph and layout callbacks. Its public path keeps the old API.
proc ::ffgraph_ui::scale {path args} {
  variable scales
  set scales($path) [dict merge {-from 0 -to 100 -orient vertical -length 100 -label {} -showvalue 1 -command {} -resolution 1 -state normal} $args]
  dict set scales($path) value [dict get $scales($path) -from]
  ttk::frame $path
  ttk::label $path.caption -text [dict get $scales($path) -label]
  ttk::label $path.value -width 5 -anchor e
  ttk::scale $path.slider -from [dict get $scales($path) -from] \
    -to [dict get $scales($path) -to] -orient [dict get $scales($path) -orient] \
    -length [dict get $scales($path) -length] -command [list ::ffgraph_ui::scaleChanged $path]
  if {[dict get $scales($path) -label] ne ""} {grid $path.caption -row 0 -column 0 -columnspan 2 -sticky w}
  grid $path.slider -row 1 -column 0 -sticky news
  if {[dict get $scales($path) -showvalue]} {grid $path.value -row 1 -column 1 -sticky e}
  grid columnconfigure $path 0 -weight 1
  grid rowconfigure $path 1 -weight 1
  rename $path ${path}.__frame
  interp alias {} $path {} ::ffgraph_ui::scaleCommand $path
  bind $path <Destroy> [list ::ffgraph_ui::forgetScale $path %W]
  scaleCommand $path set [dict get $scales($path) -from]
  return $path
}
proc ::ffgraph_ui::scaleNumber {path value} {
  variable scales
  set from [dict get $scales($path) -from]
  set to [dict get $scales($path) -to]
  set step [dict get $scales($path) -resolution]
  if {$step > 0} {set value [expr {$from + round(($value-$from)/double($step))*$step}]}
  return [expr {min(max($value,min($from,$to)),max($from,$to))}]
}
proc ::ffgraph_ui::scaleChanged {path value} {
  variable scales
  if {[dict exists $scales($path) busy]} {return}
  scaleCommand $path set $value
}
proc ::ffgraph_ui::scaleInvoke {path} {
  variable scales
  if {![info exists scales($path)]} {return}
  dict unset scales($path) pending
  if {![winfo ismapped $path]} {return}
  set command [dict get $scales($path) -command]
  set value [dict get $scales($path) value]
  if {$command ne ""} {uplevel #0 "$command [list $value]"}
}
proc ::ffgraph_ui::scaleCommand {path operation args} {
  variable scales
  switch -- $operation {
    get {return [scaleNumber $path [$path.slider get]]}
    set {
      if {[dict get $scales($path) -state] eq "disabled"} {return}
      set value [scaleNumber $path [lindex $args 0]]
      set changed [expr {$value != [dict get $scales($path) value]}]
      dict set scales($path) busy 1
      try {$path.slider set $value} finally {dict unset scales($path) busy}
      $path.value configure -text $value
      dict set scales($path) value $value
      # Classic scales coalesce changed values into one idle callback and do
      # not invoke commands while unmapped (including initial construction).
      if {$changed && [winfo ismapped $path] && ![dict exists $scales($path) pending]} {
        dict set scales($path) pending [after idle [list ::ffgraph_ui::scaleInvoke $path]]
      }
    }
    cget {return [dict get $scales($path) [lindex $args 0]]}
    config - configure {
      foreach {option value} $args {
        dict set scales($path) $option $value
        switch -- $option {
          -command - -resolution - -showvalue {}
          -label {$path.caption configure -text $value}
          default {$path.slider configure $option $value}
        }
      }
    }
    default {return [$path.slider $operation {*}$args]}
  }
}
proc ::ffgraph_ui::forgetScale {path destroyed} {
  if {$path ne $destroyed} {return}
  variable scales
  if {[dict exists $scales($path) pending]} {after cancel [dict get $scales($path) pending]}
  unset scales($path)
  rename $path {}
}

# Native buttons cannot reliably display a custom background on Windows.
# Keep semantic color samples visible as image-backed ttk controls instead.
proc ::ffgraph_ui::swatch {kind path args} {
  variable colors
  set photo [image create photo -width 36 -height 18]
  set colors($path) [dict create -background white -foreground black -relief raised photo $photo]
  ttk::$kind $path {*}$args -image $photo -compound left
  rename $path ${path}.__swatch
  interp alias {} $path {} ::ffgraph_ui::swatchCommand $path
  bind $path <Destroy> [list ::ffgraph_ui::forgetSwatch $path %W]
  $photo put white -to 0 0 36 18
  return $path
}
proc ::ffgraph_ui::swatchCommand {path operation args} {
  variable colors
  set native ${path}.__swatch
  if {$operation in {config configure cget}} {
    set option [lindex $args 0]
    if {$option eq "-bg"} {set option -background}
    if {$option eq "-fg"} {set option -foreground}
    if {[llength $args] == 1 && [dict exists $colors($path) $option]} {
      set value [dict get $colors($path) $option]
      if {$operation eq "cget"} {return $value}
      return [list $option {} {} {} $value]
    }
    if {[llength $args] > 1} {
      foreach {option value} $args {
        switch -- $option {
          -activebackground {}
          -background - -bg {
            dict set colors($path) -background $value
            [dict get $colors($path) photo] put [list [list $value]] -to 0 0 36 18
          }
          -foreground - -fg {dict set colors($path) -foreground $value; $native configure -foreground $value}
          -relief {
            dict set colors($path) -relief $value
            $native state [expr {$value eq "sunken" ? "pressed" : "!pressed"}]
          }
          default {$native configure $option $value}
        }
      }
      return
    }
  }
  return [$native $operation {*}$args]
}
proc ::ffgraph_ui::forgetSwatch {path destroyed} {
  if {$path ne $destroyed} {return}
  variable colors
  image delete [dict get $colors($path) photo]
  unset colors($path)
  rename $path {}
}
