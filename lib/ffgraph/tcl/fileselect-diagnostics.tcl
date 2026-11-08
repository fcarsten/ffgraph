# Opt-in selector diagnostics. Append and close each record so a stalled GUI
# still leaves useful output. Never run update or change focus/window state.
if {[namespace exists ::ffgraph_selector_debug]} {return}
namespace eval ::ffgraph_selector_debug {
  variable installed 0
  variable busy 0
  variable failedPath {}
  variable session [list [pid] [clock clicks]]
  variable timers {}
  variable opening 0
  variable sourceFile [file join [file dirname [info script]] fileselect.tcl]
}

proc ::ffgraph_selector_debug::record {event {details {}}} {
  variable busy
  variable session
  variable failedPath
  variable opening
  if {$busy || ![info exists ::env(FFGRAPH_FILE_SELECTOR_LOG)]} {return}
  set path $::env(FFGRAPH_FILE_SELECTOR_LOG)
  if {$path eq "" || $path eq $failedPath} {return}
  set busy 1
  set channel {}
  set code [catch {
    set data [dict create time_ms [clock milliseconds] pid [pid] session $session \
      opening $opening event $event details $details]
    if {[info exists ::fileselect(state)]} {dict set data state $::fileselect(state)}
    foreach {key script} {
      focus {focus}
      grab {grab current}
      stacking {wm stackorder .}
    } {
      if {![catch {uplevel #0 $script} value]} {dict set data $key $value}
    }
    foreach window {. .fileselect .htmessagebox} {
      dict set data windows $window exists [winfo exists $window]
      if {![winfo exists $window]} {continue}
      foreach {key command} {state {wm state} owner {wm transient} geometry {wm geometry} \
          mapped {winfo ismapped} viewable {winfo viewable}} {
        if {![catch {{*}$command $window} value]} {dict set data windows $window $key $value}
      }
    }
    set channel [open $path a]
    fconfigure $channel -encoding utf-8 -translation lf
    puts $channel [string map [list \n {\n} \r {\r}] $data]
    close $channel
    set channel {}
  } message]
  if {$channel ne ""} {catch {close $channel}}
  if {$code} {
    set failedPath $path
    catch {puts stderr "ffgraph file-selector logging failed for $path: $message"}
  }
  set busy 0
}

proc ::ffgraph_selector_debug::stateChanged {args} {
  record state-write
}

proc ::ffgraph_selector_debug::windowEvent {event window} {
  if {$window in {. .fileselect .fileselect.choose.current.e .htmessagebox}} {
    record $event $window
  }
}

proc ::ffgraph_selector_debug::command {command args} {
  variable busy
  variable opening
  variable timers
  if {$busy} {return}
  set name [namespace tail [lindex $command 0]]
  set operation [lindex $args end]
  if {$name in {wm focus grab raise tkwait}} {
    # Limit WM operations to the relevant top-level windows, and skip ordinary
    # focus/grab queries. Snapshot queries themselves are guarded by busy.
    if {$name eq "wm" && ([lindex $command 2] ni {. .fileselect .htmessagebox} \
        || [lindex $command 1] ni {withdraw deiconify iconify transient geometry minsize state})} {return}
    if {$name eq "focus" && [llength $command] < 2} {return}
    if {$name eq "grab" && [lindex $command 1] in {current status}} {return}
  } elseif {$name eq "recall"} {
    if {[lindex $command 1] ne "genEps" && \
        !([lindex $command 1] eq "modPress" && [lindex $command 2] in {l m s S})} {return}
  }
  if {$operation eq "enter"} {
    if {$name eq "FileSelectorBox"} {
      incr opening
      foreach timer $timers {after cancel $timer}
      set timers {}
      foreach delay {100 500 1500 3000} {
        lappend timers [after $delay [list ::ffgraph_selector_debug::record snapshot "after_ms=$delay"]]
      }
    }
    record call-enter $command
  } else {
    set code [lindex $args 0]
    set details [dict create command $command code $code]
    if {$code != 0} {
      dict set details result [lindex $args 1]
      if {[info exists ::errorInfo]} {dict set details errorInfo $::errorInfo}
    } elseif {[string match FileSelectorBox_GetDirContens_* $name]} {
      dict set details count [llength [lindex $args 1]]
    }
    record call-leave $details
    if {$name eq "FileSelectorBox"} {
      foreach timer $timers {after cancel $timer}
      set timers {}
    }
  }
}

proc ::ffgraph_selector_debug::enable {} {
  variable installed
  variable sourceFile
  if {$installed || ![info exists ::env(FFGRAPH_FILE_SELECTOR_LOG)] \
      || $::env(FFGRAPH_FILE_SELECTOR_LOG) eq ""} {return}
  set installed 1
  foreach name {::FileSelectorBox ::FileSelectorBox_GetArgs ::FileSelectorBox_MakeItems
      ::FileSelectorBox_GetDirStruct ::FileSelectorBox_GetDirContens_Dir
      ::FileSelectorBox_GetDirContens_File ::FileSelectorBox_Rescan
      ::FileSelectorBox_MessageBox wm focus grab raise tkwait recall} {
    if {[llength [info commands $name]]} {
      trace add execution $name {enter leave} ::ffgraph_selector_debug::command
    }
  }
  trace add variable ::fileselect(state) write ::ffgraph_selector_debug::stateChanged
  foreach event {Map Unmap Visibility Configure FocusIn FocusOut Destroy} {
    bind all <$event> +[list ::ffgraph_selector_debug::windowEvent $event %W]
  }
  record enabled [dict create source $sourceFile source_mtime [file mtime $sourceFile] \
    os $::tcl_platform(os) os_version $::tcl_platform(osVersion) \
    tcl [info patchlevel] tk $::tk_patchLevel windowing [tk windowingsystem] theme [ttk::style theme use]]
}

# Installing hooks and writing logs are both optional and must not affect startup.
catch {::ffgraph_selector_debug::enable}
