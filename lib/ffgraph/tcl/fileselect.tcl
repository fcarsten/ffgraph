#**************************************************************************/
#                                                                         
#  FILE       : fileselect.tcl
#                                                                        
#  AUTHOR     : Achim Dannecker
#                                                                         
#  DESCRIPTION: 
#               For more detail see descriptions below.
#
#  LAST UPDATE: today
#  Version    : 1.1b 
#                                                                         
#  No warranties of any kind.                                             
#  Everything else according to the gnu public license.                 
#  If you have any problems or found some bugs (hmmmm, ask someone 
#     else, but not me).
#
#**************************************************************************/

proc FileSelectorBox_SetMenuText {text} {
  ##############
  # Procedure  : FileSelectorBox_SetMenuText
  # Description: Sets the text ion the PopUpMenuButton
  # Arguments  : text: text to display
  # Returns    : none
  # Sideeffects: the author is not familiar to any strange sideeffects 
  #              (shit happens)
  ##############
  global fileselect
  if {[string length $text] > $fileselect(label_width)} {
    .fileselect.misc.left.begin_dir.l configure -text "[string range $text 0 [expr $fileselect(label_width)-3]]..."
  } else {
    .fileselect.misc.left.begin_dir.l configure -text $text
  }
  set fileselect(currentmenu) $text
}

proc FileSelectorBox_MessageBox {message title ok_but {cancel_but ""}} {
  ##############
  # Procedure  : FileSelectorBox_MessageBox
  # Description: shows a little messagebox 
  # Arguments  : message: message to show
  #              title: the title of the messagebox
  #              ok_but: text for the ok-button
  #              cancle_but: text for the cancel-button (optionaly)
  # Returns    : 1 if ok-button is hit
  #              0 else
  # Sideeffects: the author is not familiar to any strange sideeffects 
  #              (shit happens)
  ##############
  global htmessagebox
  set htmessagebox -1
  toplevel .htmessagebox
  wm title .htmessagebox $title
  wm resizable .htmessagebox 0 0
  wm protocol .htmessagebox WM_DELETE_WINDOW "set htmessagebox 0"
  message .htmessagebox.m -text $message \
    -aspect 10000 -relief ridge -bd 3 \
    -justify center -padx 40 -pady 20
  button .htmessagebox.ok \
    -text $ok_but -command "set htmessagebox 1" \
    -highlightthickness 0
  pack append .htmessagebox \
    .htmessagebox.m {top fillx} \
    .htmessagebox.ok {left expand fillx} 

  if {$cancel_but != ""} {
    button .htmessagebox.cancel \
      -text $cancel_but -command "set htmessagebox 0" \
      -highlightthickness 0
    pack append .htmessagebox \
      .htmessagebox.cancel {left expand fillx}
  }
  wm iconify .htmessagebox
  update
  set x [expr [winfo screenwidth .]/2 - [winfo width .htmessagebox]/2]
  set y [expr [winfo screenheight .]/2 - [winfo height .htmessagebox]/2]
  wm geometry .htmessagebox +$x+$y
  update
  wm deiconify .htmessagebox
  while {$htmessagebox == "-1"} {
    update
  }
  destroy .htmessagebox
  return $htmessagebox
  
}

