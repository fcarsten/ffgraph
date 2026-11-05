#!/usr/bin/env wish
# Program: finspector
# Tcl version: 7.3 (Tcl/Tk/XF)
# Tk version: 3.6
# XF version: 2.3
#

global FF_LIBDIR
source $FF_LIBDIR/tcl/xfmisc.tcl
source $FF_LIBDIR/tcl/colorBox.tcl

# procedure to show window .
proc ShowWindow. {args} {
    
  # Window manager configurations
  global tk_version
  wm positionfrom . user
  wm sizefrom . user
  wm maxsize . 600 600
  wm minsize . 100 100
  if {$tk_version >= 3.0} {
    wm protocol . WM_DELETE_WINDOW {recall deleteWindow}
  }
  wm title . {Graph Inspector}


  # build widget .insp_graph
  ttk::frame .insp_graph
  
  # build widget .insp_graph.number
  ttk::frame .insp_graph.number
  
  # build widget .insp_graph.number.nodes
  ttk::frame .insp_graph.number.nodes
  
  # build widget .insp_graph.number.nodes.label
  ttk::label .insp_graph.number.nodes.label \
    -anchor {w} \
    -text {Number of Nodes:} \
    -width {16}
  
  # build widget .insp_graph.number.nodes.entry
  ttk::entry .insp_graph.number.nodes.entry
  
  # pack widget .insp_graph.number.nodes
  pack append .insp_graph.number.nodes \
    .insp_graph.number.nodes.label {left frame center} \
    .insp_graph.number.nodes.entry {left frame center expand fillx}
  
  
  # build widget .insp_graph.number.edges
  ttk::frame .insp_graph.number.edges
  
  # build widget .insp_graph.number.edges.label
  ttk::label .insp_graph.number.edges.label \
    -anchor {w} \
    -text {Number of Edges:} \
    -width {16}
  
  # build widget .insp_graph.number.edges.entry
  ttk::entry .insp_graph.number.edges.entry
  
  # pack widget .insp_graph.number.edges
  pack append .insp_graph.number.edges \
    .insp_graph.number.edges.label {left frame center} \
    .insp_graph.number.edges.entry {left frame center expand fillx}
  
  # pack widget .insp_graph.number
  pack append .insp_graph.number \
    .insp_graph.number.nodes {top frame center expand fillx} \
    .insp_graph.number.edges {top frame center fillx}
  
  

  # build widget .insp_graph.type
  ttk::frame .insp_graph.type
  
  
  # build widget .insp_graph.type.graph
  ttk::frame .insp_graph.type.graph
  
  # build widget .insp_graph.type.graph.label1
  ttk::label .insp_graph.type.graph.label1 \
    -text {Graph Label:}
  
  # build widget .insp_graph.type.graph.value
  ttk::label .insp_graph.type.graph.value \
    -anchor {w} \
    -text {NONE}
  
  # build widget .insp_graph.type.graph.menubutton2
  ttk::menubutton .insp_graph.type.graph.menubutton2 \
    -menu {.insp_graph.type.graph.menubutton2.m} \
    -text {v}
  
  # build widget .insp_graph.type.graph.menubutton2.m
  menu .insp_graph.type.graph.menubutton2.m \
    -background {gainsboro} \
    -foreground {black} 
  
  # pack widget .insp_graph.type.graph
  pack append .insp_graph.type.graph \
    .insp_graph.type.graph.label1 {left frame center fill} \
    .insp_graph.type.graph.value {left frame center expand fill} \
    .insp_graph.type.graph.menubutton2 {right frame center fill}
  
  
  # build widget .insp_graph.type.nodes
  ttk::frame .insp_graph.type.nodes
  
  # build widget .insp_graph.type.nodes.label1
  ttk::label .insp_graph.type.nodes.label1 \
    -text {Node Label: }
  
  # build widget .insp_graph.type.nodes.value
  ttk::label .insp_graph.type.nodes.value \
    -anchor {w} \
    -text {NONE}
  
  # build widget .insp_graph.type.nodes.menubutton2
  ttk::menubutton .insp_graph.type.nodes.menubutton2 \
    -menu {.insp_graph.type.nodes.menubutton2.m} \
    -text {v}
  
  # build widget .insp_graph.type.nodes.menubutton2.m
  menu .insp_graph.type.nodes.menubutton2.m \
    -background {gainsboro} \
    -foreground {black} 
  
  # pack widget .insp_graph.type.nodes
  pack append .insp_graph.type.nodes \
    .insp_graph.type.nodes.label1 {left frame center fill} \
    .insp_graph.type.nodes.value {left frame center expand fill} \
    .insp_graph.type.nodes.menubutton2 {right frame center fill}
  
  
  # build widget .insp_graph.type.edges
    ttk::frame .insp_graph.type.edges
  
  # build widget .insp_graph.type.edges.label1
  ttk::label .insp_graph.type.edges.label1 \
    -text {Edge Label: }
  
  # build widget .insp_graph.type.edges.value
  ttk::label .insp_graph.type.edges.value \
    -anchor {w} \
    -text {NONE}
  
  # build widget .insp_graph.type.edges.menubutton2
  ttk::menubutton .insp_graph.type.edges.menubutton2 \
    -menu {.insp_graph.type.edges.menubutton2.m} \
    -text {v}
  
  # build widget .insp_graph.type.edges.menubutton2.m
  menu .insp_graph.type.edges.menubutton2.m \
    -background {gainsboro} \
    -foreground {black} 
  
  # pack widget .insp_graph.type.edges
  pack append .insp_graph.type.edges \
    .insp_graph.type.edges.label1 {left frame center fill} \
    .insp_graph.type.edges.value {left frame center expand fill} \
    .insp_graph.type.edges.menubutton2 {right frame center fill}
  
  # pack widget .insp_graph.type
  pack append .insp_graph.type \
    .insp_graph.type.graph {top frame center expand fillx} \
    .insp_graph.type.nodes {top frame center expand fillx} \
    .insp_graph.type.edges {top frame center fillx}
  
  # pack widget .insp_graph
  pack append .insp_graph \
    .insp_graph.number {top frame center expand fill} \
    .insp_graph.type {top frame center expand fill}
  
  ttk::frame .insp_element
  
  ttk::frame .insp_element.info
  
  ttk::frame .insp_element.info.labels
  
  ttk::label .insp_element.info.current \
    -text {Currently selected}
  
  ttk::label .insp_element.info.labels.name \
    -anchor {w} \
    -width {12}
  
  ttk::label .insp_element.info.labels.color \
    -anchor {w} \
    -width {12}
  
  pack append .insp_element.info.labels \
    .insp_element.info.labels.name {top frame center fillx} \
    .insp_element.info.labels.color {top frame center fillx}
  
  ttk::frame .insp_element.info.entries
  
  ttk::entry .insp_element.info.entries.name
  
  ttk::entry .insp_element.info.entries.color
  
  pack append .insp_element.info.entries \
    .insp_element.info.entries.name {top frame center expand fillx} \
    .insp_element.info.entries.color {top frame center expand fillx}
  
  pack append .insp_element.info \
    .insp_element.info.current {top frame center fillx} \
    .insp_element.info.labels {left frame center fill} \
    .insp_element.info.entries {left frame center expand fill}
  
  pack append .insp_element \
    .insp_element.info {top frame center fill} 
  
  showSyntax 
  
  # build widget .buttons
  ttk::frame .buttons
  
  # build widget .buttons.special
  ttk::frame .buttons.special
  
  # build widget .buttons.special.dismiss
  ttk::button .buttons.special.dismiss \
    -command {recall deleteWindow} \
    -text {Dismiss}
  
  # pack widget .buttons.special
  pack append .buttons.special \
    .buttons.special.dismiss {left frame center expand fillx}
  
  # pack widget .buttons
  pack append .buttons \
    .buttons.special {top frame center expand fillx}
  
  # pack widget .
  pack append . \
    .insp_graph {top frame center expand fill} \
    .insp_element {top frame center expand fill} \
    .buttons {bottom frame center fillx} 
  
  #  .insp_graph.nodes.number.entry insert end {}
  #  .insp_graph.edges.number.entry insert end {}
}


