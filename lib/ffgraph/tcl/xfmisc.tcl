#!/public/bin/pixmap-wish -f
# Program: sugi
# Tcl version: 7.3 (Tcl/Tk/XF)
# Tk version: 3.6
# XF version: 2.3
#

# module inclusion
global env
global xfLoadPath
global xfLoadInfo
set xfLoadInfo 0
if {[info exists env(XF_LOAD_PATH)]} {
  if {[string first $env(XF_LOAD_PATH) /usr/local/lib/] == -1} {
    set xfLoadPath $env(XF_LOAD_PATH):/usr/local/lib/
  } {
    set xfLoadPath /usr/local/lib/
  }
} {
  set xfLoadPath /usr/local/lib/
}

global argc
global argv
global tk_version
set tmpArgv ""
for {set counter 0} {$counter < $argc} {incr counter 1} {
  case [string tolower [lindex $argv $counter]] in {
    {-xfloadpath} {
      incr counter 1
      set xfLoadPath "[lindex $argv $counter]:$xfLoadPath"
    }
    {-xfstartup} {
      incr counter 1
      source [lindex $argv $counter]
    }
    {-xfbindfile} {
      incr counter 1
      set env(XF_BIND_FILE) "[lindex $argv $counter]"
    }
    {-xfcolorfile} {
      incr counter 1
      set env(XF_COLOR_FILE) "[lindex $argv $counter]"
    }
    {-xfcursorfile} {
      incr counter 1
      set env(XF_CURSOR_FILE) "[lindex $argv $counter]"
    }
    {-xffontfile} {
      incr counter 1
      set env(XF_FONT_FILE) "[lindex $argv $counter]"
    }
    {-xfmodelmono} {
      if {$tk_version >= 3.0} {
        tk colormodel . monochrome
      }
    }
    {-xfmodelcolor} {
      if {$tk_version >= 3.0} {
        tk colormodel . color
      }
    }
    {-xfloading} {
      set xfLoadInfo 1
    }
    {-xfnoloading} {
      set xfLoadInfo 0
    }
    {default} {
      lappend tmpArgv [lindex $argv $counter]
    }
  }
}
set argv $tmpArgv
set argc [llength $tmpArgv]
unset counter
unset tmpArgv

# User defined procedures

proc ffMax { a b } {
  if { $a < $b } {
    return $b
  } 
  return $a
}

proc ffMin { a b } {
  if { $a > $b } {
    return $b
  } 
  return $a
}

# Internal procedures

proc ffSetColorsForWin {w} {
  ##############
  # Procedure  : setColorsForWin
  # Description: sets the colors for the window "w" and all its child-windows
  # Arguments  : w: window
  # Returns    : none
  # Sideeffects: the author is not familiar to any strange sideeffects 
  #              (shit happens)
  # Author     : Achim Dannecker
  ##############
  
  $w configure -bg #dddddd
  set tmplist [winfo children $w]
  while {[llength $tmplist] != 0} {
    switch [winfo class [lindex $tmplist 0]] {
      Listbox {[lindex $tmplist 0] configure -bg #ebebeb\
		 -highlightthickness 0 \
		 -borderwidth 2 -relief ridge -selectmode browse
      }
      Scrollbar {[lindex $tmplist 0] configure -bg gold \
		   -activebackground #c6c6c6 \
		   -highlightthickness 0 \
		   -borderwidth 1 -relief ridge
      }
      Entry {[lindex $tmplist 0] configure -bg #ebebeb \
	       -highlightthickness 0 -bd 1
      }
      Checkbutton {[lindex $tmplist 0] configure -bg #ebebeb \
		     -activebackground #c6c6c6 \
		     -highlightthickness 0 \
		     -relief raised -bd 3 \
		     -padx 5 -pady 1
	bind [lindex $tmplist 0] <1> {
	  %W configure -relief sunken
	}
	bind [lindex $tmplist 0] <ButtonRelease> {
	  %W configure -relief raised
	}
      }
      Button {[lindex $tmplist 0] configure -bg #ebebeb \
		-activebackground #c6c6c6 \
		-highlightthickness 0 -relief raised -bd 2
      }
      Scale {[lindex $tmplist 0] configure -borderwidth 1 -relief ridge \
        -bg #ebebeb -fg #c6c6c6 -activebackground #c6c6c6 -relief raised -bd 1
      }
      default {[lindex $tmplist 0] configure -bg #ebebeb
      }
    }
    set tmplist [concat $tmplist [winfo children [lindex $tmplist 0]]]
    set tmplist [lreplace $tmplist 0 0]
  }
}


