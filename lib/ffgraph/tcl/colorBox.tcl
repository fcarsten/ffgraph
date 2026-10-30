# procedures to show a ColorBox in which to choose
# adapted from Sven Delmas

# last change 24.12


# Procedure: ClearList
proc ClearList { listWidget} {
  if {[$listWidget size] > 0} {
    $listWidget delete 0 end
  }
}

proc NoFunction {} {
}

# Procedure: ColorBox
proc ColorBox {{defName "black"} \
                  {colorBoxMessage "Color"}} {
  set colorBoxFileColor /usr/share/X11/rgb.txt
  global colorBox

  set colorBox(colorName) $defName
  set colorBox(paletteNr) 0


  # look if there is already a color window
  if {"[info commands .colorBox]" != ""} {
    .colorBox.palette.palette$colorBox(paletteNr) configure -relief raised
    wm iconify .colorBox
    wm deiconify .colorBox
  } {
    # get color file name
    if {!([file exists $colorBoxFileColor] &&
          [file readable $colorBoxFileColor])} {
      set colorBoxFileColor ""
    }
    # build widget structure

    toplevel .colorBox -borderwidth 0
    wm geometry .colorBox 400x250
    wm title .colorBox {Color box}
    wm maxsize .colorBox 1000 1000
    wm minsize .colorBox 100 100
    # end build of toplevel
    
    frame .colorBox.frame1 -borderwidth 0 -relief raised
    button .colorBox.frame1.ok -text "OK"
    button .colorBox.frame1.cancel -text "Cancel"
    frame .colorBox.frame2 -borderwidth 0 -relief raised
    radiobutton .colorBox.frame2.rgb\
      -command "ColorBoxShowSlides"\
      -text "RGB"\
      -value "RGB" \
      -variable colorBox(type)
    
    radiobutton .colorBox.frame2.hsv\
      -command "ColorBoxShowSlides"\
      -text "HSV"\
      -value "HSV"\
      -variable colorBox(type)
    
    radiobutton .colorBox.frame2.list\
      -command "ColorBoxShowSlides "\
      -text "List"\
      -value "List"\
      -variable colorBox(type)
    
    frame .colorBox.palette -borderwidth 0 -relief raised
    

    scale .colorBox.red \
      -background "red" \
      -showvalue 0\
      -from 0\
      -label "Red"\
      -orient horizontal\
      -relief raised\
      -sliderlength 15\
      -to 255\
      -width 8
    
    scale .colorBox.green\
      -background "green"\
      -showvalue 0\
      -from 0\
      -label "Green"\
      -orient horizontal\
      -relief raised\
      -sliderlength 15\
      -to 255\
      -width 8
    
    scale .colorBox.blue\
      -background "blue"\
      -showvalue 0\
      -from 0\
      -label "Blue"\
      -orient horizontal\
      -relief raised\
      -sliderlength 15\
      -to 255\
      -width 8
    
    scale .colorBox.h\
      -from 0\
      -showvalue 0\
      -label "Hue"\
      -orient horizontal\
      -relief raised\
      -sliderlength 15\
      -to 1000\
      -width 8
    
    scale .colorBox.s\
      -from 0\
      -showvalue 0\
      -label "Saturation * 100"\
      -orient horizontal\
      -relief raised\
      -sliderlength 15\
      -to 1000\
      -width 8
    
    scale .colorBox.v\
      -from 0\
      -showvalue 0\
      -label "Value"\
      -orient horizontal\
      -relief raised\
      -sliderlength 15\
      -to 1000\
      -width 8
    
    label .colorBox.demo\
      -relief raised\
      -text "This text shows the results"
    
    frame .colorBox.current\
      -borderwidth 0\
      -relief raised
    
    label .colorBox.current.labelcurrent\
      -relief raised
    
    entry .colorBox.current.current\
      -relief raised
    
    frame .colorBox.colors       -borderwidth 0       -relief raised
    
    scrollbar .colorBox.colors.vscroll\
      -relief raised\
      -command ".colorBox.colors.colors yview"
    
    scrollbar .colorBox.colors.hscroll\
      -orient horizontal\
      -relief raised\
      -command ".colorBox.colors.colors xview"
    
    listbox .colorBox.colors.colors\
      -exportselection false\
      -relief raised\
      -xscrollcommand ".colorBox.colors.hscroll set"\
      -yscrollcommand ".colorBox.colors.vscroll set"
    

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
        global list2rgb
        global rgb2list
        close $colorInFile
        foreach colorLine [split $colorReadList "\n"] {
          if { ([string trim $colorLine] != "") && 
               ([regexp {^!} $colorLine] != 1)} {
            set colorNewLine [lrange $colorLine 3 end]
            set colorNewLine2  [format \#%02x [lindex $colorLine 0]]
            append colorNewLine2 [format %02x [lindex $colorLine 1]]
            append colorNewLine2 [format %02x [lindex $colorLine 2]]
            set list2rgb($colorNewLine) $colorNewLine2
            set rgb2list($colorNewLine2) $colorNewLine
            .colorBox.colors.colors insert end "$colorNewLine $colorNewLine2"}
        }
      }
    }
    # bindings
    bind .colorBox.colors.colors <ButtonPress-1> "
      ColorBoxSelectColor %W %y"
    bind .colorBox.colors.colors <Button1-Motion> "
      ColorBoxSelectColor %W %y"
    bind .colorBox.colors.colors <Shift-ButtonPress-1> "
      ColorBoxSelectColor %W %y"
    bind .colorBox.colors.colors <Shift-Button1-Motion> "
      ColorBoxSelectColor %W %y"
    bind .colorBox.colors.colors <Double-1> "
    ColorBoxSelectColor %W %y
    global colorBox
    set colorBox(colorName) \[.colorBox.current.current get\]"
  
    .colorBox.frame1.ok config -command "
      global colorBox
      set colorBox(colorName) \[.colorBox.current.current get\]"
    
    .colorBox.frame1.cancel config     -command "
      global colorBox
      set colorBox(colorName) {} "
      

    .colorBox.red config -command "ColorBoxSetRGBColor"
      
    .colorBox.green config -command "ColorBoxSetRGBColor"
      
    .colorBox.blue config -command "ColorBoxSetRGBColor"
      
    .colorBox.h config -command "ColorBoxSetHSVColor"
      
    .colorBox.s config -command "ColorBoxSetHSVColor"
      
    .colorBox.v config -command "ColorBoxSetHSVColor"
      
    .colorBox.current.labelcurrent config -text "$colorBoxMessage:"
      
    # bindings
    bind .colorBox.current.current <Return> "
    ColorBoxSetPaletteList \[.colorBox.current.current get\]
    ColorBoxSetColor text \[.colorBox.current.current get\]"
      
    # set up current value
    .colorBox.current.current delete 0 end
    
    # packing
    pack append .colorBox.frame1\
      .colorBox.frame1.ok {left fill expand}\
      .colorBox.frame1.cancel {left fill expand}
    
    pack append .colorBox.frame2\
      .colorBox.frame2.rgb {left fill expand}\
      .colorBox.frame2.hsv {left fill expand}\
      .colorBox.frame2.list {left fill expand}
    
    pack append .colorBox.current\
      .colorBox.current.labelcurrent {left}\
      .colorBox.current.current {left fill expand}
    
    pack append .colorBox.colors\
      .colorBox.colors.vscroll {left filly}\
      .colorBox.colors.hscroll {bottom fillx}\
      .colorBox.colors.colors {left fill expand}
    ffSetColorsForWin .colorBox
    set counter 0
    foreach element $colorBox(palette) {
      button .colorBox.palette.palette$counter\
        -command "ColorBoxSetPalette $counter"\
        -width 3
      .colorBox.palette.palette$counter config \
        -activebackground $element\
        -background $element
      
      pack append .colorBox.palette\
        .colorBox.palette.palette$counter {left fill expand}
      incr counter
    }
        
  }

  .colorBox.frame2.rgb select
  ColorBoxShowSlides  
  
  # wait for the box to be destroyed
  update idletask

  # grab .colorBox
  tkwait variable colorBox(colorName)

  .colorBox.palette.palette$colorBox(paletteNr) configure -relief raised
  wm withdraw .colorBox
  
  if {$colorBox(colorName) != ""} {
    return $colorBox(colorName)
  } {
    return $defName
  }
}
  