# User defined procedures

# Procedure: FillElement
proc FillElement { kind} {
  if {$kind == "Group"} {
    .insp_element.info.labels.name configure -text "\# of Elements:"
  } {
    .insp_element.info.labels.name configure -text "$kind Name:"
  }
  .insp_element.info.labels.color configure -text "$kind Color:"
  .insp_element.syntax.label configure -text "$kind Syntax:"
}


# Procedure: HideElement
proc HideElement {} {
  global insp_element
  pack forget .insp_element.syntax
}


# Procedure: OptionButtonGet
proc OptionButtonGet { widget} {
  if {"[winfo class $widget.value]" in {Label TLabel}} {
    return [lindex [$widget.value config -text] 4]
  } {
    if {"[winfo class $widget.value]" in {Entry TEntry}} {
      return [$widget.value get]
    }
  }
}


# Procedure: OptionButtonSet
proc OptionButtonSet { widget valueWidget} {
  set derText [lindex [$widget entryconfig \
                         [$widget index active] -label] 4]
  
  if {"[winfo class $valueWidget.value]" in {Label TLabel}} {
    $valueWidget.value config -text $derText
  } {
    if {"[winfo class $widget.value]" in {Entry TEntry}} {
      $valueWidget.value delete 0 end
      $valueWidget.value insert 0 $derText
    }
  }
  recall setMenu $widget $derText
}

