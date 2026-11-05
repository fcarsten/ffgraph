# Drive the real menu and chooser bindings, then the canvas creation callback.
# Item tags substitute for a physical pointer so scrolling/window placement
# cannot affect which gallery icon the test selects.
proc chooseTestIcon {gallery name} {
  global iconBox
  foreach item [$gallery find all] {
    if {[$gallery type $item] ne "image"} {continue}
    set image [$gallery itemcget $item -image]
    if {$iconBox($image) ne $name} {continue}
    $gallery dtag all current
    $gallery addtag current withtag $item
    set script [$gallery bind $item <Button-1>]
    uplevel #0 [string map [list %% % %W $gallery] $script]
    $gallery dtag $item current
    return
  }
  error "Icon missing from chooser: $name"
}

.frame1.menubutton3.m.shape invoke Icon
if {$DefShape ne "Icon"} {error "Icon menu did not select Icon shape"}
recall setNoIcon
chooseTestIcon .topIcon.frameLeft.frame0.canvas redball.png
chooseTestIcon .topIcon.frameRight.frame1.canvas blueball.png
recall setCurrentMask ballmask.xbm
recall setCurrentSelMask diamondmask.xbm
controlButton1 recall .frame0.canvas2 250 180
controlButton1 recall .frame0.canvas2 400 280
if {$DefShape ne "Icon"} {error "Creating an icon changed the selected shape"}
# A menu switch must still allow ordinary figure creation afterwards.
.frame1.menubutton3.m.shape invoke Square
controlButton1 recall .frame0.canvas2 500 380
hideIconBox