proc ColorBoxRGBToList {r g b} {
  global list2rgb;
  if { catch{set retVal $list2rgb($tmpVal)} != 1}{
    return $retVal
  }
 return "black"
}

proc ColorBoxListToRGB {val} {
  global rgb2list;
  set tmpVal $r
  append tmpVal $g
  append tmpVal $b
  if { catch{set retVal $rgb2list($tmpVal)} != 1}{
    return $retVal
  }
 return "0 0 0"
}
  
# Procedure: ColorBoxHSVToRGB
proc ColorBoxHSVToRGB { colorBoxHue colorBoxSat colorBoxValue} {
# xf ignore me 6

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
    set colorBoxP [format %.0f [expr {65535.0* \
                                        $colorBoxValue*(1 - $colorBoxSat)}]]
    set colorBoxQ [format %.0f [expr {65535.0* \
                                        $colorBoxValue* \
                                        (1 - ($colorBoxSat*$colorBoxF))}]]
    set colorBoxT [format %.0f [expr {65535.0*\
                                        $colorBoxValue*\
                                        (1 - ($colorBoxSat*\
                                                (1 - $colorBoxF)))}]]
    case $i       0 {
      return "$colorBoxV $colorBoxT $colorBoxP"
    } 1 {
      return "$colorBoxQ $colorBoxV $colorBoxP"
    } 2 {
      return "$colorBoxP $colorBoxV $colorBoxT"
    } 3 {
      return "$colorBoxP $colorBoxQ $colorBoxV"
    } 4 {
      return "$colorBoxT $colorBoxP $colorBoxV"
    } 5 {
      return "$colorBoxV $colorBoxP $colorBoxQ"}
    error "i value $i is out of range"
  }
}