# Procedure: addMenu
proc addMenu { widget valueWidget text inspFlag} {
  if {$inspFlag == "NO"} {
    set inspFlag disabled
  } {
    set inspFlag normal
  }
  
  $widget add command \
    -command "OptionButtonSet $widget $valueWidget" \
    -label $text \
    -state $inspFlag
}

proc addSubMenu { widget newEntry newName} {
  if {[winfo exists $widget.$newEntry] == 0} {
    menu $widget.$newEntry
  }
  $widget add cascade -label $newName -menu $widget.$newEntry
}

proc clearMenu {widget} {
  $widget delete 0 last
}

# Procedure: ShowEdge
proc ShowEdge { name syntax color {type "NONE"}} {

  case $type in {
    {NONE Group} {
      pack forget .insp_element.syntax
    } {default} {
      pack append .insp_element \
        .insp_element.syntax {top frame center expand fill}
    }
  }
  
  FillElement Edge
  
  .insp_element.syntax.buttons.cancel configure -command {
    recall EdgeCancel
  }
  
  .insp_element.syntax.buttons.apply configure -command {
    recall EdgeApply [.insp_element.syntax.frame.text get 1.0 end]
  }
  
  bind .insp_element.info.entries.name <Key-Return> {
    recall EdgeName [%W get]
  }
  bind .insp_element.info.entries.color <Key-Return> {
    recall EdgeColor [%W get]
  }
  bind .insp_element.info.entries.color <Double-Button-3> {
    set tmpC [ColorBox [%W get]]
    %W delete 0 end
    %W insert insert $tmpC
    recall EdgeColor $tmpC
  }
  
  setName $name
  setColor $color
  setSyntax $syntax
}

# Procedure: ShowGroup
proc ShowGroup { name syntax color {type "NONE"}} {

  case $type in {
    {NONE Group} {
      pack forget .insp_element.syntax
    } {default} {
      pack append .insp_element \
        .insp_element.syntax {top frame center expand fill}
    }
  }
  
  FillElement Group
  
  .insp_element.syntax.buttons.cancel configure -command {
    recall GroupCancel
  }
  
  .insp_element.syntax.buttons.apply configure -command {
    recall GroupApply [.insp_element.syntax.frame.text get 1.0 end]
  }
  
  bind .insp_element.info.entries.name <Key-Return> {
    recall GroupName [%W get]
  }
  bind .insp_element.info.entries.color <Key-Return> {
    recall GroupColor [%W get]
  }
  bind .insp_element.info.entries.color <Double-Button-3> {
    set tmpC [ColorBox [%W get]]
    %W delete 0 end
    %W insert insert $tmpC
    recall GroupColor $tmpC
  }
  
  setName $name
  setColor $color
  setSyntax ""
}

# Procedure: showSyntax
proc showSyntax { } {
    ###########
    # Procedure: showSyntax
    # Author:   Barbara Zuendorf
    # Last Update: 15.7.94
    # Description: Builds the frame .insp_element
    # Arguments: none
    # Returns: none
    # Sideeffects: none
    ###########
    ttk::frame .insp_element.syntax


    ttk::label .insp_element.syntax.label \
            -anchor {w} \
            -text {Syntax:} \
            -width {12}

    ttk::frame .insp_element.syntax.frame

    ttk::scrollbar .insp_element.syntax.frame.scrollbar1 \
            -command {.insp_element.syntax.frame.text yview}

    text .insp_element.syntax.frame.text \
            -borderwidth {2} \
            -font {-Adobe-Helvetica-Medium-R-Normal--*-120-*} \
            -height {8} \
            -relief {ridge} \
            -width {40} \
            -wrap {word} \
            -yscrollcommand {.insp_element.syntax.frame.scrollbar1 set}

    # pack widget .insp_element.syntax.frame
    pack append .insp_element.syntax.frame \
            .insp_element.syntax.frame.scrollbar1 {right frame center filly} \
            .insp_element.syntax.frame.text {top frame center expand fill}

    ttk::frame .insp_element.syntax.buttons

    ttk::button .insp_element.syntax.buttons.cancel \
            -command {recall "cancel"} \
            -text {Cancel syntax}
    
    ttk::button .insp_element.syntax.buttons.apply \
            -command {} \
            -text {Apply syntax}

    pack append .insp_element.syntax.buttons \
            .insp_element.syntax.buttons.apply {left frame center expand fill}\
            .insp_element.syntax.buttons.cancel {left frame center expand fill}

    pack append .insp_element.syntax  \
            .insp_element.syntax.label {left frame nw}\
            .insp_element.syntax.frame {top frame center expand fill} \
            .insp_element.syntax.buttons {bottom frame center fill}

}