# Procedure: Alias
if {"[info procs Alias]" == ""} {
proc Alias { args} {
# xf ignore me 7
##########
# Procedure: Alias
# Description: establish an alias for a procedure
# Arguments: args - no argument means that a list of all aliases
#                   is returned. Otherwise the first parameter is
#                   the alias name, and the second parameter is
#                   the procedure that is aliased.
# Returns: nothing, the command that is bound to the alias or a
#          list of all aliases - command pairs. 
# Sideeffects: internalAliasList is updated, and the alias
#              proc is inserted
##########
  global internalAliasList

  if {[llength $args] == 0} {
    return $internalAliasList
  } {
    if {[llength $args] == 1} {
      set xfTmpIndex [lsearch $internalAliasList "[lindex $args 0] *"]
      if {$xfTmpIndex != -1} {
        return [lindex [lindex $internalAliasList $xfTmpIndex] 1]
      }
    } {
      if {[llength $args] == 2} {
        eval "proc [lindex $args 0] {args} {#xf ignore me 4
return \[eval \"[lindex $args 1] \$args\"\]}"
        set xfTmpIndex [lsearch $internalAliasList "[lindex $args 0] *"]
        if {$xfTmpIndex != -1} {
          set internalAliasList [lreplace $internalAliasList $xfTmpIndex $xfTmpIndex "[lindex $args 0] [lindex $args 1]"]
        } {
          lappend internalAliasList "[lindex $args 0] [lindex $args 1]"
        }
      } {
        error "Alias: wrong number or args: $args"
      }
    }
  }
}
}


# Procedure: GetSelection
if {"[info procs GetSelection]" == ""} {
proc GetSelection {} {
# xf ignore me 7
##########
# Procedure: GetSelection
# Description: get current selection
# Arguments: none
# Returns: none
# Sideeffects: none
##########

  # the save way
  set xfSelection ""
  catch "selection get" xfSelection
  if {"$xfSelection" == "selection doesn't exist or form \"STRING\" not defined"} {
    return ""
  } {
    return $xfSelection
  }
}
}


# Procedure: MenuPopupAdd
if {"[info procs MenuPopupAdd]" == ""} {
proc MenuPopupAdd { xfW xfButton xfMenu {xfModifier ""} {xfCanvasTag ""}} {
# xf ignore me 7
# the popup menu handling is from (I already gave up with popup handling :-):
#
# Copyright 1991,1992 by James Noble.
# Everyone is granted permission to copy, modify and redistribute.
# This notice must be preserved on all copies or derivates.
#
##########
# Procedure: MenuPopupAdd
# Description: attach a popup menu to widget
# Arguments: xfW - the widget
#            xfButton - the button we use
#            xfMenu - the menu to attach
#            {xfModifier} - a optional modifier
#            {xfCanvasTag} - a canvas tagOrId
# Returns: none
# Sideeffects: none
##########
  global tk_popupPriv

  set tk_popupPriv($xfMenu,focus) ""
  set tk_popupPriv($xfMenu,grab) ""
  if {"$xfModifier" != ""} {
    set press "$xfModifier-"
    set motion "$xfModifier-"
    set release "Any-"
  } {
    set press ""
    set motion ""
    set release ""
  }

  bind $xfMenu "<${motion}B${xfButton}-Motion>"  "MenuPopupMotion $xfMenu %W %X %Y"
  bind $xfMenu "<${release}ButtonRelease-${xfButton}>"  "MenuPopupRelease $xfMenu %W"
  if {"$xfCanvasTag" == ""} {
    bind $xfW "<${press}ButtonPress-${xfButton}>"  "MenuPopupPost $xfMenu %X %Y"
    bind $xfW "<${release}ButtonRelease-${xfButton}>"  "MenuPopupRelease $xfMenu %W"
  } {
    $xfW bind $xfCanvasTag "<${press}ButtonPress-${xfButton}>"  "MenuPopupPost $xfMenu %X %Y"
    $xfW bind $xfCanvasTag "<${release}ButtonRelease-${xfButton}>"  "MenuPopupRelease $xfMenu %W"
  }
}
}


# Procedure: MenuPopupMotion
if {"[info procs MenuPopupMotion]" == ""} {
proc MenuPopupMotion { xfMenu xfW xfX xfY} {
# xf ignore me 7
##########
# Procedure: MenuPopupMotion
# Description: handle the popup menu motion
# Arguments: xfMenu - the topmost menu
#            xfW - the menu
#            xfX - the root x coordinate
#            xfY - the root x coordinate
# Returns: none
# Sideeffects: none
##########
  global tk_popupPriv

  if {"[info commands $xfW]" != "" && [winfo ismapped $xfW] &&
      "[winfo class $xfW]" == "Menu" &&
      [info exists tk_popupPriv($xfMenu,focus)] &&
      "$tk_popupPriv($xfMenu,focus)" != "" &&
      [info exists tk_popupPriv($xfMenu,grab)] &&
      "$tk_popupPriv($xfMenu,grab)" != ""} {
    set xfPopMinX [winfo rootx $xfW]
    set xfPopMaxX [expr $xfPopMinX+[winfo width $xfW]]
    if {$xfX >= $xfPopMinX && $xfX <= $xfPopMaxX} {
      $xfW activate @[expr $xfY-[winfo rooty $xfW]]
      if {![catch "$xfW entryconfig @[expr $xfY-[winfo rooty $xfW]] -menu" result]} {
        if {"[lindex $result 4]" != ""} {
          foreach binding [bind $xfMenu] {
            bind [lindex $result 4] $binding [bind $xfMenu $binding]
          }
        }
      }
    } {
      $xfW activate none
    }
  }
}
}


# Procedure: MenuPopupPost
if {"[info procs MenuPopupPost]" == ""} {
proc MenuPopupPost { xfMenu xfX xfY} {
# xf ignore me 7
##########
# Procedure: MenuPopupPost
# Description: post the popup menu
# Arguments: xfMenu - the menu
#            xfX - the root x coordinate
#            xfY - the root x coordinate
# Returns: none
# Sideeffects: none
##########
  global tk_popupPriv

  if {"[info commands $xfMenu]" != ""} {
    if {![info exists tk_popupPriv($xfMenu,focus)]} {
      set tk_popupPriv($xfMenu,focus) [focus]
    } {
      if {"$tk_popupPriv($xfMenu,focus)" == ""} {
        set tk_popupPriv($xfMenu,focus) [focus]
      }
    }
    set tk_popupPriv($xfMenu,grab) $xfMenu

    catch "$xfMenu activate none"
    catch "$xfMenu post $xfX $xfY"
    catch "focus $xfMenu"
    catch "grab -global $xfMenu"
  }
}
}


# Procedure: MenuPopupRelease
if {"[info procs MenuPopupRelease]" == ""} {
proc MenuPopupRelease { xfMenu xfW} {
# xf ignore me 7
##########
# Procedure: MenuPopupRelease
# Description: remove the popup menu
# Arguments: xfMenu - the topmost menu widget
#            xfW - the menu widget
# Returns: none
# Sideeffects: none
##########
  global tk_popupPriv
  global tk_version

  if {"[info commands $xfW]" != "" && [winfo ismapped $xfW] &&
      "[winfo class $xfW]" == "Menu" &&
      [info exists tk_popupPriv($xfMenu,focus)] &&
      "$tk_popupPriv($xfMenu,focus)" != "" &&
      [info exists tk_popupPriv($xfMenu,grab)] &&
      "$tk_popupPriv($xfMenu,grab)" != ""} {
    if {$tk_version >= 3.0} {
      catch "grab release $tk_popupPriv($xfMenu,grab)"
    } {
      catch "grab none"
    }
    catch "focus $tk_popupPriv($xfMenu,focus)"
    set tk_popupPriv($xfMenu,focus) ""
    set tk_popupPriv($xfMenu,grab) ""
    if {"[$xfW index active]" != "none"} {
      $xfW invoke active; catch "$xfMenu unpost"
    }
  }
  catch "$xfMenu unpost"
}
}

# Procedure: MenuPopupHandle
if {"[info procs MenuPopupHandle]" == ""} {
proc MenuPopupHandle { xfMenu xfW xfX xfY} {
# xf ignore me 7
##########
# Procedure: MenuPopupHandle
# Description: handle the popup menus
# Arguments: xfMenu - the menu to attach
#            xfW - the widget
#            xfX - the root x coordinate
#            xfY - the root x coordinate
# Returns: none
# Sideeffects: none
##########

  if {"[info commands $xfMenu]" != "" && [winfo ismapped $xfMenu]} {
    set xfPopMinX [winfo rootx $xfMenu]
    set xfPopMaxX [expr $xfPopMinX+[winfo width $xfMenu]]
    if {($xfX >= $xfPopMinX) &&  ($xfX <= $xfPopMaxX)} {
      $xfMenu activate @[expr $xfY-[winfo rooty $xfMenu]]
    } {
      $xfMenu activate none
    }
  }
}
}

# Procedure: NoFunction
if {"[info procs NoFunction]" == ""} {
proc NoFunction { args} {
# xf ignore me 7
##########
# Procedure: NoFunction
# Description: do nothing (especially with scales and scrollbars)
# Arguments: args - a number of ignored parameters
# Returns: none
# Sideeffects: none
##########
}
}


# Procedure: SN
if {"[info procs SN]" == ""} {
proc SN { {xfName ""}} {
# xf ignore me 7
##########
# Procedure: SN
# Description: map a symbolic name to the widget path
# Arguments: xfName
# Returns: the symbolic name
# Sideeffects: none
##########

  SymbolicName $xfName
}
}


# Procedure: SymbolicName
if {"[info procs SymbolicName]" == ""} {
proc SymbolicName { {xfName ""}} {
# xf ignore me 7
##########
# Procedure: SymbolicName
# Description: map a symbolic name to the widget path
# Arguments: xfName
# Returns: the symbolic name
# Sideeffects: none
##########

  global symbolicName

  if {"$xfName" != ""} {
    set xfArrayName ""
    append xfArrayName symbolicName ( $xfName )
    if {![catch "set \"$xfArrayName\"" xfValue]} {
      return $xfValue
    } {
      if {"[info commands XFProcError]" != ""} {
        XFProcError "Unknown symbolic name:\n$xfName"
      } {
        puts stderr "XF error: unknown symbolic name:\n$xfName"
      }
    }
  }
  return ""
}
}


# Procedure: Unalias
if {"[info procs Unalias]" == ""} {
proc Unalias { aliasName} {
# xf ignore me 7
##########
# Procedure: Unalias
# Description: remove an alias for a procedure
# Arguments: aliasName - the alias name to remove
# Returns: none
# Sideeffects: internalAliasList is updated, and the alias
#              proc is removed
##########
  global internalAliasList

  set xfIndex [lsearch $internalAliasList "$aliasName *"]
  if {$xfIndex != -1} {
    rename $aliasName ""
    set internalAliasList [lreplace $internalAliasList $xfIndex $xfIndex]
  }
}
}



# application parsing procedure
proc XFLocalParseAppDefs {xfAppDefFile} {
  global xfAppDefaults

  # basically from: Michael Moore
  if {[file exists $xfAppDefFile] &&
      [file readable $xfAppDefFile] &&
      "[file type $xfAppDefFile]" == "link"} {
    catch "file type $xfAppDefFile" xfType
    while {"$xfType" == "link"} {
      if {[catch "file readlink $xfAppDefFile" xfAppDefFile]} {
        return
      }
      catch "file type $xfAppDefFile" xfType
    }
  }
  if {!("$xfAppDefFile" != "" &&
        [file exists $xfAppDefFile] &&
        [file readable $xfAppDefFile] &&
        "[file type $xfAppDefFile]" == "file")} {
    return
  }
  if {![catch "open $xfAppDefFile r" xfResult]} {
    set xfAppFileContents [read $xfResult]
    close $xfResult
    foreach line [split $xfAppFileContents "\n"] {
      # backup indicates how far to backup.  It applies to the
      # situation where a resource name ends in . and when it
      # ends in *.  In the second case you want to keep the *
      # in the widget name for pattern matching, but you want
      # to get rid of the . if it is the end of the name. 
      set backup -2  
      set line [string trim $line]
      if {[string index $line 0] == "#" || "$line" == ""} {
        # skip comments and empty lines
        continue
      }
      set list [split $line ":"]
      set resource [string trim [lindex $list 0]]
      set i [string last "." $resource]
      set j [string last "*" $resource]
      if {$j > $i} { 
        set i $j
        set backup -1
      }
      incr i
      set name [string range $resource $i end]
      incr i $backup
      set widname [string range $resource 0 $i]
      set value [string trim [lindex $list 1]]
      if {"$widname" != "" && "$widname" != "*"} {
        # insert the widget and resourcename to the application
        # defaults list.
        if {![info exists xfAppDefaults]} {
          set xfAppDefaults ""
        }
        lappend xfAppDefaults [list $widname [string tolower $name] $value]
      }
    }
  }
}

# User defined procedures

proc InputOneBox { {boxMessage "Please enter"} {boxTitle "Input"} } {
  global inputOneBox

#init
  catch {destroy $inputOneBox(toplevelName)}

  toplevel $inputOneBox(toplevelName) -borderwidth 0

  wm geometry $inputOneBox(toplevelName) 250x60
  wm title $inputOneBox(toplevelName) $boxTitle
  wm maxsize $inputOneBox(toplevelName) 1000 60
  wm minsize $inputOneBox(toplevelName) 100 60

  message $inputOneBox(toplevelName).message1 -relief ridge \
    -aspect 10000 -padx 0 -pady 0 -text "$boxMessage"

  entry $inputOneBox(toplevelName).entry1 -relief ridge 
  bind $inputOneBox(toplevelName).entry1 <Return> {
    global inputOneBox
    set inputOneBox(entry) [$inputOneBox(toplevelName).entry1 get]
    destroy $inputOneBox(toplevelName)
  }

  pack append $inputOneBox(toplevelName) \
    $inputOneBox(toplevelName).message1 {top expand fillx filly} \
    $inputOneBox(toplevelName).entry1 {bottom fillx}
  
  focus $inputOneBox(toplevelName).entry1
  # wait for the box to be destroyed
  update idletask
  # grab $inputOneBox(toplevelName)
  tkwait window $inputOneBox(toplevelName)

  return $inputOneBox(entry)
}

# Procedure: AlertBox
proc AlertBox { {alertBoxMessage "Alert message"} {alertBoxCommand ""} {alertBoxGeometry "350x150"} {alertBoxTitle "Alert box"} args} {
# xf ignore me 5
##########
# Procedure: AlertBox
# Description: show alert box
# Arguments: {alertBoxMessage} - the text to display
#            {alertBoxCommand} - the command to call after ok
#            {alertBoxGeometry} - the geometry for the window
#            {alertBoxTitle} - the title for the window
#            {args} - labels of buttons
# Returns: The number of the selected button, ot nothing
# Sideeffects: none
# Notes: there exist also functions called:
#          AlertBoxFile - to open and read a file automatically
#          AlertBoxFd - to read from an already opened filedescriptor
##########
#
# global alertBox(activeBackground) - active background color
# global alertBox(activeForeground) - active foreground color
# global alertBox(after) - destroy alert box after n seconds
# global alertBox(anchor) - anchor for message box
# global alertBox(background) - background color
# global alertBox(font) - message font
# global alertBox(foreground) - foreground color
# global alertBox(justify) - justify for message box
# global alertBox(toplevelName) - the toplevel name

  global alertBox

  # show alert box
  if {[llength $args] > 0} {
    eval AlertBoxInternal "\{$alertBoxMessage\}" "\{$alertBoxCommand\}" "\{$alertBoxGeometry\}" "\{$alertBoxTitle\}" $args
  } {
    AlertBoxInternal $alertBoxMessage $alertBoxCommand $alertBoxGeometry $alertBoxTitle
  }

  if {[llength $args] > 0} {
    # wait for the box to be destroyed
    update idletask
    # grab $alertBox(toplevelName)
    tkwait window $alertBox(toplevelName)

    return $alertBox(button)
  }
}


# Procedure: AlertBoxFd
proc AlertBoxFd { {alertBoxInFile ""} {alertBoxCommand ""} {alertBoxGeometry "350x150"} {alertBoxTitle "Alert box"} args} {
# xf ignore me 5
##########
# Procedure: AlertBoxFd
# Description: show alert box containing a filedescriptor
# Arguments: {alertBoxInFile} - a filedescriptor to read. The descriptor
#                               is closed after reading
#            {alertBoxCommand} - the command to call after ok
#            {alertBoxGeometry} - the geometry for the window
#            {alertBoxTitle} - the title for the window
#            {args} - labels of buttons
# Returns: The number of the selected button, ot nothing
# Sideeffects: none
# Notes: there exist also functions called:
#          AlertBox - to display a passed string
#          AlertBoxFile - to open and read a file automatically
##########
#
# global alertBox(activeBackground) - active background color
# global alertBox(activeForeground) - active foreground color
# global alertBox(after) - destroy alert box after n seconds
# global alertBox(anchor) - anchor for message box
# global alertBox(background) - background color
# global alertBox(font) - message font
# global alertBox(foreground) - foreground color
# global alertBox(justify) - justify for message box
# global alertBox(toplevelName) - the toplevel name

  global alertBox

  # check file existance
  if {"$alertBoxInFile" == ""} {
    puts stderr "No filedescriptor specified"
    return
  }

  set alertBoxMessage [read $alertBoxInFile]
  close $alertBoxInFile

  # show alert box
  if {[llength $args] > 0} {
    eval AlertBoxInternal "\{$alertBoxMessage\}" "\{$alertBoxCommand\}" "\{$alertBoxGeometry\}" "\{$alertBoxTitle\}" $args
  } {
    AlertBoxInternal $alertBoxMessage $alertBoxCommand $alertBoxGeometry $alertBoxTitle
  }

  if {[llength $args] > 0} {
    # wait for the box to be destroyed
    update idletask
    # grab $alertBox(toplevelName)
    tkwait window $alertBox(toplevelName)

    return $alertBox(button)
  }
}


# Procedure: AlertBoxFile
proc AlertBoxFile { {alertBoxFile ""} {alertBoxCommand ""} {alertBoxGeometry "350x150"} {alertBoxTitle "Alert box"} args} {
# xf ignore me 5
##########
# Procedure: AlertBoxFile
# Description: show alert box containing a file
# Arguments: {alertBoxFile} - filename to read
#            {alertBoxCommand} - the command to call after ok
#            {alertBoxGeometry} - the geometry for the window
#            {alertBoxTitle} - the title for the window
#            {args} - labels of buttons
# Returns: The number of the selected button, ot nothing
# Sideeffects: none
# Notes: there exist also functions called:
#          AlertBox - to display a passed string
#          AlertBoxFd - to read from an already opened filedescriptor
##########
#
# global alertBox(activeBackground) - active background color
# global alertBox(activeForeground) - active foreground color
# global alertBox(after) - destroy alert box after n seconds
# global alertBox(anchor) - anchor for message box
# global alertBox(background) - background color
# global alertBox(font) - message font
# global alertBox(foreground) - foreground color
# global alertBox(justify) - justify for message box
# global alertBox(toplevelName) - the toplevel name

  global alertBox

  # check file existance
  if {"$alertBoxFile" == ""} {
    puts stderr "No filename specified"
    return
  }

  if {[catch "open $alertBoxFile r" alertBoxInFile]} {
    puts stderr "$alertBoxInFile"
    return
  }

  set alertBoxMessage [read $alertBoxInFile]
  close $alertBoxInFile

  # show alert box
  if {[llength $args] > 0} {
    eval AlertBoxInternal "\{$alertBoxMessage\}" "\{$alertBoxCommand\}" "\{$alertBoxGeometry\}" "\{$alertBoxTitle\}" $args
  } {
    AlertBoxInternal $alertBoxMessage $alertBoxCommand $alertBoxGeometry $alertBoxTitle
  }

  if {[llength $args] > 0} {
    # wait for the box to be destroyed
    update idletask
    # grab $alertBox(toplevelName)
    tkwait window $alertBox(toplevelName)

    return $alertBox(button)
  }
}


# Procedure: AlertBoxInternal
proc AlertBoxInternal { alertBoxMessage alertBoxCommand alertBoxGeometry alertBoxTitle args} {
# xf ignore me 6
  global alertBox

  set tmpButtonOpt ""
  set tmpFrameOpt ""
  set tmpMessageOpt ""
  if {"$alertBox(activeBackground)" != ""} {
    append tmpButtonOpt "-activebackground \"$alertBox(activeBackground)\" "
  }
  if {"$alertBox(activeForeground)" != ""} {
    append tmpButtonOpt "-activeforeground \"$alertBox(activeForeground)\" "
  }
  if {"$alertBox(background)" != ""} {
    append tmpButtonOpt "-background \"$alertBox(background)\" "
    append tmpFrameOpt "-background \"$alertBox(background)\" "
    append tmpMessageOpt "-background \"$alertBox(background)\" "
  }
  if {"$alertBox(font)" != ""} {
    append tmpButtonOpt "-font \"$alertBox(font)\" "
    append tmpMessageOpt "-font \"$alertBox(font)\" "
  }
  if {"$alertBox(foreground)" != ""} {
    append tmpButtonOpt "-foreground \"$alertBox(foreground)\" "
    append tmpMessageOpt "-foreground \"$alertBox(foreground)\" "
  }

  # start build of toplevel
  if {"[info commands XFDestroy]" != ""} {
    catch {XFDestroy $alertBox(toplevelName)}
  } {
    catch {destroy $alertBox(toplevelName)}
  }
  toplevel $alertBox(toplevelName)     -borderwidth 0
  catch "$alertBox(toplevelName) config $tmpFrameOpt"
  if {[catch "wm geometry $alertBox(toplevelName) $alertBoxGeometry"]} {
    wm geometry $alertBox(toplevelName) 350x150
  }
  wm title $alertBox(toplevelName) $alertBoxTitle
  wm maxsize $alertBox(toplevelName) 1000 1000
  wm minsize $alertBox(toplevelName) 100 100
  # end build of toplevel

  message $alertBox(toplevelName).message1     -anchor "$alertBox(anchor)"     -justify "$alertBox(justify)"     -relief raised     -text "$alertBoxMessage"
  catch "$alertBox(toplevelName).message1 config $tmpMessageOpt"

  set xfTmpWidth     [string range $alertBoxGeometry 0 [expr [string first x $alertBoxGeometry]-1]]
  if {"$xfTmpWidth" != ""} {
    # set message size
    catch "$alertBox(toplevelName).message1 configure       -width [expr $xfTmpWidth-10]"
  } {
    $alertBox(toplevelName).message1 configure       -aspect 1500
  }

  frame $alertBox(toplevelName).frame1     -borderwidth 0     -relief raised
  catch "$alertBox(toplevelName).frame1 config $tmpFrameOpt"

  set alertBoxCounter 0
  set buttonNum [llength $args]
  if {$buttonNum > 0} {
    while {$alertBoxCounter < $buttonNum} {
      button $alertBox(toplevelName).frame1.button$alertBoxCounter         -text "[lindex $args $alertBoxCounter]"         -command "
          global alertBox
          set alertBox(button) $alertBoxCounter
          if {\"\[info commands XFDestroy\]\" != \"\"} {
            catch {XFDestroy $alertBox(toplevelName)}
          } {
            catch {destroy $alertBox(toplevelName)}
          }"
      catch "$alertBox(toplevelName).frame1.button$alertBoxCounter config $tmpButtonOpt"

      pack append $alertBox(toplevelName).frame1                   $alertBox(toplevelName).frame1.button$alertBoxCounter {left fillx expand}

      incr alertBoxCounter
    }
  } {
    button $alertBox(toplevelName).frame1.button0       -text "OK"       -command "
        global alertBox
        set alertBox(button) 0
        if {\"\[info commands XFDestroy\]\" != \"\"} {
          catch {XFDestroy $alertBox(toplevelName)}
        } {
          catch {destroy $alertBox(toplevelName)}
        }
        $alertBoxCommand"
    catch "$alertBox(toplevelName).frame1.button0 config $tmpButtonOpt"

    pack append $alertBox(toplevelName).frame1                 $alertBox(toplevelName).frame1.button0 {left fillx expand}
  }

  # packing
  pack append $alertBox(toplevelName)               $alertBox(toplevelName).frame1 {bottom fill}               $alertBox(toplevelName).message1 {top fill expand}

  if {$alertBox(after) != 0} {
    after [expr $alertBox(after)*1000]       "catch \"$alertBox(toplevelName).frame1.button0 invoke\""
  }
}


# # Procedure: FSBox
# proc FSBox { {fsBoxMessage "Select file:"} {fsBoxFileName ""} {fsBoxActionOk ""} {fsBoxActionCancel ""}} {
# # xf ignore me 5
# ##########
# # Procedure: FSBox
# # Description: show file selector box
# # Arguments: fsBoxMessage - the text to display
# #            fsBoxFileName - a file name that should be selected
# #            fsBoxActionOk - the action that should be performed on ok
# #            fsBoxActionCancel - the action that should be performed on cancel
# # Returns: the filename that was selected, or nothing
# # Sideeffects: none
# ##########
# # 
# # global fsBox(activeBackground) - active background color
# # global fsBox(activeForeground) - active foreground color
# # global fsBox(background) - background color
# # global fsBox(font) - text font
# # global fsBox(foreground) - foreground color
# # global fsBox(extensions) - scan directory for extensions
# # global fsBox(scrollActiveForeground) - scrollbar active background color
# # global fsBox(scrollBackground) - scrollbar background color
# # global fsBox(scrollForeground) - scrollbar foreground color
# # global fsBox(scrollSide) - side where scrollbar is located

#   global fsBox

#   set tmpButtonOpt ""
#   set tmpFrameOpt ""
#   set tmpMessageOpt ""
#   set tmpScaleOpt ""
#   set tmpScrollOpt ""
#   if {"$fsBox(activeBackground)" != ""} {
#     append tmpButtonOpt "-activebackground \"$fsBox(activeBackground)\" "
#   }
#   if {"$fsBox(activeForeground)" != ""} {
#     append tmpButtonOpt "-activeforeground \"$fsBox(activeForeground)\" "
#   }
#   if {"$fsBox(background)" != ""} {
#     append tmpButtonOpt "-background \"$fsBox(background)\" "
#     append tmpFrameOpt "-background \"$fsBox(background)\" "
#     append tmpMessageOpt "-background \"$fsBox(background)\" "
#   }
#   if {"$fsBox(font)" != ""} {
#     append tmpButtonOpt "-font \"$fsBox(font)\" "
#     append tmpMessageOpt "-font \"$fsBox(font)\" "
#   }
#   if {"$fsBox(foreground)" != ""} {
#     append tmpButtonOpt "-foreground \"$fsBox(foreground)\" "
#     append tmpMessageOpt "-foreground \"$fsBox(foreground)\" "
#   }
#   if {"$fsBox(scrollActiveForeground)" != ""} {
#     append tmpScrollOpt "-activeforeground \"$fsBox(scrollActiveForeground)\" "
#   }
#   if {"$fsBox(scrollBackground)" != ""} {
#     append tmpScrollOpt "-background \"$fsBox(scrollBackground)\" "
#   }
#   if {"$fsBox(scrollForeground)" != ""} {
#     append tmpScrollOpt "-foreground \"$fsBox(scrollForeground)\" "
#   }

#   if {[file exists [file tail $fsBoxFileName]] &&
#       [IsAFile [file tail $fsBoxFileName]]} {
#     set fsBox(name) [file tail $fsBoxFileName]
#   } {
#     set fsBox(name) ""
#   }
#   if {[file exists $fsBoxFileName] && [IsADir $fsBoxFileName]} {
#     set fsBox(path) $fsBoxFileName
#   } {
#     if {"[file rootname $fsBoxFileName]" != "."} {
#       set fsBox(path) [file rootname $fsBoxFileName]
#     }
#   }
#   if {$fsBox(showPixmap)} {
#     set fsBox(path) [string trimleft $fsBox(path) @]
#   }
#   if {"$fsBox(path)" != "" && [file exists $fsBox(path)] &&
#       [IsADir $fsBox(path)]} {
#     set fsBox(internalPath) $fsBox(path)
#   } {
#     if {"$fsBox(internalPath)" == "" ||
#         ![file exists $fsBox(internalPath)]} {
#       set fsBox(internalPath) [pwd]
#     }
#   }
#   # build widget structure

#   # start build of toplevel
#   if {"[info commands XFDestroy]" != ""} {
#     catch {XFDestroy .fsBox}
#   } {
#     catch {destroy .fsBox}
#   }
#   toplevel .fsBox     -borderwidth 0
#   catch ".fsBox config $tmpFrameOpt"
#   wm geometry .fsBox 350x300 
#   wm title .fsBox {File select box}
#   wm maxsize .fsBox 1000 1000
#   wm minsize .fsBox 100 100
#   # end build of toplevel

#   label .fsBox.message1     -anchor c     -relief raised     -text "$fsBoxMessage"
#   catch ".fsBox.message1 config $tmpMessageOpt"

#   frame .fsBox.frame1     -borderwidth 0     -relief raised
#   catch ".fsBox.frame1 config $tmpFrameOpt"

#   button .fsBox.frame1.ok     -text "OK"     -command "
#       global fsBox
#       set fsBox(name) \[.fsBox.file.file get\]
#       if {$fsBox(showPixmap)} {
#         set fsBox(path) @\[.fsBox.path.path get\]
#       } {
#         set fsBox(path) \[.fsBox.path.path get\]
#       }
#       set fsBox(internalPath) \[.fsBox.path.path get\]
#       $fsBoxActionOk
#       if {\"\[info commands XFDestroy\]\" != \"\"} {
#         catch {XFDestroy .fsBox}
#       } {
#         catch {destroy .fsBox}
#       }"
#   catch ".fsBox.frame1.ok config $tmpButtonOpt"

#   button .fsBox.frame1.rescan     -text "Rescan"     -command {
#       global fsBox
#       FSBoxFSShow [.fsBox.path.path get]         [.fsBox.pattern.pattern get] $fsBox(all)}
#   catch ".fsBox.frame1.rescan config $tmpButtonOpt"

#   button .fsBox.frame1.cancel     -text "Cancel"     -command "
#       global fsBox
#       set fsBox(name) {}
#       set fsBox(path) {}
#       $fsBoxActionCancel
#       if {\"\[info commands XFDestroy\]\" != \"\"} {
#         catch {XFDestroy .fsBox}
#       } {
#         catch {destroy .fsBox}
#       }"
#   catch ".fsBox.frame1.cancel config $tmpButtonOpt"

#   if {$fsBox(showPixmap)} {
#     frame .fsBox.frame2       -borderwidth 0       -relief raised
#     catch ".fsBox.frame2 config $tmpFrameOpt"

#     scrollbar .fsBox.frame2.scrollbar3       -command {.fsBox.frame2.canvas2 xview}       -orient {horizontal}       -relief {raised}
#     catch ".fsBox.frame2.scrollbar3 config $tmpScrollOpt"

#     scrollbar .fsBox.frame2.scrollbar1       -command {.fsBox.frame2.canvas2 yview}       -relief {raised}
#     catch ".fsBox.frame2.scrollbar1 config $tmpScrollOpt"

#     canvas .fsBox.frame2.canvas2       -confine {true}       -relief {raised}       -scrollregion {0c 0c 20c 20c}       -width {100}       -xscrollcommand {.fsBox.frame2.scrollbar3 set}       -yscrollcommand {.fsBox.frame2.scrollbar1 set}
#     catch ".fsBox.frame2.canvas2 config $tmpFrameOpt"

#     .fsBox.frame2.canvas2 addtag currentBitmap withtag [.fsBox.frame2.canvas2 create bitmap 5 5 -anchor nw]
#   }

#   frame .fsBox.path     -borderwidth 0     -relief raised
#   catch ".fsBox.path config $tmpFrameOpt"

#   frame .fsBox.path.paths     -borderwidth 2     -relief raised
#   catch ".fsBox.path.paths config $tmpFrameOpt"

#   menubutton .fsBox.path.paths.paths     -borderwidth 0     -menu ".fsBox.path.paths.paths.menu"     -relief flat     -text "Pathname:"
#   catch ".fsBox.path.paths.paths config $tmpButtonOpt"

#   menu .fsBox.path.paths.paths.menu
#   catch ".fsBox.path.paths.paths.menu config $tmpButtonOpt"

#   .fsBox.path.paths.paths.menu add command      -label "[string trimright $fsBox(internalPath) {/@}]"      -command "
#        global fsBox
#        FSBoxFSShow \[.fsBox.path.path get\]          \[.fsBox.pattern.pattern get\] \$fsBox(all)
#        .fsBox.path.path delete 0 end
#        .fsBox.path.path insert 0 [string trimright $fsBox(internalPath) {/@}]"

#   entry .fsBox.path.path     -relief raised
#   catch ".fsBox.path.path config $tmpMessageOpt"

#   if {![IsADir $fsBox(internalPath)]} {
#     set $fsBox(internalPath) [pwd]
#   }
#   .fsBox.path.path insert 0 $fsBox(internalPath)

#   frame .fsBox.pattern     -borderwidth 0     -relief raised
#   catch ".fsBox.pattern config $tmpFrameOpt"

#   frame .fsBox.pattern.patterns     -borderwidth 2     -relief raised
#   catch ".fsBox.pattern.patterns config $tmpFrameOpt"

#   menubutton .fsBox.pattern.patterns.patterns     -borderwidth 0     -menu ".fsBox.pattern.patterns.patterns.menu"     -relief flat     -text "Selection pattern:"
#   catch ".fsBox.pattern.patterns.patterns config $tmpButtonOpt"

#   menu .fsBox.pattern.patterns.patterns.menu
#   catch ".fsBox.pattern.patterns.patterns.menu config $tmpButtonOpt"

#   .fsBox.pattern.patterns.patterns.menu add checkbutton     -label "Scan extensions"     -variable fsBoxExtensions     -command {
#       global fsBox
#       FSBoxFSShow [.fsBox.path.path get]         [.fsBox.pattern.pattern get] $fsBox(all)}

#   entry .fsBox.pattern.pattern     -relief raised
#   catch ".fsBox.pattern.pattern config $tmpMessageOpt"

#   .fsBox.pattern.pattern insert 0 $fsBox(pattern)
  
#   frame .fsBox.files     -borderwidth 0     -relief raised
#   catch ".fsBox.files config $tmpFrameOpt"

#   scrollbar .fsBox.files.vscroll     -relief raised     -command ".fsBox.files.files yview"
#   catch ".fsBox.files.vscroll config $tmpScrollOpt"

#   scrollbar .fsBox.files.hscroll     -orient horiz     -relief raised     -command ".fsBox.files.files xview"
#   catch ".fsBox.files.hscroll config $tmpScrollOpt"

#   listbox .fsBox.files.files     -exportselection false     -relief raised     -xscrollcommand ".fsBox.files.hscroll set"     -yscrollcommand ".fsBox.files.vscroll set"
#   catch ".fsBox.files.files config $tmpMessageOpt"

#   frame .fsBox.file     -borderwidth 0     -relief raised
#   catch ".fsBox.file config $tmpFrameOpt"

#   label .fsBox.file.labelfile     -relief raised     -text "Filename:"
#   catch ".fsBox.file.labelfile config $tmpMessageOpt"

#   entry .fsBox.file.file     -relief raised
#   catch ".fsBox.file.file config $tmpMessageOpt"

#   .fsBox.file.file delete 0 end
#   .fsBox.file.file insert 0 $fsBox(name)
  
#   checkbutton .fsBox.pattern.all     -offvalue 0     -onvalue 1     -text "Show all files"     -variable fsBox(all)     -command {
#       global fsBox
#       FSBoxFSShow [.fsBox.path.path get]         [.fsBox.pattern.pattern get] $fsBox(all)}
#   catch ".fsBox.pattern.all config $tmpButtonOpt"

#   FSBoxFSShow $fsBox(internalPath) $fsBox(pattern) $fsBox(all)

#   # bindings
#   bind .fsBox.files.files <Double-Button-1> "
#     FSBoxFSFileSelectDouble %W $fsBox(showPixmap) \{$fsBoxActionOk\} %y"
#   bind .fsBox.files.files <ButtonPress-1> "
#     FSBoxFSFileSelect %W $fsBox(showPixmap) %y"
#   bind .fsBox.files.files <Button1-Motion> "
#     FSBoxFSFileSelect %W $fsBox(showPixmap) %y"
#   bind .fsBox.files.files <Shift-Button1-Motion> "
#     FSBoxFSFileSelect %W $fsBox(showPixmap) %y"
#   bind .fsBox.files.files <Shift-ButtonPress-1> "
#     FSBoxFSFileSelect %W $fsBox(showPixmap) %y"

#   bind .fsBox.path.path <Tab> {
#     FSBoxFSNameComplete path}
#   bind .fsBox.path.path <Return> {
#     global tk_version
#     global fsBox
#     FSBoxFSShow [.fsBox.path.path get]       [.fsBox.pattern.pattern get] $fsBox(all)
#     FSBoxFSInsertPath
#     if {$tk_version >= 3.0} {
#       .fsBox.file.file icursor end
#     } {
#       .fsBox.file.file cursor end
#     }
#     focus .fsBox.file.file}
#   catch "bind .fsBox.path.path <Up> {}"
#   bind .fsBox.path.path <Down> {
#     global tk_version
#     if {$tk_version >= 3.0} {
#       .fsBox.file.file icursor end
#     } {
#       .fsBox.file.file cursor end
#     }
#     focus .fsBox.file.file}

#   bind .fsBox.file.file <Tab> {
#     FSBoxFSNameComplete file}
#   bind .fsBox.file.file <Return> "
#     global fsBox
#     set fsBox(name) \[.fsBox.file.file get\]
#     if {$fsBox(showPixmap)} {
#       set fsBox(path) @\[.fsBox.path.path get\]
#     } {
#       set fsBox(path) \[.fsBox.path.path get\]
#     }
#     set fsBox(internalPath) \[.fsBox.path.path get\]
#     $fsBoxActionOk
#     if {\"\[info commands XFDestroy\]\" != \"\"} {
#       catch {XFDestroy .fsBox}
#     } {
#       catch {destroy .fsBox}
#     }"
#   bind .fsBox.file.file <Up> {
#     global tk_version
#     if {$tk_version >= 3.0} {
#       .fsBox.path.path icursor end
#     } {
#       .fsBox.path.path cursor end
#     }
#     focus .fsBox.path.path}
#   bind .fsBox.file.file <Down> {
#     global tk_version
#     if {$tk_version >= 3.0} {
#       .fsBox.pattern.pattern icursor end
#     } {
#       .fsBox.pattern.pattern cursor end
#     }
#     focus .fsBox.pattern.pattern}

#   bind .fsBox.pattern.pattern <Return> {
#     global fsBox
#     FSBoxFSShow [.fsBox.path.path get]       [.fsBox.pattern.pattern get] $fsBox(all)}
#   bind .fsBox.pattern.pattern <Up> {
#     global tk_version
#     if {$tk_version >= 3.0} {
#       .fsBox.file.file icursor end
#     } {
#       .fsBox.file.file cursor end
#     }
#     focus .fsBox.file.file}
#   catch "bind .fsBox.pattern.pattern <Down> {}"

#   # packing
#     pack append .fsBox.files \
# 	    .fsBox.files.vscroll "$fsBox(scrollSide) filly" \
# 	    .fsBox.files.hscroll {bottom fillx} \
# 	    .fsBox.files.files {left fill expand}

#     pack append .fsBox.file \
# 	    .fsBox.file.labelfile {left} \
# 	    .fsBox.file.file {left fill expand}

#     pack append .fsBox.frame1 \
# 	    .fsBox.frame1.ok {left fill expand} \
# 	    .fsBox.frame1.rescan {left fill expand} \
# 	    .fsBox.frame1.cancel {left fill expand}

#   pack append .fsBox.path.paths               .fsBox.path.paths.paths {left}
#   pack append .fsBox.pattern.patterns               .fsBox.pattern.patterns.patterns {left}
#   pack append .fsBox.path               .fsBox.path.paths {left}               .fsBox.path.path {left fill expand}
#   pack append .fsBox.pattern               .fsBox.pattern.patterns {left}               .fsBox.pattern.all {right fill}               .fsBox.pattern.pattern {left fill expand}
#   if {$fsBox(showPixmap)} {
#     pack append .fsBox.frame2                 .fsBox.frame2.scrollbar1 {left filly}                 .fsBox.frame2.canvas2 {top expand fill}                 .fsBox.frame2.scrollbar3 {top fillx} 

#     pack append .fsBox                 .fsBox.message1 {top fill}                 .fsBox.frame1 {bottom fill}                 .fsBox.pattern {bottom fill}                 .fsBox.file {bottom fill}                 .fsBox.path {bottom fill}                 .fsBox.frame2 {right fill}                 .fsBox.files {left fill expand}
#   } {
#     pack append .fsBox                 .fsBox.message1 {top fill}                 .fsBox.frame1 {bottom fill}                 .fsBox.pattern {bottom fill}                 .fsBox.file {bottom fill}                 .fsBox.path {bottom fill}                 .fsBox.files {left fill expand}
#   }

#   if {"$fsBoxActionOk" == "" && "$fsBoxActionCancel" == ""} {
#     # wait for the box to be destroyed
#     update idletask
#     # grab .fsBox
#     tkwait window .fsBox

#     if {"[string trim $fsBox(path)]" != "" ||
#         "[string trim $fsBox(name)]" != ""} {
#       if {"[string trimleft [string trim $fsBox(name)] /]" == ""} {
#         return [string trimright [string trim $fsBox(path)] /]
#       } {
#         return [string trimright [string trim $fsBox(path)] /]/[string trimleft [string trim $fsBox(name)] /]
#       }
#     }
#   }
# }


# # Procedure: FSBoxBindSelectOne
# proc FSBoxBindSelectOne { fsBoxW fsBoxY} {
# # xf ignore me 6
#   set fsBoxNearest [$fsBoxW nearest $fsBoxY]
#   if {$fsBoxNearest >= 0} {
#       global tk_version
#       if { $tk_version < 4.0 } {
# 	  $fsBoxW select from $fsBoxNearest
# 	  $fsBoxW select to $fsBoxNearest
#       } {
# 	  $fsBoxW select set $fsBoxNearest
#       }
#   }
# }


# # Procedure: FSBoxFSFileSelect
# proc FSBoxFSFileSelect { fsBoxW fsBoxShowPixmap fsBoxY} {
# # xf ignore me 6
#   global fsBox

#   FSBoxBindSelectOne $fsBoxW $fsBoxY
#   set fsBoxNearest [$fsBoxW nearest $fsBoxY]
#   if {$fsBoxNearest >= 0} {
#     set fsBoxTmpEntry [$fsBoxW get $fsBoxNearest]
#     if {"[string index $fsBoxTmpEntry           [expr [string length $fsBoxTmpEntry]-1]]" == "/" ||
#         "[string index $fsBoxTmpEntry           [expr [string length $fsBoxTmpEntry]-1]]" == "@"} {
#       set fsBoxFileName [string range $fsBoxTmpEntry 0             [expr [string length $fsBoxTmpEntry]-2]]
#       if {![IsADir [string trimright $fsBox(internalPath)/$fsBoxFileName @]] &&
#           ![IsASymlink [string trimright $fsBox(internalPath)/$fsBoxFileName @]]} {
#         set fsBoxFileName $fsBoxTmpEntry
#       }
#     } {
#       if {"[string index $fsBoxTmpEntry             [expr [string length $fsBoxTmpEntry]-1]]" == "*"} {
#         set fsBoxFileName [string range $fsBoxTmpEntry 0           [expr [string length $fsBoxTmpEntry]-2]]
#         if {![file executable $fsBox(internalPath)/$fsBoxFileName]} {
#           set fsBoxFileName $fsBoxTmpEntry
#         }
#       } {
#         set fsBoxFileName $fsBoxTmpEntry
#       }
#     }
#     if {![IsADir [string trimright $fsBox(internalPath)/$fsBoxFileName @]]} {
#       set fsBox(name) $fsBoxFileName
#       .fsBox.file.file delete 0 end
#       .fsBox.file.file insert 0 $fsBox(name)
#       if {$fsBoxShowPixmap} {
#         catch ".fsBox.frame2.canvas2 itemconfigure currentBitmap -bitmap \"@$fsBox(internalPath)/$fsBox(name)\""
#       }
#     }
#   }
# }


# # Procedure: FSBoxFSFileSelectDouble
# proc FSBoxFSFileSelectDouble { fsBoxW fsBoxShowPixmap fsBoxAction fsBoxY} {
# # xf ignore me 6
#   global fsBox

#   FSBoxBindSelectOne $fsBoxW $fsBoxY
#   set fsBoxNearest [$fsBoxW nearest $fsBoxY]
#   if {$fsBoxNearest >= 0} {
#     set fsBoxTmpEntry [$fsBoxW get $fsBoxNearest]
#     if {"$fsBoxTmpEntry" == "../"} {
#       set fsBoxTmpEntry [string trimright [string trim $fsBox(internalPath)] "@/"]
#       if {"$fsBoxTmpEntry" == ""} {
#         return
#       }
#       FSBoxFSShow [file dirname $fsBoxTmpEntry]         [.fsBox.pattern.pattern get] $fsBox(all)
#       .fsBox.path.path delete 0 end
#       .fsBox.path.path insert 0 $fsBox(internalPath)
#     } {
#       if {"[string index $fsBoxTmpEntry             [expr [string length $fsBoxTmpEntry]-1]]" == "/" ||
#           "[string index $fsBoxTmpEntry             [expr [string length $fsBoxTmpEntry]-1]]" == "@"} {
#         set fsBoxFileName [string range $fsBoxTmpEntry 0               [expr [string length $fsBoxTmpEntry]-2]]
#         if {![IsADir [string trimright $fsBox(internalPath)/$fsBoxFileName @]] &&
#             ![IsASymlink [string trimright $fsBox(internalPath)/$fsBoxFileName @]]} {
#           set fsBoxFileName $fsBoxTmpEntry
#         }
#       } {
#         if {"[string index $fsBoxTmpEntry               [expr [string length $fsBoxTmpEntry]-1]]" == "*"} {
#           set fsBoxFileName [string range $fsBoxTmpEntry 0                 [expr [string length $fsBoxTmpEntry]-2]]
#           if {![file executable $fsBox(internalPath)/$fsBoxFileName]} {
#             set fsBoxFileName $fsBoxTmpEntry
#           }
#         } {
#           set fsBoxFileName $fsBoxTmpEntry
#         }
#       }
#       if {[IsADir [string trimright $fsBox(internalPath)/$fsBoxFileName @]]} {
#         set fsBox(internalPath) "[string trimright $fsBox(internalPath) {/@}]/$fsBoxFileName"
#         FSBoxFSShow $fsBox(internalPath)           [.fsBox.pattern.pattern get] $fsBox(all)
#         .fsBox.path.path delete 0 end
#         .fsBox.path.path insert 0 $fsBox(internalPath)
#       } {
#         set fsBox(name) $fsBoxFileName
#         if {$fsBoxShowPixmap} {
#           set fsBox(path) @$fsBox(internalPath)
#         } {
#           set fsBox(path) $fsBox(internalPath)
#         }
#         if {"$fsBoxAction" != ""} {
#           eval "global fsBox; $fsBoxAction"
#         }
#         if {"[info commands XFDestroy]" != ""} {
#           catch {XFDestroy .fsBox}
#         } {
#           catch {destroy .fsBox}
#         }
#       }
#     }
#   }
# }


# # Procedure: FSBoxFSInsertPath
# proc FSBoxFSInsertPath {} {
# # xf ignore me 6
#   global fsBox

#   set fsBoxLast [.fsBox.path.paths.paths.menu index last]
#   set fsBoxNewEntry [string trimright [.fsBox.path.path get] "/@"]
#   for {set fsBoxCounter 0} {$fsBoxCounter <= $fsBoxLast} {incr fsBoxCounter 1} {
#     if {"$fsBoxNewEntry" ==           "[lindex [.fsBox.path.paths.paths.menu entryconfigure                     $fsBoxCounter -label] 4]"} {
#       return
#     }
#   }
#   if {$fsBoxLast < 9} {
#     .fsBox.path.paths.paths.menu add command       -label "$fsBoxNewEntry"       -command "
#         global fsBox
#         FSBoxFSShow $fsBoxNewEntry           \[.fsBox.pattern.pattern get\] \$fsBox(all)
#         .fsBox.path.path delete 0 end
#         .fsBox.path.path insert 0 $fsBoxNewEntry"
#   } {
#     for {set fsBoxCounter 0} {$fsBoxCounter < $fsBoxLast} {incr fsBoxCounter 1} {
#       .fsBox.path.paths.paths.menu entryconfigure         $fsBoxCounter -label           [lindex [.fsBox.path.paths.paths.menu entryconfigure             [expr $fsBoxCounter+1] -label] 4]
#       .fsBox.path.paths.paths.menu entryconfigure $fsBoxCounter         -command "
#           global fsBox
#           FSBoxFSShow [lindex [.fsBox.path.paths.paths.menu entryconfigure             [expr $fsBoxCounter+1] -label] 4]             \[.fsBox.pattern.pattern get\] \$fsBox(all)
#           .fsBox.path.path delete 0 end
#           .fsBox.path.path insert 0 [lindex             [.fsBox.path.paths.paths.menu entryconfigure               [expr $fsBoxCounter+1] -label] 4]"
#     }
#     .fsBox.path.paths.paths.menu entryconfigure $fsBoxLast       -label "$fsBoxNewEntry"
#     .fsBox.path.paths.paths.menu entryconfigure $fsBoxCounter       -command "
#         global fsBox
#         FSBoxFSShow \[.fsBox.path.path get\]           \[.fsBox.pattern.pattern get\] \$fsBox(all)
#         .fsBox.path.path delete 0 end
#         .fsBox.path.path insert 0 $fsBoxNewEntry"
#   }
# }


# # Procedure: FSBoxFSNameComplete
# proc FSBoxFSNameComplete { fsBoxType} {
# # xf ignore me 6
#   global tk_version
#   global fsBox

#   set fsBoxNewFile ""
#   if {"$fsBoxType" == "path"} {
#     set fsBoxDirName [file dirname [.fsBox.path.path get]]
#     set fsBoxFileName [file tail [.fsBox.path.path get]]
#   } {
#     set fsBoxDirName [file dirname [.fsBox.path.path get]/]
#     set fsBoxFileName [file tail [.fsBox.file.file get]]
#   }

#   set fsBoxNewFile ""
#   if {[IsADir [string trimright $fsBoxDirName @]]} {
#     catch "glob -nocomplain $fsBoxDirName/${fsBoxFileName}*" fsBoxResult
#     foreach fsBoxCounter $fsBoxResult {
#       if {"$fsBoxNewFile" == ""} {
#         set fsBoxNewFile [file tail $fsBoxCounter]
#       } {
#         if {"[string index [file tail $fsBoxCounter] 0]" !=
#             "[string index $fsBoxNewFile 0]"} {
#           set fsBoxNewFile ""
#           break
#         }
#         set fsBoxCounter1 0
#         set fsBoxTmpFile1 $fsBoxNewFile
#         set fsBoxTmpFile2 [file tail $fsBoxCounter]
#         set fsBoxLength1 [string length $fsBoxTmpFile1]
#         set fsBoxLength2 [string length $fsBoxTmpFile2]
#         set fsBoxNewFile ""
#         if {$fsBoxLength1 > $fsBoxLength2} {
#           set fsBoxLength1 $fsBoxLength2
#         }
#         while {$fsBoxCounter1 < $fsBoxLength1} {
#           if {"[string index $fsBoxTmpFile1 $fsBoxCounter1]" ==                 "[string index $fsBoxTmpFile2 $fsBoxCounter1]"} {
#             append fsBoxNewFile [string index $fsBoxTmpFile1 $fsBoxCounter1]
#           } {
#             break
#           }
#           incr fsBoxCounter1 1
#         }
#       }
#     }
#   }
#   if {"$fsBoxNewFile" != ""} {
#     if {[IsADir [string trimright $fsBoxDirName/$fsBoxNewFile @]] ||
#         ![IsAFile [string trimright $fsBoxDirName/$fsBoxNewFile @]]} {
#       if {[IsADir [string trimright $fsBoxDirName/$fsBoxNewFile @]]} {
#         if {"$fsBoxDirName" == "/"} {
#           .fsBox.path.path delete 0 end
#           .fsBox.path.path insert 0 "/[string trimright [string trim $fsBoxNewFile /] @]/"
#         } {
#           .fsBox.path.path delete 0 end
#           .fsBox.path.path insert 0 "[string trimright $fsBoxDirName /]/[string trimright [string trim $fsBoxNewFile /] @]/"
#         }
#         FSBoxFSShow [.fsBox.path.path get]           [.fsBox.pattern.pattern get] $fsBox(all)
#         FSBoxFSInsertPath
#       } {
#         .fsBox.path.path delete 0 end
#         .fsBox.path.path insert 0 "[string trimright $fsBoxDirName /]/[string trimright [string trim $fsBoxNewFile /] @]"
#       }
#     } {
#       .fsBox.path.path delete 0 end
#       .fsBox.path.path insert 0 "[string trimright $fsBoxDirName {@/}]/"
#       .fsBox.file.file delete 0 end
#       .fsBox.file.file insert 0 $fsBoxNewFile
#       if {$tk_version >= 3.0} {
#         .fsBox.file.file icursor end
#       } {
#         .fsBox.file.file cursor end
#       }
#       focus .fsBox.file.file
#     }
#   }
# }


# # Procedure: FSBoxFSShow
# proc FSBoxFSShow { fsBoxPath fsBoxPattern fsBoxAll} {
# # xf ignore me 6
#   global fsBox

#   set tmpButtonOpt ""
#   if {"$fsBox(activeBackground)" != ""} {
#     append tmpButtonOpt "-activebackground \"$fsBox(activeBackground)\" "
#   }
#   if {"$fsBox(activeForeground)" != ""} {
#     append tmpButtonOpt "-activeforeground \"$fsBox(activeForeground)\" "
#   }
#   if {"$fsBox(background)" != ""} {
#     append tmpButtonOpt "-background \"$fsBox(background)\" "
#   }
#   if {"$fsBox(font)" != ""} {
#     append tmpButtonOpt "-font \"$fsBox(font)\" "
#   }
#   if {"$fsBox(foreground)" != ""} {
#     append tmpButtonOpt "-foreground \"$fsBox(foreground)\" "
#   }

#   set fsBox(pattern) $fsBoxPattern
#   if {[file exists $fsBoxPath] && [file readable $fsBoxPath] &&
#       [IsADir $fsBoxPath]} {
#     set fsBox(internalPath) $fsBoxPath
#   } {
#     if {[file exists $fsBoxPath] && [file readable $fsBoxPath] &&
#         [IsAFile $fsBoxPath]} {
#       set fsBox(internalPath) [file dirname $fsBoxPath]
#       .fsBox.file.file delete 0 end
#       .fsBox.file.file insert 0 [file tail $fsBoxPath]
#       set fsBoxPath $fsBox(internalPath)
#     } {
#       while {"$fsBoxPath" != "" && "$fsBoxPath" != "/" &&
#              ![file isdirectory $fsBoxPath]} {
#         set fsBox(internalPath) [file dirname $fsBoxPath]
#          set fsBoxPath $fsBox(internalPath)
#       }
#     }
#   }
#   if {"$fsBoxPath" == ""} {
#     set fsBoxPath "/"
#     set fsBox(internalPath) "/"
#   }
#   .fsBox.path.path delete 0 end
#   .fsBox.path.path insert 0 $fsBox(internalPath)

#   if {[.fsBox.files.files size] > 0} {
#     .fsBox.files.files delete 0 end
#   }
#   if {$fsBoxAll} {
#     if {[catch "exec ls -F -a $fsBoxPath" fsBoxResult]} {
#       puts stderr "$fsBoxResult"
#     }
#   } {
#     if {[catch "exec ls -F $fsBoxPath" fsBoxResult]} {
#       puts stderr "$fsBoxResult"
#     }
#   }
#   set fsBoxElementList [lsort $fsBoxResult]

#   foreach fsBoxCounter [winfo children .fsBox.pattern.patterns.patterns] {
#     if {[string length [info commands XFDestroy]] > 0} {
#       catch {XFDestroy $fsBoxCounter}
#     } {
#       catch {destroy $fsBoxCounter}
#     }
#   }
#   menu .fsBox.pattern.patterns.patterns.menu
#   catch ".fsBox.pattern.patterns.patterns.menu config $tmpButtonOpt"

#   if {$fsBox(extensions)} {
#     .fsBox.pattern.patterns.patterns.menu add command       -label "*"       -command {
#         global fsBox
#         set fsBox(pattern) "*"
#         .fsBox.pattern.pattern delete 0 end
#         .fsBox.pattern.pattern insert 0 $fsBox(pattern)
#         FSBoxFSShow [.fsBox.path.path get] $fsBox(pattern)           $fsBox(all)}
#   }

#   if {"$fsBoxPath" != "/"} {
#     .fsBox.files.files insert end "../"
#   }
#   foreach fsBoxCounter $fsBoxElementList {
#     if {[string match $fsBoxPattern $fsBoxCounter] ||
#         [IsADir [string trimright $fsBoxPath/$fsBoxCounter "/@"]]} {
#       if {"$fsBoxCounter" != "../" &&
#           "$fsBoxCounter" != "./"} {
#         .fsBox.files.files insert end $fsBoxCounter
#       }
#     }

#     if {$fsBox(extensions)} {
#       catch "file rootname $fsBoxCounter" fsBoxRootName
#       catch "file extension $fsBoxCounter" fsBoxExtension
#       set fsBoxExtension [string trimright $fsBoxExtension "/*@"]
#       if {"$fsBoxExtension" != "" && "$fsBoxRootName" != ""} {
#         set fsBoxInsert 1
#         set fsBoxLast [.fsBox.pattern.patterns.patterns.menu index last]
#         for {set fsBoxCounter1 0} {$fsBoxCounter1 <= $fsBoxLast} {incr fsBoxCounter1 1} {
#           if {"*$fsBoxExtension" ==                 "[lindex [.fsBox.pattern.patterns.patterns.menu entryconfigure                         $fsBoxCounter1 -label] 4]"} {
#             set fsBoxInsert 0
#           }
#         }
# 	if {$fsBoxInsert} {
#           .fsBox.pattern.patterns.patterns.menu add command             -label "*$fsBoxExtension"             -command "
#               global fsBox
#               set fsBox(pattern) \"*$fsBoxExtension\"
#               .fsBox.pattern.pattern delete 0 end
#               .fsBox.pattern.pattern insert 0 \$fsBox(pattern)
#               FSBoxFSShow \[.fsBox.path.path get\] \$fsBox(pattern)                 \$fsBox(all)"
#         }
#       }
#     }
#   }
#   if {$fsBox(extensions)} {
#     .fsBox.pattern.patterns.patterns.menu add separator
#   }
#   if {$fsBox(extensions) || 
#       "[.fsBox.pattern.patterns.patterns.menu index last]" == "none"} {
#     .fsBox.pattern.patterns.patterns.menu add checkbutton       -label "Scan extensions"       -variable "fsBox(extensions)"       -command {
#         global fsBox
#         FSBoxFSShow [.fsBox.path.path get]           [.fsBox.pattern.pattern get] $fsBox(all)}
#   }
# }


# # Procedure: IsADir
# proc IsADir { pathName} {
# # xf ignore me 5
# ##########
# # Procedure: IsADir
# # Description: check if name is a directory (including symbolic links)
# # Arguments: pathName - the path to check
# # Returns: 1 if its a directory, otherwise 0
# # Sideeffects: none
# ##########

#   if {[file isdirectory $pathName]} {
#     return 1
#   } {
#     catch "file type $pathName" fileType
#     if {"$fileType" == "link"} {
#       if {[catch "file readlink $pathName" linkName]} {
#         return 0
#       }
#       catch "file type $linkName" fileType
#       while {"$fileType" == "link"} {
#         if {[catch "file readlink $linkName" linkName]} {
#           return 0
#         }
#         catch "file type $linkName" fileType
#       }
#       return [file isdirectory $linkName]
#     }
#   }
#   return 0
# }


# # Procedure: IsAFile
# proc IsAFile { fileName} {
# # xf ignore me 5
# ##########
# # Procedure: IsAFile
# # Description: check if filename is a file (including symbolic links)
# # Arguments: fileName - the filename to check
# # Returns: 1 if its a file, otherwise 0
# # Sideeffects: none
# ##########

#   if {[file isfile $fileName]} {
#     return 1
#   } {
#     catch "file type $fileName" fileType
#     if {"$fileType" == "link"} {
#       if {[catch "file readlink $fileName" linkName]} {
#         return 0
#       }
#       catch "file type $linkName" fileType
#       while {"$fileType" == "link"} {
#         if {[catch "file readlink $linkName" linkName]} {
#           return 0
#         }
#         catch "file type $linkName" fileType
#       }
#       return [file isfile $linkName]
#     }
#   }
#   return 0
# }


# # Procedure: IsASymlink
# proc IsASymlink { fileName} {
# # xf ignore me 5
# ##########
# # Procedure: IsASymlink
# # Description: check if filename is a symbolic link
# # Arguments: fileName - the path/filename to check
# # Returns: none
# # Sideeffects: none
# ##########

#   catch "file type $fileName" fileType
#   if {"$fileType" == "link"} {
#     return 1
#   }
#   return 0
# }

# Procedure: ColorBox
proc ColorBox { {colorBoxFileColor "/usr/local/lib/xf/lib/Colors"} {colorBoxMessage "Color"} {colorBoxEntryW ""} {colorBoxTargetW ""}} {
# xf ignore me 5
##########
# Procedure: ColorBox
# Description: select a color
# Arguments: {colorBoxFileColor} - the color file with all colornames
#            {colorBoxMessage} - a message to display
#            {colorBoxEntryW} - the widget name for the resulting color name
#            {colorBoxTargetW} - the widget we configure
# Returns: colorname, or nothing
# Sideeffects: none
##########
# 
# global colorBox(activeBackground) - active background color
# global colorBox(activeForeground) - active foreground color
# global colorBox(background) - background color
# global colorBox(font) - text font
# global colorBox(foreground) - foreground color
# global colorBox(palette) - a palette of colors
# global colorBox(scrollActiveForeground) - scrollbar active background color
# global colorBox(scrollBackground) - scrollbar background color
# global colorBox(scrollForeground) - scrollbar foreground color
# global colorBox(scrollSide) - side where scrollbar is located

  global colorBox

  set colorBox(colorName) ""
  set colorBox(paletteNr) 0

  set tmpButtonOpt ""
  set tmpFrameOpt ""
  set tmpMessageOpt ""
  set tmpScaleOpt ""
  set tmpScrollOpt ""
  if {"$colorBox(activeBackground)" != ""} {
    append tmpButtonOpt "-activebackground \"$colorBox(activeBackground)\" "
  }
  if {"$colorBox(activeForeground)" != ""} {
    append tmpButtonOpt "-activeforeground \"$colorBox(activeForeground)\" "
  }
  if {"$colorBox(background)" != ""} {
    append tmpButtonOpt "-background \"$colorBox(background)\" "
    append tmpFrameOpt "-background \"$colorBox(background)\" "
    append tmpMessageOpt "-background \"$colorBox(background)\" "
    append tmpScaleOpt "-background \"$colorBox(background)\" "
  }
  if {"$colorBox(font)" != ""} {
    append tmpButtonOpt "-font \"$colorBox(font)\" "
    append tmpMessageOpt "-font \"$colorBox(font)\" "
  }
  if {"$colorBox(foreground)" != ""} {
    append tmpButtonOpt "-foreground \"$colorBox(foreground)\" "
    append tmpMessageOpt "-foreground \"$colorBox(foreground)\" "
    append tmpScaleOpt "-foreground \"$colorBox(foreground)\" "
  }
  if {"$colorBox(scrollActiveForeground)" != ""} {
    append tmpScaleOpt "-activeforeground \"$colorBox(scrollActiveForeground)\" "
    append tmpScrollOpt "-activeforeground \"$colorBox(scrollActiveForeground)\" "
  }
  if {"$colorBox(scrollBackground)" != ""} {
    append tmpScrollOpt "-background \"$colorBox(scrollBackground)\" "
  }
  if {"$colorBox(scrollForeground)" != ""} {
    append tmpScaleOpt "-sliderforeground \"$colorBox(scrollForeground)\" "
    append tmpScrollOpt "-foreground \"$colorBox(scrollForeground)\" "
  }

  # get color file name
  if {!([file exists $colorBoxFileColor] &&
        [file readable $colorBoxFileColor])} {
    set colorBoxFileColor ""
  }
  if {"$colorBoxFileColor" == ""} {
    global env
    if {[info exists env(XF_COLOR_FILE)]} {
      if {[file exists $env(XF_COLOR_FILE)] &&
          [file readable $env(XF_COLOR_FILE)]} {
        set colorBoxFileColor $env(XF_COLOR_FILE)
      }
    }
  }
  if {"$colorBoxMessage" == ""} {
    set colorBoxMessage "Color"
  }

  # save the the current widget color
  if {"$colorBoxTargetW" != ""} {
    if {[catch "$colorBoxTargetW config -[string tolower $colorBoxMessage]" result]} {
      set colorBoxSavedColor ""
    } {
      set colorBoxSavedColor [lindex $result 4]
    }
  } {
    set colorBoxSavedColor ""
  }

  # look if there is already a color window
  if {"[info commands .colorBox]" == ""} {
    # build widget structure

    # start build of toplevel
    if {"[info commands XFDestroy]" != ""} {
      catch {XFDestroy .colorBox}
    } {
      catch {destroy .colorBox}
    }
    toplevel .colorBox  -borderwidth 0
    catch ".colorBox config $tmpFrameOpt"
    wm geometry .colorBox 400x250
    wm title .colorBox {Color box}
    wm maxsize .colorBox 1000 1000
    wm minsize .colorBox 100 100
    # end build of toplevel

    set colorBox(oldWidget) $colorBoxEntryW

    frame .colorBox.frame1  -borderwidth 0  -relief raised
    catch ".colorBox.frame1 config $tmpFrameOpt"
 
    button .colorBox.frame1.ok  -text "OK"
    catch ".colorBox.frame1.ok config $tmpButtonOpt"

    button .colorBox.frame1.cancel  -text "Cancel"
    catch ".colorBox.frame1.cancel config $tmpButtonOpt"

    frame .colorBox.frame2  -borderwidth 0  -relief raised
    catch ".colorBox.frame2 config $tmpFrameOpt"
 
    radiobutton .colorBox.frame2.rgb  -command "ColorBoxShowSlides $colorBoxMessage \"$colorBoxTargetW\""  -text "RGB"  -variable colorBox(type)
    catch ".colorBox.frame2.rgb config $tmpButtonOpt"

    radiobutton .colorBox.frame2.hsv  -command "ColorBoxShowSlides $colorBoxMessage \"$colorBoxTargetW\""  -text "HSV"  -variable colorBox(type)
    catch ".colorBox.frame2.hsv config $tmpButtonOpt"

    radiobutton .colorBox.frame2.list  -command "ColorBoxShowSlides $colorBoxMessage \"$colorBoxTargetW\""  -text "List"  -variable colorBox(type)
    catch ".colorBox.frame2.list config $tmpButtonOpt"

    frame .colorBox.palette  -borderwidth 0  -relief raised
    catch ".colorBox.palette config $tmpFrameOpt"
 
    set counter 0
    foreach element $colorBox(palette) {
      button .colorBox.palette.palette$counter  -command "ColorBoxSetPalette $colorBoxMessage \"$colorBoxTargetW\" $counter"  -width 3
      catch ".colorBox.palette.palette$counter config  -activebackground \"$element\"  -background \"$element\""

      pack append .colorBox.palette .colorBox.palette.palette$counter {left fill expand}
      incr counter
    }

    scale .colorBox.red  -background "red"  -from 0  -label "Red"  -orient horizontal  -relief raised  -sliderlength 15  -to 255  -width 8
    catch ".colorBox.red config $tmpScaleOpt"

    scale .colorBox.green  -background "green"  -from 0  -label "Green"  -orient horizontal  -relief raised  -sliderlength 15  -to 255  -width 8
    catch ".colorBox.green config $tmpScaleOpt"

    scale .colorBox.blue  -background "blue"  -from 0  -label "Blue"  -orient horizontal  -relief raised  -sliderlength 15  -to 255  -width 8
    catch ".colorBox.blue config $tmpScaleOpt"

    scale .colorBox.h  -from 0  -label "Hue"  -orient horizontal  -relief raised  -sliderlength 15  -to 1000  -width 8
    catch ".colorBox.h config $tmpScaleOpt"

   scale .colorBox.s  -from 0  -label "Saturation * 100"  -orient horizontal  -relief raised  -sliderlength 15  -to 1000  -width 8
    catch ".colorBox.s config $tmpScaleOpt"

    scale .colorBox.v  -from 0  -label "Value"  -orient horizontal  -relief raised  -sliderlength 15  -to 1000  -width 8
    catch ".colorBox.v config $tmpScaleOpt"

    label .colorBox.demo  -relief raised  -text "This text shows the results :-)"
    catch ".colorBox.demo config $tmpMessageOpt"

    frame .colorBox.current  -borderwidth 0  -relief raised
    catch ".colorBox.current config $tmpFrameOpt"

    label .colorBox.current.labelcurrent  -relief raised
    catch ".colorBox.current.labelcurrent config $tmpMessageOpt"

    entry .colorBox.current.current  -relief raised
    catch ".colorBox.current.current config $tmpMessageOpt"

    frame .colorBox.colors  -borderwidth 0  -relief raised
    catch ".colorBox.colors config $tmpFrameOpt"

    scrollbar .colorBox.colors.vscroll  -relief raised  -command ".colorBox.colors.colors yview"
    catch ".colorBox.colors.vscroll config $tmpScrollOpt"

    scrollbar .colorBox.colors.hscroll  -orient horiz  -relief raised  -command ".colorBox.colors.colors xview"
    catch ".colorBox.colors.hscroll config $tmpScrollOpt"

    listbox .colorBox.colors.colors  -exportselection false  -relief raised  -xscrollcommand ".colorBox.colors.hscroll set"  -yscrollcommand ".colorBox.colors.vscroll set"
    catch ".colorBox.colors.colors config $tmpMessageOpt"

    # read color file
    if {"$colorBoxFileColor" != ""} {
      if {[catch "open $colorBoxFileColor r" colorInFile]} {
        set colorBoxFileColor ""
        if {"[info commands AlertBox]" != ""} {
          AlertBox "$colorInFile"
        } {
          puts stderr "$colorInFile"
        }
      } {
        set colorReadList [read $colorInFile]
        close $colorInFile
        foreach colorLine [split $colorReadList "\n"] {
          if {"[string trim $colorLine]" != ""} {
            set colorNewLine [lrange $colorLine 3 end]
            append colorNewLine " " [format #%02x [lindex $colorLine 0]]
            append colorNewLine [format %02x [lindex $colorLine 1]]
            append colorNewLine [format %02x [lindex $colorLine 2]]
            .colorBox.colors.colors insert end $colorNewLine
          }
        }
      }
    }

    # bindings
    bind .colorBox.colors.colors <ButtonPress-1> "
      ColorBoxSelectColor %W $colorBoxMessage \"$colorBoxTargetW\" %y"
    bind .colorBox.colors.colors <Button1-Motion> "
      ColorBoxSelectColor %W $colorBoxMessage \"$colorBoxTargetW\" %y"
    bind .colorBox.colors.colors <Shift-ButtonPress-1> "
      ColorBoxSelectColor %W $colorBoxMessage \"$colorBoxTargetW\" %y"
    bind .colorBox.colors.colors <Shift-Button1-Motion> "
      ColorBoxSelectColor %W $colorBoxMessage \"$colorBoxTargetW\" %y"
  } {
    if {"[winfo class $colorBox(oldWidget)]" == "Text"} {
      catch "$colorBox(oldWidget) delete 1.0 end"
      catch "$colorBox(oldWidget) insert 1.0 [.colorBox.current.current get]"
    } {
      if {"[winfo class $colorBox(oldWidget)]" == "Entry"} {
        catch "$colorBox(oldWidget) delete 0 end"
        catch "$colorBox(oldWidget) insert 0 [.colorBox.current.current get]"
      }
    }

    set colorBox(oldWidget) $colorBoxEntryW
  }
   
  .colorBox.frame1.ok config  -command "
      global colorBox
      set colorBox(colorName) \[.colorBox.current.current get\]
      if {\"$colorBoxEntryW\" != \"\"} {
        if {\"\[winfo class $colorBoxEntryW\]\" == \"Text\"} {
          catch \"$colorBoxEntryW delete 1.0 end\"
          catch \"$colorBoxEntryW insert 1.0 \\\"\$colorBox(colorName)\\\"\"
        } {
          if {\"\[winfo class $colorBoxEntryW\]\" == \"Entry\"} {
            catch \"$colorBoxEntryW delete 0 end\"
            catch \"$colorBoxEntryW insert 0 \\\"\$colorBox(colorName)\\\"\"
          }
        }
      }
      if {\"\[info commands XFDestroy\]\" != \"\"} {
        catch {XFDestroy .colorBox}
      } {
        catch {destroy .colorBox}
      }"

  .colorBox.frame1.cancel config  -command "
      global colorBox
      set colorBox(colorName) {}
      if {\"$colorBoxTargetW\" != \"\"} {
        catch \"$colorBoxTargetW config -\[string tolower $colorBoxMessage\] $colorBoxSavedColor\"
      }
      if {\"\[info commands XFDestroy\]\" != \"\"} {
        catch {XFDestroy .colorBox}
      } {
        catch {destroy .colorBox}
      }"

  .colorBox.red config  -command "ColorBoxSetRGBColor $colorBoxMessage \"$colorBoxTargetW\""

  .colorBox.green config  -command "ColorBoxSetRGBColor $colorBoxMessage \"$colorBoxTargetW\""

  .colorBox.blue config  -command "ColorBoxSetRGBColor $colorBoxMessage \"$colorBoxTargetW\""

  .colorBox.h config  -command "ColorBoxSetHSVColor $colorBoxMessage \"$colorBoxTargetW\""

  .colorBox.s config  -command "ColorBoxSetHSVColor $colorBoxMessage \"$colorBoxTargetW\""

  .colorBox.v config  -command "ColorBoxSetHSVColor $colorBoxMessage \"$colorBoxTargetW\""

  .colorBox.current.labelcurrent config  -text "$colorBoxMessage:"

  # bindings
  bind .colorBox.current.current <Return> "
    ColorBoxSetPaletteList \[.colorBox.current.current get\]
    ColorBoxSetColor $colorBoxMessage \"$colorBoxTargetW\" text \[.colorBox.current.current get\]"

  bind .colorBox.colors.colors <Double-1> "
    ColorBoxSelectColor %W $colorBoxMessage \"$colorBoxTargetW\" %y
    global colorBox
    set colorBox(colorName) \[.colorBox.current.current get\]
    if {\"$colorBoxEntryW\" != \"\"} {
      if {\"\[winfo class $colorBoxEntryW\]\" == \"Text\"} {
        catch \"$colorBoxEntryW delete 1.0 end\"
        catch \"$colorBoxEntryW insert 1.0 \\\"\$colorBox(colorName)\\\"\"
      } {
        if {\"\[winfo class $colorBoxEntryW\]\" == \"Entry\"} {
          catch \"$colorBoxEntryW delete 0 end\"
          catch \"$colorBoxEntryW insert 0 \\\"\$colorBox(colorName)\\\"\"
        }
      }
    }
    if {\"\[info commands XFDestroy\]\" != \"\"} {
      catch {XFDestroy .colorBox}
    } {
      catch {destroy .colorBox}
    }"

  # set up current value
  .colorBox.current.current delete 0 end
  if {"$colorBoxEntryW" != ""} {
    if {"[winfo class $colorBoxEntryW]" == "Text"} {
      .colorBox.current.current insert 0 [$colorBoxEntryW get 1.0 end]
    } {
      if {"[winfo class $colorBoxEntryW]" == "Entry"} {
        .colorBox.current.current insert 0 [$colorBoxEntryW get]
      }
    }
  }
  if {"[.colorBox.current.current get]" != ""} {
    ColorBoxSetColor $colorBoxMessage $colorBoxTargetW text [.colorBox.current.current get]
  }
    
  # packing
  pack append .colorBox.frame1  .colorBox.frame1.ok {left fill expand}  .colorBox.frame1.cancel {left fill expand}
  pack append .colorBox.frame2  .colorBox.frame2.rgb {left fill expand}  .colorBox.frame2.hsv {left fill expand}  .colorBox.frame2.list {left fill expand}
  pack append .colorBox.current  .colorBox.current.labelcurrent {left}  .colorBox.current.current {left fill expand}
  pack append .colorBox.colors  .colorBox.colors.vscroll "$colorBox(scrollSide) filly"  .colorBox.colors.hscroll {bottom fillx}  .colorBox.colors.colors {left fill expand}

  ColorBoxShowSlides $colorBoxMessage $colorBoxTargetW

  catch "wm deiconify .colorBox"

  if {"$colorBoxEntryW" == ""} {
    # wait for the box to be destroyed
    update idletask
    # grab .colorBox
    tkwait window .colorBox

    return $colorBox(colorName)
  }
}


# Procedure: ColorBoxHSVToRGB
proc ColorBoxHSVToRGB { colorBoxHue colorBoxSat colorBoxValue} {
# xf ignore me 6
# The HSV <-> RGB converting routines are from the
# tcolor demo that is part of the demo site of Tk.

  set colorBoxV [format %.0f [expr 65535.0*$colorBoxValue]]
  if {$colorBoxSat == 0} {
    return "$colorBoxV $colorBoxV $colorBoxV"
  } else {
    set colorBoxHue [expr $colorBoxHue*6.0]
    if {$colorBoxHue >= 6.0} {
      set colorBoxHue 0.0
    }
    scan $colorBoxHue. %d i
    set colorBoxF [expr $colorBoxHue-$i]
    set colorBoxP [format %.0f [expr {65535.0*$colorBoxValue*(1 - $colorBoxSat)}]]
    set colorBoxQ [format %.0f [expr {65535.0*$colorBoxValue*(1 - ($colorBoxSat*$colorBoxF))}]]
    set colorBoxT [format %.0f [expr {65535.0*$colorBoxValue*(1 - ($colorBoxSat*(1 - $colorBoxF)))}]]
    case $i  0 {return "$colorBoxV $colorBoxT $colorBoxP"}  1 {return "$colorBoxQ $colorBoxV $colorBoxP"}  2 {return "$colorBoxP $colorBoxV $colorBoxT"}  3 {return "$colorBoxP $colorBoxQ $colorBoxV"}  4 {return "$colorBoxT $colorBoxP $colorBoxV"}  5 {return "$colorBoxV $colorBoxP $colorBoxQ"}
    error "i value $i is out of range"
  }
}


# Procedure: ColorBoxRGBToHSV
proc ColorBoxRGBToHSV { colorBoxRed colorBoxGreen colorBoxBlue} {
# xf ignore me 6
# The HSV <-> RGB converting routines are from the
# tcolor demo that is part of the demo site of Tk.

  if {$colorBoxRed > $colorBoxGreen} {
    set colorBoxMax $colorBoxRed.0
    set colorBoxMin $colorBoxGreen.0
  } else {
    set colorBoxMax $colorBoxGreen.0
    set colorBoxMin $colorBoxRed.0
  }
  if {$colorBoxBlue > $colorBoxMax} {
    set colorBoxMax $colorBoxBlue.0
  } else {
    if {$colorBoxBlue < $colorBoxMin} {
      set colorBoxMin $colorBoxBlue.0
    }
  }
  set range [expr $colorBoxMax-$colorBoxMin]
  if {$colorBoxMax == 0} {
    set colorBoxSat 0
  } else {
    set colorBoxSat [expr {($colorBoxMax-$colorBoxMin)/$colorBoxMax}]
  }
  if {$colorBoxSat == 0} {
    set colorBoxHue 0
  } else {
    set colorBoxRC [expr {($colorBoxMax - $colorBoxRed)/$range}]
    set colorBoxGC [expr {($colorBoxMax - $colorBoxGreen)/$range}]
    set colorBoxBC [expr {($colorBoxMax - $colorBoxBlue)/$range}]
    if {$colorBoxRed == $colorBoxMax} {
      set colorBoxHue [expr {.166667*($colorBoxBC - $colorBoxGC)}]
    } else {
      if {$colorBoxGreen == $colorBoxMax} {
        set colorBoxHue [expr {.166667*(2 + $colorBoxRC - $colorBoxBC)}]
      } else {
        set colorBoxHue [expr {.166667*(4 + $colorBoxGC - $colorBoxRC)}]
      }
    }
  }
  return [list $colorBoxHue $colorBoxSat [expr {$colorBoxMax/65535}]]
}


# Procedure: ColorBoxSelectColor
proc ColorBoxSelectColor { colorW colorBoxMessage colorBoxTargetW colorY} {
# xf ignore me 6

  set colorNearest [$colorW nearest $colorY]
  if {$colorNearest >= 0} {
    $colorW select from $colorNearest
    $colorW select to $colorNearest
    set colorTmpValue [$colorW get $colorNearest]
    set colorCurrentColor [lrange $colorTmpValue 0  [expr [llength $colorTmpValue]-2]]
    set colorCurrentValue [lrange $colorTmpValue  [expr [llength $colorTmpValue]-1] end]

    scan [string range $colorCurrentValue 1 2] "%x" colorBoxValue
    .colorBox.red set $colorBoxValue
    scan [string range $colorCurrentValue 3 4] "%x" colorBoxValue
    .colorBox.green set $colorBoxValue
    scan [string range $colorCurrentValue 5 6] "%x" colorBoxValue
    .colorBox.blue set $colorBoxValue

    .colorBox.current.current delete 0 end
    .colorBox.current.current insert 0 $colorCurrentColor
    ColorBoxSetColor $colorBoxMessage $colorBoxTargetW list $colorCurrentColor
    ColorBoxSetPaletteList $colorCurrentColor
  }
}


# Procedure: ColorBoxSetColor
proc ColorBoxSetColor { colorBoxMessage colorBoxTargetW colorBoxType colorBoxValue} {
# xf ignore me 6
  global colorBox

  .colorBox.red config  -command "NoFunction"
  .colorBox.green config  -command "NoFunction"
  .colorBox.blue config  -command "NoFunction"
  .colorBox.h config  -command "NoFunction"
  .colorBox.s config  -command "NoFunction"
  .colorBox.v config  -command "NoFunction"

  set colorBoxSetColor ""
  if {"$colorBoxValue" != ""} {
    if {"$colorBoxType" != "text"} {
      .colorBox.current.current delete 0 end
      .colorBox.current.current insert 0 $colorBoxValue
    }
    if {[string match "*oreground*" $colorBoxMessage]} {
      catch ".colorBox.demo config -foreground $colorBoxValue"
    } {
      catch ".colorBox.demo config -background $colorBoxValue"
    }
    if {"$colorBoxTargetW" != ""} {
      catch "$colorBoxTargetW config -[string tolower $colorBoxMessage]  $colorBoxValue"
    }
  }
  case $colorBoxType in {
    {text palette} {
      if {[string match "*oreground*" $colorBoxMessage]} {
        set red [expr [lindex [winfo rgb .colorBox.demo [lindex [.colorBox.demo config -foreground] 4]] 0]/256]
        set green [expr [lindex [winfo rgb .colorBox.demo [lindex [.colorBox.demo config -foreground] 4]] 1]/256]
        set blue [expr [lindex [winfo rgb .colorBox.demo [lindex [.colorBox.demo config -foreground] 4]] 2]/256]
      } {
        set red [expr [lindex [winfo rgb .colorBox.demo [lindex [.colorBox.demo config -background] 4]] 0]/256]
        set green [expr [lindex [winfo rgb .colorBox.demo [lindex [.colorBox.demo config -background] 4]] 1]/256]
        set blue [expr [lindex [winfo rgb .colorBox.demo [lindex [.colorBox.demo config -background] 4]] 2]/256]
      }
      if {"$colorBox(type)" == "rgb"} {
        .colorBox.red set $red
        .colorBox.green set $green
        .colorBox.blue set $blue
      } {
        if {"$colorBox(type)" == "hsv"} {
          set colorBoxHSV [ColorBoxRGBToHSV [expr $red*256] [expr $green*256] [expr $blue*256]]
          .colorBox.h set [format %.0f [expr [lindex $colorBoxHSV 0]*1000.0]]
          .colorBox.s set [format %.0f [expr [lindex $colorBoxHSV 1]*1000.0]]
          .colorBox.v set [format %.0f [expr [lindex $colorBoxHSV 2]*1000.0]]
        }
      }
    }
  }
  .colorBox.red config  -command "ColorBoxSetRGBColor $colorBoxMessage \"$colorBoxTargetW\""
  .colorBox.green config  -command "ColorBoxSetRGBColor $colorBoxMessage \"$colorBoxTargetW\""
  .colorBox.blue config  -command "ColorBoxSetRGBColor $colorBoxMessage \"$colorBoxTargetW\""
  .colorBox.h config  -command "ColorBoxSetHSVColor $colorBoxMessage \"$colorBoxTargetW\""
  .colorBox.s config  -command "ColorBoxSetHSVColor $colorBoxMessage \"$colorBoxTargetW\""
  .colorBox.v config  -command "ColorBoxSetHSVColor $colorBoxMessage \"$colorBoxTargetW\""
}


# Procedure: ColorBoxSetHSVColor
proc ColorBoxSetHSVColor { colorBoxMessage colorBoxTargetW colorBoxValue} {
# xf ignore me 6
  global colorBox

  set colorBoxRGB [ColorBoxHSVToRGB [expr [.colorBox.h get]/1000.0] [expr [.colorBox.s get]/1000.0] [expr [.colorBox.v get]/1000.0]]
  ColorBoxSetColor $colorBoxMessage $colorBoxTargetW hsv  [format #%04x%04x%04x [lindex $colorBoxRGB 0] [lindex $colorBoxRGB 1] [lindex $colorBoxRGB 2]]
  ColorBoxSetPaletteList [format #%04x%04x%04x [lindex $colorBoxRGB 0] [lindex $colorBoxRGB 1] [lindex $colorBoxRGB 2]]
}


# Procedure: ColorBoxSetPalette
proc ColorBoxSetPalette { colorBoxMessage colorBoxTargetW colorBoxElement} {
# xf ignore me 6
  global colorBox

  set colorBox(paletteNr) $colorBoxElement
  ColorBoxSetColor $colorBoxMessage $colorBoxTargetW palette  [lindex [.colorBox.palette.palette$colorBoxElement config -background] 4]
}


# Procedure: ColorBoxSetPaletteList
proc ColorBoxSetPaletteList { colorBoxValue} {
# xf ignore me 6
  global colorBox

  catch ".colorBox.palette.palette$colorBox(paletteNr) config  -activebackground $colorBoxValue"
  catch ".colorBox.palette.palette$colorBox(paletteNr) config  -background $colorBoxValue"
  set colorBox(palette)  [lreplace $colorBox(palette) $colorBox(paletteNr) $colorBox(paletteNr)  $colorBoxValue]
}


# Procedure: ColorBoxSetRGBColor
proc ColorBoxSetRGBColor { colorBoxMessage colorBoxTargetW colorBoxValue} {
# xf ignore me 6
  global colorBox

  ColorBoxSetColor $colorBoxMessage $colorBoxTargetW rgb  [format #%02x%02x%02x [.colorBox.red get]  [.colorBox.green get] [.colorBox.blue get]]
  ColorBoxSetPaletteList [format #%02x%02x%02x [.colorBox.red get]  [.colorBox.green get] [.colorBox.blue get]]
}


# Procedure: ColorBoxShowSlides
proc ColorBoxShowSlides { colorBoxMessage colorBoxTargetW} {
# xf ignore me 6
  global colorBox

  catch "pack unpack .colorBox.frame1"
  catch "pack unpack .colorBox.frame2"
  catch "pack unpack .colorBox.current"
  catch "pack unpack .colorBox.demo"
  catch "pack unpack .colorBox.h"
  catch "pack unpack .colorBox.s"
  catch "pack unpack .colorBox.v"
  catch "pack unpack .colorBox.red"
  catch "pack unpack .colorBox.green"
  catch "pack unpack .colorBox.blue"
  catch "pack unpack .colorBox.colors"
  case $colorBox(type) in {
    {rgb} {
      pack append .colorBox  .colorBox.frame1 {bottom fillx}  .colorBox.frame2 {bottom fillx}  .colorBox.current {bottom fillx}  .colorBox.palette {bottom fillx}  .colorBox.red {top fillx}  .colorBox.green {top fillx}  .colorBox.blue {top fillx}  .colorBox.demo {bottom fill expand}
    }
    {hsv} {
      pack append .colorBox  .colorBox.frame1 {bottom fillx}  .colorBox.frame2 {bottom fillx}  .colorBox.current {bottom fillx}  .colorBox.palette {bottom fillx}  .colorBox.h {top fillx}  .colorBox.s {top fillx}  .colorBox.v {top fillx}  .colorBox.demo {bottom fill expand}
    }
    {list} {
      pack append .colorBox  .colorBox.frame1 {bottom fillx}  .colorBox.frame2 {bottom fillx}  .colorBox.current {bottom fillx}  .colorBox.palette {bottom fillx}  .colorBox.demo {bottom fillx}  .colorBox.colors {top fill expand}
    }
  }
  if {[string match "*oreground*" $colorBoxMessage]} {
    ColorBoxSetColor $colorBoxMessage $colorBoxTargetW text  [lindex [.colorBox.demo config -foreground] 4]
  } {
    ColorBoxSetColor $colorBoxMessage $colorBoxTargetW text  [lindex [.colorBox.demo config -background] 4]
  }
}

# application loading procedure
proc XFLocalLoadAppDefs {{xfClasses ""} {xfPriority "startupFile"} {xfAppDefFile ""}} {
  global env

  if {"$xfAppDefFile" == ""} {
    set xfFileList ""
    if {[info exists env(XUSERFILESEARCHPATH)]} {
      append xfFileList [split $env(XUSERFILESEARCHPATH) :]
    }
    if {[info exists env(XAPPLRESDIR)]} {
      append xfFileList [split $env(XAPPLRESDIR) :]
    }
    if {[info exists env(XFILESEARCHPATH)]} {
      append xfFileList [split $env(XFILESEARCHPATH) :]
    }
    append xfFileList " /usr/lib/X11/app-defaults"
    append xfFileList " /usr/X11/lib/X11/app-defaults"

    foreach xfCounter1 $xfClasses {
      foreach xfCounter2 $xfFileList {
        set xfPathName $xfCounter2
        if {[regsub -all "%N" "$xfPathName" "$xfCounter1" xfResult]} {
          set xfPathName $xfResult
        }
        if {[regsub -all "%T" "$xfPathName" "app-defaults" xfResult]} {
          set xfPathName $xfResult
        }
        if {[regsub -all "%S" "$xfPathName" "" xfResult]} {
          set xfPathName $xfResult
        }
        if {[regsub -all "%C" "$xfPathName" "" xfResult]} {
          set xfPathName $xfResult
        }
        if {[file exists $xfPathName] &&
            [file readable $xfPathName] &&
            ("[file type $xfPathName]" == "file" ||
             "[file type $xfPathName]" == "link")} {
          catch "option readfile $xfPathName $xfPriority"
          if {"[info commands XFParseAppDefs]" != ""} {
            XFParseAppDefs $xfPathName
          } {
            if {"[info commands XFLocalParseAppDefs]" != ""} {
              XFLocalParseAppDefs $xfPathName
            }
          }
        } {
          if {[file exists $xfCounter2/$xfCounter1] &&
              [file readable $xfCounter2/$xfCounter1] &&
              ("[file type $xfCounter2/$xfCounter1]" == "file" ||
               "[file type $xfCounter2/$xfCounter1]" == "link")} {
            catch "option readfile $xfCounter2/$xfCounter1 $xfPriority"
            if {"[info commands XFParseAppDefs]" != ""} {
              XFParseAppDefs $xfCounter2/$xfCounter1
            } {
              if {"[info commands XFLocalParseAppDefs]" != ""} {
                XFLocalParseAppDefs $xfCounter2/$xfCounter1
              }
            }
          }
        }
      }
    }
  } {
    # load a specific application defaults file
    if {[file exists $xfAppDefFile] &&
        [file readable $xfAppDefFile] &&
        ("[file type $xfAppDefFile]" == "file" ||
         "[file type $xfAppDefFile]" == "link")} {
      catch "option readfile $xfAppDefFile $xfPriority"
      if {"[info commands XFParseAppDefs]" != ""} {
        XFParseAppDefs $xfAppDefFile
      } {
        if {"[info commands XFLocalParseAppDefs]" != ""} {
          XFLocalParseAppDefs $xfAppDefFile
        }
      }
    }
  }
}

# application setting procedure
proc XFLocalSetAppDefs {{xfWidgetPath "."}} {
  global xfAppDefaults

  if {![info exists xfAppDefaults]} {
    return
  }
  foreach xfCounter $xfAppDefaults {
    if {"$xfCounter" == ""} {
      break
    }
    set widname [lindex $xfCounter 0]
    if {[string match $widname ${xfWidgetPath}] ||
        [string match "${xfWidgetPath}*" $widname]} {
      set name [string tolower [lindex $xfCounter 1]]
      set value [lindex $xfCounter 2]
      # Now lets see how many tcl commands match the name
      # pattern specified.
      set widlist [info command $widname]
      if {"$widlist" != ""} {
        foreach widget $widlist {
          # make sure this command is a widget.
          if {![catch "winfo id $widget"] &&
              [string match "${xfWidgetPath}*" $widget]} {
            catch "$widget configure -$name $value" 
          }
        }
      }
    }
  }
}



# startup source
proc StartupSrc {args} {
    global {colorBox}
    set {colorBox(activeBackground)} {}
    set {colorBox(activeForeground)} {}
    set {colorBox(background)} {}
    set {colorBox(colorName)} {ghost white}
    set {colorBox(font)} {}
    set {colorBox(foreground)} {}
    set {colorBox(oldWidget)} {}
    set {colorBox(palette)} {{ghost white} black gray50 #0000ff red green \
            yellow orange}
    set {colorBox(paletteNr)} {0}
    set {colorBox(scrollActiveForeground)} {}
    set {colorBox(scrollBackground)} {}
    set {colorBox(scrollForeground)} {}
    set {colorBox(scrollSide)} {left}
    set {colorBox(type)} {list}
    global auto_path
    set auto_path  [linsert $auto_path 0 .]
}


# end source
proc EndSrc {} {
.frame0.frame5.radiobutton16 select
# .frame0.frame4.radiobutton10 select
.frame0.frame3.checkbutton8 toggle
}

# prepare auto loading
global auto_path
global tk_library
global xfLoadPath
foreach xfElement [eval list [split $xfLoadPath :] $auto_path] {
  if {[file exists $xfElement/tclIndex]} {
    lappend auto_path $xfElement
  }
}
lappend auto_path "hypertext"

catch "unset auto_index"

catch "unset auto_oldpath"

catch "unset auto_execs"

# initialize global variables
proc InitGlobals {} {
  global {colorBox}
  set {colorBox(activeBackground)} {}
  set {colorBox(activeForeground)} {}
  set {colorBox(background)} {}
  set {colorBox(colorName)} {ghost white}
  set {colorBox(font)} {}
  set {colorBox(foreground)} {}
  set {colorBox(oldWidget)} {}
  set {colorBox(palette)} {{ghost white} black gray50 #0000ff red green yellow orange}
  set {colorBox(paletteNr)} {0}
  set {colorBox(scrollActiveForeground)} {}
  set {colorBox(scrollBackground)} {}
  set {colorBox(scrollForeground)} {}
  set {colorBox(scrollSide)} {left}
  set {colorBox(type)} {list}
  global {alertBox}
  set {alertBox(activeBackground)} {}
  set {alertBox(activeForeground)} {}
  set {alertBox(after)} {0}
  set {alertBox(anchor)} {nw}
  set {alertBox(background)} {}
  set {alertBox(button)} {0}
  set {alertBox(font)} {}
  set {alertBox(foreground)} {}
  set {alertBox(justify)} {center}
  set {alertBox(toplevelName)} {.alertBox}
  global inputOneBox
  set inputOneBox(toplevelName) .inputOneBox
  set inputOneBox(entry) ""
  global {fsBox}
  set {fsBox(activeBackground)} {}
  set {fsBox(activeForeground)} {}
  set {fsBox(all)} {0}
  set {fsBox(background)} {}
  set {fsBox(button)} {0}
  set {fsBox(extensions)} {0}
  set {fsBox(font)} {}
  set {fsBox(foreground)} {}
  set {fsBox(internalPath)} {/tmp_mnt/home/bw/friedric/working/w8}
  set {fsBox(name)} {}
  set {fsBox(path)} {}
  set {fsBox(pattern)} {*}
  set {fsBox(scrollActiveForeground)} {}
  set {fsBox(scrollBackground)} {}
  set {fsBox(scrollForeground)} {}
  set {fsBox(scrollSide)} {left}
  set {fsBox(showPixmap)} {0}
  global {fsBoxExtensions}
  set {fsBoxExtensions} {0}
  global {result}
  set {result} {-scrollincrement scrollIncrement ScrollIncrement 10 10}

  # please don't modify the following
  # variables. They are needed by xf.
  global {autoLoadList}
  global {internalAliasList}
  set {internalAliasList} {}
  global {moduleList}
  global {preloadList}
  set {preloadList(xfInternal)} {}
  global {symbolicName}
  set {symbolicName(root)} {.}
  global {xfWmSetPosition}
  set {xfWmSetPosition} {.top0 .}
  global {xfWmSetSize}
  set {xfWmSetSize} {. .top0}
  global {xfAppDefToplevels}
  set {xfAppDefToplevels} {. }
}

# initialize global variables

InitGlobals
