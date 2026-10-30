#!/public/bin/pixmap-wish -f
# Program: povexp
#

proc Povexp_MakeMyEntry {name text recall_text} {
  frame $name
  label $name.l -text $text -anchor w
  entry $name.e1 -width 8 -relief sunken -bg white
  entry $name.e2 -width 8 -relief sunken -bg white
  $name.e1 insert 1 ""
  $name.e2 insert 1 ""

  bind $name.e1 <KeyPress> {
    if {("[expr %N -48]" != "%A") && ("%A" != "\{\}") && \
          ("%K" != "BackSpace") && ("%K" != "Delete")} {
      break
    }
  }
  bind $name.e2 <KeyPress> {
    if {("[expr %N -48]" != "%A") && ("%A" != "\{\}") && \
          ("%K" != "BackSpace") && ("%K" != "Delete")} {
      break
    }
  }


  bind $name.e1 <KeyRelease> "global pov; set pov($name,pre) \[%W get\];\
                                recall pre.$recall_text \[%W get\]"
  bind $name.e2 <KeyRelease> "global pov; set pov($name,post) \[%W get\];\
                                recall post.$recall_text \[%W get\]"

  # bind $name.e2 <KeyRelease> "recall post.$recall_text \[%W get\]"

  pack append $name \
    $name.e1 {left frame w}\
    $name.l {left frame w expand fillx} \
    $name.e2 {right frame w}
}

proc Povexp_MakeParams {zeug} {

  frame .pov.$zeug -bd 3 -relief ridge
  label .pov.$zeug.l -text "$zeug:" -anchor nw 
  frame .pov.$zeug.coords -relief ridge
  
  Povexp_MakeMyEntry .pov.$zeug.coords.x x zeug_x
  Povexp_MakeMyEntry .pov.$zeug.coords.y y zeug_y
  Povexp_MakeMyEntry .pov.$zeug.coords.z z zeug_z

  pack append .pov.$zeug.coords \
    .pov.$zeug.coords.x {top expand fillx} \
    .pov.$zeug.coords.y {top expand fillx} \
    .pov.$zeug.coords.z {top expand fillx} 

  pack append .pov.$zeug \
    .pov.$zeug.l {top frame nw expand fillx} \
    .pov.$zeug.coords {top frame nw expand fillx} 
}
 


# procedure to show window .
proc ShowPovWindow {} {# xf ignore me 7
  toplevel .pov
  # Window manager configurations
  global tk_version
  wm positionfrom .pov program
  wm sizefrom .pov program
  #wm maxsize .pov 130 500
  
  wm title .pov {POVRay Export}
  
  Povexp_MakeParams camera
  Povexp_MakeParams objects

  pack append .pov \
    .pov.camera {top frame nw expand fillx padx 4 pady 4} \
    .pov.objects {top frame nw expand fillx padx 4 pady 4} 
}

# display/remove toplevel windows.
option add *HighlightThickness 0
option add *BorderWidth 2