# Procedure: ShowGraph
proc ShowGraph { name syntax color {type "NONE"} } {

  case $type in {
    {NONE Group} {
      pack forget .insp_element.syntax
    } {default} {
      pack append .insp_element \
        .insp_element.syntax {top frame center expand fill}
    }
  }

  FillElement Graph
  
  .insp_element.syntax.buttons.cancel configure -command {
    recall GraphCancel
  }
    
  .insp_element.syntax.buttons.apply configure -command {
    recall GraphApply [.insp_element.syntax.frame.text get 1.0 end]
  }
  
  bind .insp_element.info.entries.name <Key-Return> {
    recall GraphName [%W get]
  }
  bind .insp_element.info.entries.color <Key-Return> {
    recall GraphColor [%W get]
  }
  bind .insp_element.info.entries.color <Double-Button-3> {
    set tmpC [ColorBox [%W get]]
    %W delete 0 end
    %W insert insert $tmpC
    recall GraphColor $tmpC
  }
  
  setName $name
  setColor $color
  setSyntax $syntax
}


# Procedure: ShowNode
proc ShowNode { name syntax color {type "NONE"}} {
	
  case $type in {
    {NONE Group} {
      pack forget .insp_element.syntax
    } {default} {
      pack append .insp_element \
        .insp_element.syntax {top frame center expand fill}
    }
  }

  FillElement Node
  
  .insp_element.syntax.buttons.cancel configure -command {
    recall NodeCancel
  }
  
  .insp_element.syntax.buttons.apply configure -command {
    recall NodeApply [.insp_element.syntax.frame.text get 1.0 end]
  }
  
  bind .insp_element.info.entries.name <Key-Return> {
    recall NodeName [%W get]
  }
  bind .insp_element.info.entries.color <Key-Return> {
    recall NodeColor [%W get]
  }
  bind .insp_element.info.entries.color <Double-Button-3> {
    set tmpC [ColorBox [%W get]]
    %W delete 0 end
    %W insert insert $tmpC
    recall NodeColor $tmpC
  }
  setName $name
  setColor $color
  setSyntax $syntax
}

# Procedure: UpdateGraph
proc UpdateGraph { node_number edge_number} {
  setNumNodes $node_number
  setNumEdges $edge_number
}


# Procedure: setColor
proc setColor { color} {
  .insp_element.info.entries.color delete 0 end
  .insp_element.info.entries.color insert end $color
}

# Procedure: setName
proc setName { name} {
  .insp_element.info.entries.name delete 0 end
  .insp_element.info.entries.name insert end $name
}


# Procedure: setNumEdges
proc setNumEdges { text} {
  .insp_graph.number.edges.entry configure -state normal
  .insp_graph.number.edges.entry delete 0 end
  .insp_graph.number.edges.entry insert end $text
  .insp_graph.number.edges.entry configure -state disabled
}


# Procedure: setNumNodes
proc setNumNodes { text} {
  .insp_graph.number.nodes.entry configure -state normal
  .insp_graph.number.nodes.entry delete 0 end
  .insp_graph.number.nodes.entry insert end $text
  .insp_graph.number.nodes.entry configure -state disabled
}


# Procedure: setSyntax
proc setSyntax { syntax} {
  .insp_element.syntax.frame.text delete 1.0 end
  .insp_element.syntax.frame.text insert insert $syntax
}


# Internal procedures


# initialize global variables
proc InitGlobals {} {
}

# initialize global variables
InitGlobals

# display/remove toplevel windows.
ShowWindow.

# eof
#