proc FileSelectorBox_PopUpMenuButton {name text menu} {
  ##############
  # Procedure  : 
  # Description: 
  # Arguments  : 
  # Returns    : 
  # Sideeffects: the author is not familiar to any strange sideeffects 
  #              (shit happens)
  ##############
  global fileselect
  set fileselect(label_width) 15
  frame $name -relief raised -bd 3
  label $name.l -relief flat -width $fileselect(label_width) -anchor nw
  FileSelectorBox_SetMenuText $text
  set fileselect(currentmenu) $text
  frame $name.knopf  -width 14 -height 6 -relief raised -bd 2
  pack append $name \
    $name.l {left frame nw} \
    $name.knopf {right frame center padx 4}
  proc FileSelectorBox_GetMenuEntries {menu} {
    ##############
    # Procedure  : 
    # Description: 
    # Arguments  : 
    # Returns    : 
    # Sideeffects: the author is not familiar to any strange sideeffects 
    #              (shit happens)
    ##############
    set result ""
    for {set i 0} {$i <= [$menu index end]} {incr i} {
      lappend result [$menu entrycget $i -label]
    }
    return $result
  }
  proc FileSelectorBox_IsEqual {a b} {if {$a == $b} {return 1} {return 0}}
  proc FileSelectorBox_PostMenu {menu X Y y} {
    ##############
    # Procedure  : 
    # Description: 
    # Arguments  : 
    # Returns    : 
    # Sideeffects: the author is not familiar to any strange sideeffects 
    #              (shit happens)
    ##############
    global fileselect
    set tmpnum 0
    foreach j [FileSelectorBox_GetMenuEntries $menu] {
      if {[FileSelectorBox_IsEqual [set j] $fileselect(currentmenu)]} {
	set result [set tmpnum]
      } {
	incr tmpnum
      }
    }
    $menu post [expr $X -[winfo width $menu]/2] [expr $Y - $y - [$menu yposition [set result]]]
    after 50 "catch [grab $menu] tmp"
    focus $menu
    
  }
  foreach i [list $name $name.l $name.knopf] {
    bind $i <1> "FileSelectorBox_PostMenu $menu %X %Y %y"
    bind $i <ButtonRelease-1> "$menu unpost"
  }
}

proc FileSelectorBox_GetDirContens_Dir {dir pattern} {
  ##############
  # Procedure  : 
  # Description: 
  # Arguments  : 
  # Returns    : 
  # Sideeffects: the author is not familiar to any strange sideeffects 
  #              (shit happens)
  ##############
  global fileselect
  if {$fileselect(show_all_dirs)  == "1"} {
    set tmppattern "* .*"
  } else {
    set tmppattern $pattern
  }
  .fileselect configure -cursor watch
  update
  set tmp_list ""
  foreach j $tmppattern {
    foreach i [glob -nocomplain $dir/$j] {
      if {([file isdirectory $dir/[file tail $i]] == "1") && \
	    ([file tail $i] != ".") && ([file tail $i] != "..")} {
	lappend tmp_list "[file tail $i]/"
      }
    }
  }
  set tmp_list [lsort $tmp_list]
  .fileselect configure -cursor {}
  update
  return $tmp_list
}

proc FileSelectorBox_GetDirContens_File {dir pattern} {
  ##############
  # Procedure  : 
  # Description: 
  # Arguments  : 
  # Returns    : 
  # Sideeffects: the author is not familiar to any strange sideeffects 
  #              (shit happens)
  ##############
  global fileselect
  if {$fileselect(show_all_files)  == "1"} {
    set tmppattern "* .*"
  } else {
    set tmppattern $pattern
  }
  .fileselect configure -cursor watch
  update
  set tmp_list ""
  foreach j $tmppattern {
    foreach i [glob -nocomplain $dir/$j] {
      if {[file isfile $dir/[file tail $i]] == "1"} {
	lappend tmp_list "[file tail $i]"
      }
    }
  }
  set tmp_list [lsort $tmp_list]
  .fileselect configure -cursor {}
  update
  return $tmp_list
}

proc FileSelectorBox_FillListbox {listbox list} {
  ##############
  # Procedure  : 
  # Description: 
  # Arguments  : 
  # Returns    : 
  # Sideeffects: the author is not familiar to any strange sideeffects 
  #              (shit happens)
  ##############
  $listbox delete 0 end
  foreach i $list {
    $listbox insert end $i
  }
}

proc FileSelectorBox_FillEntry {entry text} {
  ##############
  # Procedure  : 
  # Description: 
  # Arguments  : 
  # Returns    : 
  # Sideeffects: the author is not familiar to any strange sideeffects 
  #              (shit happens)
  ##############
  $entry delete 0 end
  $entry insert 0 $text
  set tmp [$entry  xview]
  set tmp [expr [lindex $tmp 1] - [lindex $tmp 0]]
  $entry xview moveto $tmp
}