# Procedure: ColorBoxRGBToHSV
proc ColorBoxRGBToHSV { colorBoxRed colorBoxGreen colorBoxBlue} {
# xf ignore me 6

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
proc ColorBoxSelectColor { colorW colorY} {
# xf ignore me 6

  set colorNearest [$colorW nearest $colorY]
  if {$colorNearest >= 0} {
    $colorW select set $colorNearest
    set colorTmpValue [$colorW get $colorNearest]
    set colorCurrentColor [lrange $colorTmpValue 0 \
                             [expr [llength $colorTmpValue]-2]]
    set colorCurrentValue [lrange $colorTmpValue\
                             [expr [llength $colorTmpValue]-1] end]

    scan [string range $colorCurrentValue 1 2] "%x" colorBoxValue
    .colorBox.red set $colorBoxValue
    scan [string range $colorCurrentValue 3 4] "%x" colorBoxValue
    .colorBox.green set $colorBoxValue
    scan [string range $colorCurrentValue 5 6] "%x" colorBoxValue
    .colorBox.blue set $colorBoxValue

    .colorBox.current.current delete 0 end
    .colorBox.current.current insert 0 $colorCurrentColor
    ColorBoxSetColor list $colorCurrentColor
    ColorBoxSetPaletteList $colorCurrentColor
  }
}


# Procedure: ColorBoxSetColor
proc ColorBoxSetColor {colorBoxType colorBoxValue} {
# xf ignore me 6
  global colorBox

  .colorBox.red config     -command "NoFunction"
  .colorBox.green config     -command "NoFunction"
  .colorBox.blue config     -command "NoFunction"
  .colorBox.h config     -command "NoFunction"
  .colorBox.s config     -command "NoFunction"
  .colorBox.v config     -command "NoFunction"

  set colorBoxSetColor ""
  if {"$colorBoxValue" != ""} {
    if {"$colorBoxType" != "text"} {
      .colorBox.current.current delete 0 end
      .colorBox.current.current insert 0 $colorBoxValue
    }
    .colorBox.demo config -background $colorBoxValue
  }
  case $colorBoxType in {
    {text palette} {
      set red [expr [lindex [winfo rgb .colorBox.demo \
                               [lindex [.colorBox.demo config -background]\
                                  4]] 0]/256]
      set green [expr [lindex [winfo rgb .colorBox.demo\
                                 [lindex [.colorBox.demo config -background]\
                                    4]] 1]/256]
      set blue [expr [lindex [winfo rgb .colorBox.demo\
                                [lindex [.colorBox.demo config -background]\
                                   4]] 2]/256]
      if {"$colorBox(type)" == "RGB"} {
        .colorBox.red set $red
        .colorBox.green set $green
        .colorBox.blue set $blue
      }
      if {"$colorBox(type)" == "HSV"} {
        set colorBoxHSV [ColorBoxRGBToHSV [expr $red*256]\
                           [expr $green*256] [expr $blue*256]]
        .colorBox.h set [format %.0f [expr [lindex $colorBoxHSV 0]*1000.0]]
        .colorBox.s set [format %.0f [expr [lindex $colorBoxHSV 1]*1000.0]]
        .colorBox.v set [format %.0f [expr [lindex $colorBoxHSV 2]*1000.0]]
      }
    }
  }
  .colorBox.red config -command "ColorBoxSetRGBColor"
  .colorBox.green config -command "ColorBoxSetRGBColor"
  .colorBox.blue config -command "ColorBoxSetRGBColor"
  .colorBox.h config -command "ColorBoxSetHSVColor"
  .colorBox.s config -command "ColorBoxSetHSVColor"
  .colorBox.v config -command "ColorBoxSetHSVColor"
}




# Procedure: ColorBoxSetPalette
proc ColorBoxSetPalette { colorBoxElement} {
# xf ignore me 6
  global colorBox

  .colorBox.palette.palette$colorBox(paletteNr) configure -relief raised
  .colorBox.palette.palette$colorBoxElement configure -relief sunken

  set colorBox(paletteNr) $colorBoxElement
  ColorBoxSetColor palette\
    [lindex [.colorBox.palette.palette$colorBoxElement config -background] 4]
}


# Procedure: ColorBoxSetPaletteList
proc ColorBoxSetPaletteList { colorBoxValue} {
# xf ignore me 6
  global colorBox

  .colorBox.palette.palette$colorBox(paletteNr) config\
    -activebackground $colorBoxValue
  .colorBox.palette.palette$colorBox(paletteNr) config\
    -background $colorBoxValue
  set colorBox(palette) [lreplace $colorBox(palette) $colorBox(paletteNr) \
                           $colorBox(paletteNr) $colorBoxValue]
}


# Procedure: ColorBoxSetHSVColor
proc ColorBoxSetHSVColor {colorBoxValue} {
# xf ignore me 6
  global colorBox

  set colorBoxRGB [ColorBoxHSVToRGB [expr [.colorBox.h get]/1000.0] \
                     [expr [.colorBox.s get]/1000.0]\
                     [expr [.colorBox.v get]/1000.0]]
  ColorBoxSetColor hsv [format \#%04x%04x%04x [lindex $colorBoxRGB 0]\
                          [lindex $colorBoxRGB 1] [lindex $colorBoxRGB 2]] 

  ColorBoxSetPaletteList [format \#%04x%04x%04x [lindex $colorBoxRGB 0]\
                            [lindex $colorBoxRGB 1] [lindex $colorBoxRGB 2]] 
}

# Procedure: ColorBoxSetRGBColor
proc ColorBoxSetRGBColor { colorBoxValue} {
# xf ignore me 6
  global colorBox

  ColorBoxSetColor rgb [format \#%02x%02x%02x [.colorBox.red get] \
                          [.colorBox.green get] [.colorBox.blue get]] 
  ColorBoxSetPaletteList [format \#%02x%02x%02x [.colorBox.red get] \
                            [.colorBox.green get] [.colorBox.blue get]] 
}


# Procedure: ColorBoxShowSlides
proc ColorBoxShowSlides {} {
# xf ignore me 6
  global colorBox

  pack unpack .colorBox.h
  pack unpack .colorBox.s
  pack unpack .colorBox.v
  pack unpack .colorBox.red
  pack unpack .colorBox.green
  pack unpack .colorBox.blue
  pack unpack .colorBox.colors

  case $colorBox(type) in {
    {RGB} {
      pack append .colorBox \
        .colorBox.frame1 {bottom fillx}\
        .colorBox.frame2 {bottom fillx}\
        .colorBox.current {bottom fillx}\
        .colorBox.palette {bottom fillx}\
        .colorBox.red {top fillx}\
        .colorBox.green {top fillx}\
        .colorBox.blue {top fillx}\
        .colorBox.demo {bottom fill expand}
    }
    {HSV} {
      pack append .colorBox \
        .colorBox.frame1 {bottom fillx} \
        .colorBox.frame2 {bottom fillx} \
        .colorBox.current {bottom fillx} \
        .colorBox.palette {bottom fillx} \
        .colorBox.h {top fillx} \
        .colorBox.s {top fillx} \
        .colorBox.v {top fillx} \
        .colorBox.demo {bottom fill expand}
    }
    {List} {
      pack append .colorBox \
        .colorBox.frame1 {bottom fillx} \
        .colorBox.frame2 {bottom fillx} \
        .colorBox.current {bottom fillx} \
        .colorBox.palette {bottom fillx} \
        .colorBox.demo {bottom fillx} \
        .colorBox.colors {top fill expand}
    }
  }
  ColorBoxSetColor text [lindex [.colorBox.demo config -background] 4]
}


# Procedure: SelectColor
proc SelectColor {} {

  set selColor [ColorBox]
  if {"$selColor" != ""} {
    [SymbolicName ResourceValue] delete 1.0 end
    [SymbolicName ResourceValue] insert end $selColor
  }
}

global {colorBox}
set {colorBox(activeBackground)} {}
set {colorBox(activeForeground)} {}
set {colorBox(background)} {}
set {colorBox(colorName)} {}
set {colorBox(font)} {}
set {colorBox(foreground)} {}
set {colorBox(palette)} {white black gray50 blue red green yellow orange}
set {colorBox(paletteNr)} {0}
set {colorBox(scrollActiveForeground)} {}
set {colorBox(scrollBackground)} {}
set {colorBox(scrollForeground)} {}
set {colorBox(scrollSide)} {left}
set {colorBox(type)} {List}

#
# eof
#

