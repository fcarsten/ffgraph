proc imageCheck {condition message} {
  if {![uplevel 1 [list expr $condition]]} {error $message}
}
# Older embedders supplied the icon directory with a trailing separator.
append ICON_LIB_DIR /
set channel [open [file join $ffgraph_test_source tests image_assets_manifest.tsv] r]
set manifest [read $channel]
close $channel
set count 0
foreach line [split $manifest \n] {
  if {$line eq "" || [string index $line 0] eq "#"} {continue}
  lassign [split $line \t] legacy png width height checksum sourceHash
  set path [file join $ICON_LIB_DIR $png]
  set picture [::ffgraph_icons::load $path]
  imageCheck {[image type $picture] eq "photo"} "PNG did not load as a Tk photo: $png"
  imageCheck {[image width $picture] == $width && [image height $picture] == $height} "Changed dimensions: $png"
  set pixels {}
  for {set y 0} {$y < $height} {incr y} {
    for {set x 0} {$x < $width} {incr x} {
      lappend pixels {*}[$picture get $x $y] [expr {[$picture transparency get $x $y] ? 0 : 255}]
    }
  }
  imageCheck {[zlib crc32 [binary format c* $pixels]] == $checksum} "Changed source pixels: $png"
  imageCheck {[::ffgraph_icons::resolve $legacy] eq [file normalize $path]} "Broken graph alias: $legacy"
  imageCheck {[::ffgraph_icons::resolve [file normalize [file join $ICON_LIB_DIR $legacy]]] eq [file normalize $path]} "Broken absolute bundled alias: $legacy"
  incr count
}
imageCheck {$count == 85} "Incomplete asset conversion"
imageCheck {[llength [glob -nocomplain $ICON_LIB_DIR/xpm/*.xpm]] == 0} "Bundled Xpm assets remain"
imageCheck {[llength [glob -nocomplain $ICON_LIB_DIR/xbm/*.xbm]] == 0} "Bundled XBM assets remain"

# Historical implicit normal/selected pairs, as well as the new PNG names.
lassign [createIcon procedure] normal selected
imageCheck {$normal ne $selected} "Historical N/S selection pair was lost"
imageCheck {$normal eq [lindex [createIcon procedureN.png] 0]} "Legacy normal alias differs from PNG"
imageCheck {$selected eq [lindex [createIcon procedureS.png] 0]} "Legacy selected alias differs from PNG"
set canvas .frame0.canvas2
set item [addNode $canvas 70 80 10 black redball.png blueball.png]
imageCheck {[$canvas type $item] eq "image"} "Explicit PNG icon rendered as a figure"
set base $unselectImage($canvas,$item)
set selectedBase $selectImage($canvas,$item)
set legacyMask [::ffgraph_icons::maskImage $base ballmask.xbm]
imageCheck {[::ffgraph_icons::maskImage $base ballmask] eq $legacyMask} "Extensionless legacy mask chose the color icon"
imageCheck {[::ffgraph_icons::maskImage $base ballmask.xpm] eq $legacyMask} "Legacy mask extension changed its one-bit meaning"
changeMask $item ballmask.xbm diamondmask.xbm $canvas
set masked $unselectImage($canvas,$item)
imageCheck {[$masked transparency get 0 0]} "Converted mask did not clip corner"
imageCheck {![$masked transparency get 7 6]} "Converted mask clipped center"
imageCheck {![$base transparency get 0 0]} "Mask mutated a cached original"
setSelected $item $canvas
imageCheck {[$canvas itemcget $item -image] eq $selectImage($canvas,$item)} "Selected mask was not displayed"
set imageCount [llength [image names]]
for {set repeat 0} {$repeat < 10} {incr repeat} {
  changeMask $item ballmask.xbm diamondmask.xbm $canvas
}
imageCheck {[llength [image names]] == $imageCount} "Repeated masks allocate duplicate images"
set before [$canvas itemcget $item -image]
imageCheck {[catch {changeMask $item default.xbm __NoMask__ $canvas}]} "Mismatched mask sizes accepted"
imageCheck {[$canvas itemcget $item -image] eq $before} "Failed mask changed the displayed image"
changeMask $item __NoMask__ __NoMask__ $canvas
imageCheck {$unselectImage($canvas,$item) eq $base && $selectImage($canvas,$item) eq $selectedBase} "Removing masks did not restore originals"
imageCheck {[catch {changeToIcon $item does-not-exist.png __NoIcon__ 0 0 $canvas}]} "Missing icon accepted"
imageCheck {[$canvas type $item] eq "image"} "Failed icon change destroyed the item"
removeItem $canvas $item

# External byte/X10-short XBM files still work without the Xpm extension.
foreach unit {char short} values {{0x01,0x01,0x02,0x00} {0x0101,0x0002}} {
  set channel [file tempfile bitmap [file join [pwd] external-XXXXXX.xbm]]
  puts $channel "#define test_width 9\n#define test_height 2\nstatic $unit test_bits\[\] = \{ $values \};"
  close $channel
  # Windows Tcl's tempfile uses .TMP regardless of the requested suffix.
  file rename $bitmap ${bitmap}.xbm
  append bitmap .xbm
  try {
    set photo [::ffgraph_icons::load $bitmap]
    imageCheck {[image type $photo] eq "photo" && [image width $photo] == 9} "External XBM failed"
    imageCheck {![$photo transparency get 0 0] && ![$photo transparency get 8 0]} "XBM row padding changed"
    imageCheck {[$photo transparency get 0 1] && ![$photo transparency get 1 1]} "XBM bit order changed"
  } finally {file delete $bitmap}
}

# The optional Linux-only reader is exercised with a non-bundled Xpm file.
set channel [file tempfile external [file join [pwd] {external icon with spaces-XXXXXX.xpm}]]
puts $channel {/* XPM */
static char *external[] = {
"2 2 3 1", "a c #ff0000", "b c #0000ff", ". c None", "a.", "ba"};}
close $channel
file rename $external ${external}.xpm
append external .xpm
try {
  if {$ffgraph_test_xpm} {
    set legacy [::ffgraph_icons::load $external]
    imageCheck {[image type $legacy] eq "pixmap" && [image width $legacy] == 2} "External Xpm compatibility failed"
    set previousRoot $ICON_LIB_DIR
    set customRoot ${external}.icons
    file mkdir $customRoot/xpm
    file copy $external $customRoot/xpm/custom.xpm
    try {
      set ICON_LIB_DIR $customRoot
      set implicit [lindex [createIcon custom] 0]
      imageCheck {[image type $implicit] eq "pixmap"} "Extensionless external Xpm reference failed"
    } finally {set ICON_LIB_DIR $previousRoot; file delete -force $customRoot}
    set mask [image create photo -width 2 -height 2]
    $mask put black -to 0 0
    set maskFile ${external}.png
    $mask write $maskFile -format png
    try {
      set clipped [::ffgraph_icons::maskImage $legacy $maskFile]
      imageCheck {[image type $clipped] eq "pixmap"} "PNG mask failed on legacy Xpm"
    } finally {file delete $maskFile; image delete $mask}
  } else {
    imageCheck {[lsearch -exact [image types] pixmap] < 0} "Unexpected Xpm extension in default build"
    imageCheck {[catch {::ffgraph_icons::load $external} message]} "External Xpm unexpectedly supported"
    imageCheck {[string match {*FFGRAPH_XPM=ON*} $message]} "Missing Xpm compatibility diagnostic"
  }
} finally {file delete $external}

showIconBox
imageCheck {[winfo exists .topIcon]} "PNG icon chooser failed"
set chooser .topIcon.frameLeft.frame0.canvas
# Canvas image items have dimension tags rather than an 'image' tag.
set images 0
foreach id [$chooser find all] {
  if {[$chooser type $id] eq "image"} {incr images}
}
imageCheck {$images == 85} "Chooser did not display every converted asset"
imageCheck {[lsearch -exact [array get iconBox] xbm/default.png] >= 0} "Chooser lost monochrome subdirectory names"
hideIconBox
puts "All 85 PNG rasters match source CRCs; aliases, selection, masks and chooser passed"