proc FileSelectorBox_SetColorsForWin {w} {
  ##############
  # Procedure  : setColorsForWin
  # Description: sets the colors for the window "w" and all its child-windows
  # Arguments  : w: window
  # Returns    : none
  # Sideeffects: the author is not familiar to any strange sideeffects 
  #              (shit happens)
  ##############
  $w configure -bg #dddddd
  set tmplist [winfo children $w]
  while {[llength $tmplist] != 0} {
    switch [winfo class [lindex $tmplist 0]] {
      Listbox {[lindex $tmplist 0] configure -bg #ebebeb\
		 -highlightthickness 0 \
		 -borderwidth 2 -relief ridge -selectmode browse
      }
      Scrollbar {[lindex $tmplist 0] configure -bg #9f9f9f \
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
		     -relief raised -bd 2 \
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
      default {[lindex $tmplist 0] configure -bg #ebebeb
      }
    }
    set tmplist [concat $tmplist [winfo children [lindex $tmplist 0]]]
    set tmplist [lreplace $tmplist 0 0]
  }
}

proc FileSelectorBox_MakeListboxAll {name {title ""}} {
  ##############
  # Procedure  : MakeListboxAll
  # Description: creates a listbox with the assoziated scrollbars
  # Arguments  : name: name of the frame of the listbox
  #              title: optional title of the listbox if it is empty
  #                     the title label will not be added
  # Returns    : none
  # Sideeffects: the author is not familiar to any strange sideeffects 
  #              (shit happens)
  ##############
  frame $name

  if {$title != ""} {
    label $name.l -text $title -anchor nw
    pack append $name $name.l {top frame nw}
  }
  
  scrollbar $name.vert_scroll \
    -command "$name.li yview"
  
  scrollbar $name.horz_scroll \
    -command "$name.li xview" \
    -orient horizontal \
    -width 10
  listbox $name.li \
    -width 1 \
    -height 1 \
    -xscrollcommand "$name.horz_scroll set"\
    -yscrollcommand "$name.vert_scroll set"

  pack append $name \
    $name.vert_scroll {right frame center filly} \
    $name.horz_scroll {bottom frame center fillx}  \
    $name.li {top frame center expand fill} 
}

proc FileSelectorBox_MakeEntryAll {name {title ""}} {
  ##############
  # Procedure  : MakeEntryAll
  # Description: creates an entry with the assoziated scrollbars
  # Arguments  : name: name of the frame of the entry
  #              title: optional title of the entry if it is empty
  #                     the title label will not be added
  # Returns    : none
  # Sideeffects: the author is not familiar to any strange sideeffects 
  #              (shit happens)
  ##############
  frame $name -relief ridge -borderwidth 2
  
  if {$title != ""} {
    label $name.l -text $title -anchor nw -pady 0
    pack append $name $name.l {top frame nw}
  }
  scrollbar $name.horz_scrollbar \
    -command "$name.e xview" \
    -orient {horizontal} \
    -relief  {sunken}
  
  entry $name.e \
    -relief {sunken} \
    -xscrollcommand "$name.horz_scrollbar set"
  pack append $name \
    $name.e {top frame center expand fillx} \
    $name.horz_scrollbar {bottom frame center expand fillx} 
}


proc FileSelectorBox_SetListboxes {num} {
  ##############
  # Procedure  : 
  # Description: 
  # Arguments  : 
  # Returns    : 
  # Sideeffects: the author is not familiar to any strange sideeffects 
  #              (shit happens)
  ##############
  global fileselect
  if {($num >= 0) && ($num < $fileselect(maxlayers))} {
    set fileselect(cur_num) $num
    .fileselect.choose.horz_scrollbar set $fileselect(maxlayers) 2 $num $num
    FileSelectorBox_FillListbox .fileselect.choose.l1.li $fileselect(layer_$num)
    FileSelectorBox_FillListbox .fileselect.choose.l2.li $fileselect(layer_[expr $num +1])
    FileSelectorBox_FillEntry .fileselect.choose.current.e  $fileselect(dir_[expr $num + 1])
  }
}

proc FileSelectorBox_Rescan {} {
  ##############
  # Procedure  : 
  # Description: 
  # Arguments  : 
  # Returns    : 
  # Sideeffects: the author is not familiar to any strange sideeffects 
  #              (shit happens)
  ##############
  global fileselect
  set tmpN $fileselect(maxlayers)
  set fileselect(layer_$tmpN) \
    [FileSelectorBox_GetDirContens_Dir $fileselect(dir_$tmpN) *]
  if {$fileselect(show_all_files)  == "1"} {
    .fileselect.misc.right.pattern.e configure -state disabled
    .fileselect.misc.right.pattern.l configure -text "Pattern:  all Files"
  } else {
    .fileselect.misc.right.pattern.e configure -state normal
    set fileselect(pattern) [.fileselect.misc.right.pattern.e get]
    .fileselect.misc.right.pattern.l configure -text "Pattern:  [.fileselect.misc.right.pattern.e get]"
  }
  set tmp_list [FileSelectorBox_GetDirContens_File $fileselect(dir_$tmpN) $fileselect(pattern)]
  foreach j $tmp_list {
    lappend fileselect(layer_$tmpN) "$j"
  }
  FileSelectorBox_SetListboxes [expr $tmpN-1]
}

proc FileSelectorBox_MakeItems {} {
  ##############
  # Procedure  : 
  # Description: 
  # Arguments  : 
  # Returns    : 
  # Sideeffects: the author is not familiar to any strange sideeffects 
  #              (shit happens)
  ##############
  global fileselect
  frame .fileselect.choose -bd 3 -relief ridge
  FileSelectorBox_MakeEntryAll .fileselect.choose.current "Current File:"
  
  .fileselect.choose.current.horz_scrollbar configure -width 10
  FileSelectorBox_MakeListboxAll .fileselect.choose.l1 
  FileSelectorBox_MakeListboxAll .fileselect.choose.l2 
  label .fileselect.choose.l -text "Status: normal..." -anchor w


  scrollbar .fileselect.choose.horz_scrollbar -orient horizontal \
    -command {FileSelectorBox_SetListboxes}
  pack append .fileselect.choose \
    .fileselect.choose.current {top fillx pady 4} \
    .fileselect.choose.l {top fillx} \
    .fileselect.choose.horz_scrollbar {bottom fillx pady 4} \
    .fileselect.choose.l1 {left expand fill} \
    .fileselect.choose.l2 {left expand fill} 
  
  frame .fileselect.action 
  button .fileselect.action.ok -text "Ok" -command {set fileselect(state) ok} -pady 0
  button .fileselect.action.dismiss -text "Dismiss" -command "set fileselect(state) dismiss"  -pady 0

  pack append .fileselect.action \
    .fileselect.action.ok {left frame center expand fillx padx 5} \
    .fileselect.action.dismiss {left frame center expand fillx padx 5}

  menu .fileselect.menu_begin_dir -tearoff 0 -relief raised -bd 2
  .fileselect.menu_begin_dir add command \
    -command {FileSelectorBox_GetDirStruct "~" $fileselect(pattern); \
		FileSelectorBox_SetMenuText Home}  \
    -underline -1 -label "Home"
  .fileselect.menu_begin_dir add command \
    -command {FileSelectorBox_GetDirStruct "/" $fileselect(pattern); \
		FileSelectorBox_SetMenuText Root} \
    -underline -1 -label "Root"
  .fileselect.menu_begin_dir add command \
    -command {FileSelectorBox_GetDirStruct $fileselect(start_dir) $fileselect(pattern); \
		FileSelectorBox_SetMenuText Current} \
    -underline -1 -label "Current"
  .fileselect.menu_begin_dir post 0 0
  .fileselect.menu_begin_dir unpost

  frame .fileselect.misc
  frame .fileselect.misc.left 
  
  checkbutton .fileselect.misc.left.dots_dir \
    -variable fileselect(show_all_dirs) -text "Show all Directories" -pady 0 -state normal \
    -command FileSelectorBox_Rescan -anchor nw
  set fileselect(show_all_dirs) 1
  checkbutton .fileselect.misc.left.dots \
    -variable fileselect(show_all_files) -text "Show all Files" -pady 0 -state normal \
    -command FileSelectorBox_Rescan -anchor nw
   button .fileselect.misc.left.expand -text "Expand"  -pady 0 -command {
     global fileselect
     set tmp [pwd]
     cd $fileselect(cur_dir)
     FileSelectorBox_GetDirStruct [pwd] $fileselect(pattern)
     cd $tmp
   }

  FileSelectorBox_PopUpMenuButton .fileselect.misc.left.begin_dir Current ".fileselect.menu_begin_dir"
  pack append .fileselect.misc.left \
    .fileselect.misc.left.dots_dir {top frame nw expand filly fillx pady 0} \
    .fileselect.misc.left.dots {top frame nw expand filly fillx pady 0} \
    .fileselect.misc.left.begin_dir {top frame w expand filly fillx pady 0} \
    .fileselect.misc.left.expand  {top frame nw expand filly fillx pady 0} 

  frame .fileselect.misc.right
  FileSelectorBox_MakeEntryAll .fileselect.misc.right.pattern "Pattern:"
  .fileselect.misc.right.pattern.horz_scrollbar configure -width 10
  button .fileselect.misc.right.rescan -text "Rescan" -command FileSelectorBox_Rescan  -pady 0 

  pack append .fileselect.misc.right \
      .fileselect.misc.right.pattern {top expand fill} \
      .fileselect.misc.right.rescan {top frame center expand fill} 

  pack append .fileselect.misc \
    .fileselect.misc.right {right expand fill} \
    .fileselect.misc.left {left filly frame w} 

  pack append .fileselect \
    .fileselect.choose {top frame center expand fill padx 4} \
    .fileselect.misc {top frame center fillx pady 15} \
    .fileselect.action {top frame center fillx}
}

proc FileSelectorBox_GetDirStruct {dir_name pattern} {
  ##############
  # Procedure  : FileSelectorBox_GetDirStruct
  # Description: Reads out the directory-structure
  # Arguments  : dir_name: the directory
  #              pattern: the pattern 
  # Returns    : none
  # Sideeffects: the author is not familiar to any strange sideeffects 
  #              (shit happens)
  ##############
  proc FileSelectorBox_Getdirname {name} {
    return [string range $name 0 [expr [string last "/" $name]-1]]
  }
  global fileselect
  if {[string last $dir_name "/"] != [expr [string length $dir_name]-1]} {
    set fileselect(cur_dir) "$dir_name/"
  } else {
    set fileselect(cur_dir) "$dir_name"
  }
  set cur_path $fileselect(cur_dir)
  set tmpnum 0
  while {([string length $cur_path] >= 1)} {
    set tmpS "[FileSelectorBox_Getdirname $cur_path]"
    set cur_path "$tmpS"
    incr tmpnum
  }
  set cur_path $fileselect(cur_dir)
  set fileselect(maxlayers) $tmpnum
  set fileselect(cur_num) [expr $tmpnum-1]
  set tmpS $cur_path

  while {([string length $cur_path] >= 1)} {
    set tmpS "[FileSelectorBox_Getdirname $cur_path]"
    set fileselect(layer_$tmpnum) [FileSelectorBox_GetDirContens_Dir $tmpS *]
    set fileselect(dir_$tmpnum) "$tmpS/"
    set cur_path "$tmpS"
    incr tmpnum -1
  }
  if {$tmpS == "/"} {
    set fileselect(layer_0) "$tmpS"
    set fileselect(dir_0) "$tmpS"
  } else {
    set fileselect(layer_0) "$tmpS/"
    set fileselect(dir_0) "$tmpS/"
  }
  set tmp_list [FileSelectorBox_GetDirContens_File $fileselect(cur_dir) $pattern]
  foreach j $tmp_list {
    lappend fileselect(layer_$fileselect(maxlayers)) "$j"
  }
  .fileselect.choose.horz_scrollbar set $fileselect(maxlayers) 2 \
    [expr $fileselect(maxlayers) - 1] [expr $fileselect(maxlayers) - 1]
  .fileselect.choose.l1.li delete 0 end
  .fileselect.choose.l2.li delete 0 end
  FileSelectorBox_FillListbox .fileselect.choose.l1.li $fileselect(layer_[expr $fileselect(maxlayers) -1])
  FileSelectorBox_FillListbox .fileselect.choose.l2.li $fileselect(layer_$fileselect(maxlayers))
  FileSelectorBox_FillEntry .fileselect.choose.current.e $fileselect(cur_dir)
}

proc FileSelectorBox_MakeBindings {} {
  bind .fileselect.choose.current <Enter> {
    global fileselect
    set fileselect(search_l) ""
    set fileselect(search_n) 0
    set fileselect(focus_to_entry) 1
    focus %W.e
    .fileselect.choose.l configure -text "Status: normal..."
    set fileselect(search) "no"
  }
  bind .fileselect.choose.current <Leave> {
    focus .fileselect
  }

  bind .fileselect.misc.right.pattern <Enter> {
    global fileselect; 
    focus %W.e
    .fileselect.choose.l configure -text "Status: normal..."
    set fileselect(search) "no"
  }
  bind .fileselect.misc.right.pattern <Leave> {
    focus .fileselect
  }
  bind .fileselect.misc.right.pattern.e <Return> {
    FileSelectorBox_Rescan
  }

  bind .fileselect <Control-s> {
    global fileselect
    set fileselect(search) "yes"
    set fileselect(search_l) ""
    set fileselect(search_n) 0
    .fileselect.choose.l2.li see 0
    .fileselect.choose.l2.li selection clear 0 end
    .fileselect.choose.l2.li selection set 0
    .fileselect.choose.l configure -text "Status: search..."
    FileSelectorBox_FillEntry .fileselect.choose.current.e \
      $fileselect(dir_[expr $fileselect(cur_num)+1])
    focus .fileselect
  }

  bind .fileselect <Return> {
    global fileselect
    if  {([focus] != ".fileselect.misc.right.pattern.e") && 
	 ([focus] != ".fileselect.choose.current.e") && 
	 ($fileselect(search) == "yes")} {
      
      set new_str [.fileselect.choose.l2.li get $fileselect(search_n)]
      FileSelectorBox_FillEntry .fileselect.choose.current.e  $fileselect(dir_[expr $fileselect(cur_num)+1])/$new_str
      set fileselect(state) ok
      break
    }
  }
  
  bind .fileselect <Alt-r> {
    .fileselect.misc.right.rescan invoke
  }
  bind .fileselect <Alt-e> {
    .fileselect.misc.left.expand invoke
  }
  bind .fileselect <Alt-f> {
    .fileselect.misc.left.dots invoke
  }
  bind .fileselect <Alt-d> {
    .fileselect.misc.left.dots_dir invoke
  }
  bind .fileselect <Alt-p> {
    global fileselect 
    focus .fileselect.misc.right.pattern.e
    .fileselect.choose.l configure -text "Status: normal..."
    set fileselect(search) "no"
  }
  bind .fileselect <KeyPress> {
    global fileselect
    if {"%K" == "Escape"} {
      set fileselect(state) dismiss
      break;
    }
    if  {([focus] != ".fileselect.misc.right.pattern.e") && 
	 ([focus] != ".fileselect.choose.current.e")} {
      switch %K {
	"Left" {
	  FileSelectorBox_SetListboxes [expr $fileselect(cur_num)-1]
	  .fileselect.choose.l configure -text "Status: normal..."
	  set fileselect(search) "no"
	  break
	}
	"Right" {
	  FileSelectorBox_SetListboxes [expr $fileselect(cur_num)+1]
	  .fileselect.choose.l configure -text "Status: normal..."
	  set fileselect(search) "no"
	  break
	}
	"Down" {
	  set tmpN [expr [.fileselect.choose.l2.li curselection]+1]
	  .fileselect.choose.l2.li see $tmpN
	  .fileselect.choose.l2.li selection clear 0 end
	  .fileselect.choose.l2.li selection set $tmpN
	  set fileselect(search_n) [.fileselect.choose.l2.li curselection]
	  set fileselect(search_l) [.fileselect.choose.l2.li get $fileselect(search_n)]
	  FileSelectorBox_FillEntry .fileselect.choose.current.e \
	    $fileselect(dir_[expr $fileselect(cur_num)+1])[.fileselect.choose.l2.li \
							   get $fileselect(search_n)]
	  break
	}
	"Up" {
	  set tmpN [expr [.fileselect.choose.l2.li curselection]-1]
	  .fileselect.choose.l2.li see $tmpN
	  .fileselect.choose.l2.li selection clear 0 end
	  .fileselect.choose.l2.li selection set $tmpN
	  set fileselect(search_n) [.fileselect.choose.l2.li curselection]
	  set fileselect(search_l) [.fileselect.choose.l2.li get $fileselect(search_n)]
	  FileSelectorBox_FillEntry .fileselect.choose.current.e \
	    $fileselect(dir_[expr $fileselect(cur_num)+1])[.fileselect.choose.l2.li \
							   get $fileselect(search_n)]
	  break
	}
      }
      if {$fileselect(search) == "yes"} {
	switch %K {
	  "BackSpace" {
	    set new_str [string range $fileselect(search_l) 0 \
			   [expr [string length $fileselect(search_l)] -2]]
	    set fileselect(search_n) 0
	    FileSelectorBox_FillEntry .fileselect.choose.current.e  \
	      $fileselect(dir_[expr $fileselect(cur_num)+1])/$new_str
	  }
	  "Tab" {
	    set new_str [.fileselect.choose.l2.li get $fileselect(search_n)]
	    if {[string last "/" $new_str] != "-1"} {
	      incr fileselect(cur_num)
	      FileSelectorBox_GetNewDirFromClick $new_str
	      FileSelectorBox_FillListbox .fileselect.choose.l1.li \
		$fileselect(layer_$fileselect(cur_num))
	      set fileselect(search) "yes"
	      set fileselect(search_l) ""
	      set fileselect(search_n) 0
	      .fileselect.choose.l2.li see 0
	      .fileselect.choose.l2.li selection clear 0 end
	      .fileselect.choose.l2.li selection set 0
	      set new_str ""
	      
	    }
	  }
	  "Control_L" -
	  "Control_R" {break}
	  default {
	    if {"%A" != "{}"} {
	      set new_str "$fileselect(search_l)%A"
	    } else {
	      set new_str "$fileselect(search_l)"
	    }
	  }
	}
	set tmpN [lsearch $fileselect(layer_[expr $fileselect(cur_num)+1]) "$new_str*"]
	if {$tmpN != "-1"} {
	  set fileselect(search_l) $new_str
	  set fileselect(search_n) $tmpN
	  .fileselect.choose.l2.li see $tmpN
	  .fileselect.choose.l2.li selection clear 0 end
	  .fileselect.choose.l2.li selection set $tmpN
	  FileSelectorBox_FillEntry .fileselect.choose.current.e  \
	    $fileselect(dir_[expr $fileselect(cur_num)+1])$new_str
	} else {
	  bell
	}
      }
    }
  }

  proc FileSelectorBox_GetNewDirFromClick {dirname} {
    global fileselect
    set tmpdir "$fileselect(dir_$fileselect(cur_num))$dirname"
    set fileselect(maxlayers) [expr $fileselect(cur_num)+1]
    set fileselect(layer_$fileselect(maxlayers)) \
      [FileSelectorBox_GetDirContens_Dir $tmpdir *]
    set tmp_list [FileSelectorBox_GetDirContens_File $tmpdir $fileselect(pattern)]
    foreach j $tmp_list {
      lappend fileselect(layer_$fileselect(maxlayers)) "$j"
    }
    .fileselect.choose.horz_scrollbar set $fileselect(maxlayers) 2 \
      $fileselect(cur_num) $fileselect(cur_num)
    FileSelectorBox_FillListbox .fileselect.choose.l2.li $fileselect(layer_$fileselect(maxlayers))
    FileSelectorBox_FillEntry .fileselect.choose.current.e  $tmpdir
    set fileselect(dir_[expr $fileselect(cur_num) + 1]) $tmpdir
    if {[lsearch -exact [FileSelectorBox_GetMenuEntries .fileselect.menu_begin_dir] $tmpdir] == -1}  {
      .fileselect.menu_begin_dir add command \
	-command "FileSelectorBox_GetDirStruct $tmpdir $fileselect(pattern);FileSelectorBox_SetMenuText $tmpdir" \
	-underline -1 -label "$tmpdir"
    }  
  }
  bind .fileselect.choose.l1.li <ButtonRelease-1> {
    FileSelectorBox_GetNewDirFromClick [%W get anchor]
  }
  
  bind .fileselect.choose.l2.li <ButtonRelease-1> {
    global fileselect
    set tmpS [%W get anchor]
    if {[string first "/" $tmpS] != "-1"} {
      incr fileselect(cur_num)
      FileSelectorBox_GetNewDirFromClick [%W get anchor]
      FileSelectorBox_FillListbox .fileselect.choose.l1.li $fileselect(layer_$fileselect(cur_num))
    } else {
      FileSelectorBox_FillEntry .fileselect.choose.current.e \
	"$fileselect(dir_[expr $fileselect(cur_num)+1])[%W get anchor]"
    }
  }
  bind .fileselect.choose.l2.li <Double-1> {
    global fileselect
    if {[string first "/" [%W get anchor]] == -1} {
      set fileselect(state) ok
    }
  }

}

proc FileSelectorBox_GetArgs {args} {
  global fileselect
  set fileselect(search) "no"
  set fileselect(expand) 0
  set fileselect(previous_settings) 0
  set fileselect(default_file) ""
  
  foreach i [lindex $args 0] {
    if {[llength $i] == "1"} {
      switch $i {
	expand {set fileselect(expand) "1"}
	previous_settings {set fileselect(previous_settings) "1"}
      }
    }
    if {[llength $i] == "2"} {
      switch [lindex $i 0] {
	default_file {set fileselect(default_file) [lindex $i 1]}
      }
    }
  }
}

proc FileSelectorBox {title pattern given_start_dir args} {
  ##############
  # Procedure  : 
  # Description: 
  # Arguments  : 
  # Returns    : 
  # Sideeffects: the author is not familiar to any strange sideeffects 
  #              (shit happens)
  ##############
  global fileselect
  global fileselectcolor
  FileSelectorBox_GetArgs $args

  set fileselect(focus_to_entry) 0
  if {[winfo exists .fileselect] == "0"} {
    set fileselect(previous_settings) "0"
    toplevel .fileselect
    wm minsize .fileselect 300 400
    wm geometry .fileselect 300x400+200+200
    FileSelectorBox_MakeItems
    FileSelectorBox_MakeBindings
    FileSelectorBox_SetColorsForWin .fileselect
    bind .fileselect.choose.current.e <Return> {set fileselect(state) ok;break}
  } {
    switch [wm state .fileselect] {
      iconic -
      withdrawn {catch "wm deiconify .fileselect"}
      default {}
    }
  }
  if {$fileselect(previous_settings) == "0"} {
    wm title .fileselect $title
    FileSelectorBox_FillEntry .fileselect.misc.right.pattern.e $pattern
    update
    if {[string first "/" $given_start_dir] != "-1"} {
      set start_dir $given_start_dir
    } else {
      set start_dir "$given_start_dir"
    }
    set fileselect(pattern) $pattern
    if {[file isdirectory $start_dir] == "1"} {
      set tmp [string range $start_dir 0 [expr [string last "/" $start_dir]-1]]
      if {$start_dir != $tmp} {
	set fileselect(start_dir) $start_dir
      } else {
	set fileselect(start_dir) $tmp
      }
      FileSelectorBox_GetDirStruct $fileselect(start_dir) $pattern
    } else {
      set fileselect(start_dir) [pwd]
      FileSelectorBox_GetDirStruct [pwd] $pattern
      puts stderr "FSBox: No valid Dir-Name ->$start_dir<-"
    }
    
    .fileselect.misc.right.pattern.l configure -text "Pattern:  [.fileselect.misc.right.pattern.e get]"
    set fileselect(search_l) ""
    set fileselect(search_n) 0
    if {$fileselect(expand)} {
      .fileselect.misc.left.expand invoke
    }
    if {$fileselect(default_file) != ""} {
      set tmpN [lsearch -exact $fileselect(layer_[expr $fileselect(cur_num)+1]) \
		  "$fileselect(default_file)"]
      if {$tmpN != "-1"} {
	.fileselect.choose.l2.li see $tmpN
	.fileselect.choose.l2.li selection clear 0 end
	.fileselect.choose.l2.li selection set $tmpN
	set fileselect(search_n) [.fileselect.choose.l2.li curselection]
	set fileselect(search_l) [.fileselect.choose.l2.li get $fileselect(search_n)]
      }
      FileSelectorBox_FillEntry .fileselect.choose.current.e \
	$fileselect(dir_[expr $fileselect(cur_num)+1])$fileselect(default_file)

    }
  }

  set fileselect(state) wait
  
#   while {$fileselect(state) == "wait"} {
#     update
#   }
  tkwait variable fileselect(state)
  set result [.fileselect.choose.current.e get]
  if {[file isdirectory [file dirname $result]] != "1"} {
    if {[FileSelectorBox_MessageBox \
	   "The Directory\n\n[file dirname $result]\n\ndoesn't exists!\nShall i create it?" \
	   "Attention" "Do it" "Forget it"] =="1"} {
      catch "exec sh -c \"mkdir -p [file dirname $result]\"" tmp
      if {$tmp != ""} {
	puts stderr  "FSBox: Start action-> $tmp"
      }
    }
  }
  wm withdraw .fileselect
  
  if {$fileselect(state) == "ok"} {
    return "[file dirname $result]/[file tail $result]"
  } {
    return "";
  }
}
