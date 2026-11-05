# PNG assets are the normal path. Legacy graph strings are resolved here;
# they are deliberately not rewritten in the graph model or serialized files.
source [file join [file dirname [info script]] .. icons legacy-icons.tcl]
namespace eval ::ffgraph_icons {
  variable cache
  variable masks
  variable originals
  variable placeholders
  variable warnings
}

proc ::ffgraph_icons::resolve {name} {
  global ICON_LIB_DIR
  variable aliases
  if {[file pathtype $name] eq "absolute"} {
    set directory [file dirname [file normalize $name]]
    set kind [file tail $directory]
    if {$kind in {xpm xbm} && [file dirname $directory] eq [file normalize $ICON_LIB_DIR]} {
      set key [file join $kind [file tail $name]]
      if {[dict exists $aliases $key]} {
        set converted [file join $ICON_LIB_DIR [dict get $aliases $key]]
        if {[file isfile $converted]} {return [file normalize $converted]}
      }
    }
    if {[file isfile $name]} {return [file normalize $name]}
    return ""
  }
  set extension [string tolower [file extension $name]]
  set kind [string trimleft $extension .]
  if {$kind eq ""} {set kind xpm}
  set oldName $name
  if {$extension eq "" && [file tail $name] ne ".xpm"} {append oldName .xpm}
  if {[file dirname $oldName] eq "."} {set oldName [file join $kind $oldName]}
  if {[dict exists $aliases $oldName]} {
    set converted [file join $ICON_LIB_DIR [dict get $aliases $oldName]]
    if {[file isfile $converted]} {return [file normalize $converted]}
  }
  foreach candidate [list [file join $ICON_LIB_DIR png $name] \
                           [file join $ICON_LIB_DIR $name] \
                           [file join $ICON_LIB_DIR $oldName] \
                           [file join $ICON_LIB_DIR $kind $name]] {
    if {[file isfile $candidate]} {return [file normalize $candidate]}
  }
  return ""
}

# External XBM files remain readable without Xpm. Convert their one-bit data to
# an ordinary transparent photo so PNG masks work on these images as well.
proc ::ffgraph_icons::bitmapPhoto {path} {
  set channel [open $path r]
  try {set text [read $channel]} finally {close $channel}
  regsub -all {(?s)/\*.*?\*/} $text {} text
  if {![regexp {#define\s+\S+_width\s+(\d+)} $text -> width] ||
      ![regexp {#define\s+\S+_height\s+(\d+)} $text -> height] ||
      ![regexp {static\s+(?:unsigned\s+)?(char|short)\s+[^\{]+\{([^\}]*)\}} $text -> unit data]} {
    error "Invalid XBM image: $path"
  }
  set bits [expr {$unit eq "short" ? 16 : 8}]
  set stride [expr {($width + $bits - 1) / $bits}]
  set values [regexp -all -inline {0[xX][0-9a-fA-F]+|[0-9]+} $data]
  if {$width <= 0 || $height <= 0 || [llength $values] != $stride * $height} {
    error "Invalid XBM dimensions: $path"
  }
  set result [image create photo -width $width -height $height]
  try {
    for {set y 0} {$y < $height} {incr y} {
      for {set x 0} {$x < $width} {incr x} {
        set value [lindex $values [expr {$y * $stride + $x / $bits}]]
        if {$value & (1 << ($x % $bits))} {$result put black -to $x $y}
      }
    }
  } on error {message options} {
    image delete $result
    return -options $options $message
  }
  return $result
}

proc ::ffgraph_icons::load {path} {
  variable cache
  global iconFileCache
  if {[info exists cache($path)]} {return $cache($path)}
  set type [string trimleft [string tolower [file extension $path]] .]
  switch -- $type {
    xpm {
      if {[lsearch -exact [image types] pixmap] < 0} {
        error "External Xpm images require a Linux build with FFGRAPH_XPM=ON: $path"
      }
      set picture [image create pixmap -file $path]
    }
    xbm {set picture [bitmapPhoto $path]}
    default {set picture [image create photo -file $path]}
  }
  set cache($path) $picture
  set iconFileCache($picture,$type) $path
  return $picture
}

proc ::ffgraph_icons::pair {normal selected} {
  set path [resolve $normal]
  set selectedPath ""
  if {$path eq ""} {
    # Historical extensionless names and N/S icon pairs remain supported.
    set extension [file extension $normal]
    if {$extension eq ""} {set extension .xpm}
    set root [file rootname $normal]
    set path [resolve ${root}N${extension}]
    if {$selected eq "__NoIcon__"} {set selectedPath [resolve ${root}S${extension}]}
  }
  if {$path eq ""} {return ""}
  if {$selected ne "__NoIcon__"} {set selectedPath [resolve $selected]}
  if {$selectedPath eq ""} {set selectedPath $path}
  return [list [load $path] [load $selectedPath] \
    [string trimleft [file extension $path] .] [string trimleft [file extension $selectedPath] .]]
}

# Only graph rendering opts into recovery; the chooser and direct Tcl callers
# retain strict errors. Keep the original references in the C++ graph model.
proc ::ffgraph_icons::nodePair {normal selected allowFallback} {
  variable warnings
  if {![catch {
    set pair [pair $normal $selected]
    if {$pair eq ""} {error "Cannot find icon: $normal"}
  } message options]} {return [list $pair 0]}
  if {!$allowFallback} {return -options $options $message}
  set fallback [pair xbm/default.png xbm/default.png]
  if {$fallback eq ""} {error "$message; cannot find default icon, please check installation"}
  set key [list $normal $selected]
  if {![info exists warnings($key)]} {
    set warnings($key) 1
    puts stderr "Warning: $message; using default icon without the original mask ($normal, $selected)"
  }
  return [list $fallback 1]
}

proc ::ffgraph_icons::remember {canvas item pair {placeholder 0}} {
  global unselectImage selectImage iconType iconSelType
  variable originals
  variable placeholders
  lassign $pair unselectImage($canvas,$item) selectImage($canvas,$item) \
                iconType($canvas,$item) iconSelType($canvas,$item)
  set originals($canvas,$item) $pair
  set placeholders($canvas,$item) $placeholder
}

proc ::ffgraph_icons::maskImage {base mask} {
  variable masks
  global iconFileCache
  if {$mask eq "__NoMask__"} {return $base}
  # Historical masks always came from the XBM directory, even when FF_I
  # stored an extensionless name or an Xpm-style name. Preserve that choice
  # rather than accidentally using the opaque, similarly named color icon.
  set path ""
  if {[file extension $mask] in {"" ".xpm"}} {
    set path [resolve [file rootname $mask].xbm]
  }
  if {$path eq ""} {set path [resolve $mask]}
  if {$path eq ""} {error "Cannot find icon mask: $mask"}
  set key [list $base $path]
  if {[info exists masks($key)]} {return $masks($key)}
  set maskPhoto [load $path]
  if {[image type $maskPhoto] ne "photo"} {error "Masks must be PNG or XBM images"}
  set width [image width $base]
  set height [image height $base]
  if {$width != [image width $maskPhoto] || $height != [image height $maskPhoto]} {
    error "Icon and mask have different dimensions"
  }
  if {[image type $base] eq "photo"} {
    set result [image create photo]
    $result copy $base
    for {set y 0} {$y < $height} {incr y} {
      for {set x 0} {$x < $width} {incr x} {
        if {[$maskPhoto transparency get $x $y]} {$result transparency set $x $y 1}
      }
    }
  } else {
    # The optional legacy Linux pixmap handler accepts XBM mask data. Derive
    # that data from the PNG mask; no legacy mask assets need to be shipped.
    set bytes {}
    for {set y 0} {$y < $height} {incr y} {
      for {set x 0} {$x < $width} {incr x 8} {
        set byte 0
        for {set b 0} {$b < 8 && $x + $b < $width} {incr b} {
          if {![$maskPhoto transparency get [expr {$x+$b}] $y]} {set byte [expr {$byte | (1 << $b)}]}
        }
        lappend bytes [format 0x%02x $byte]
      }
    }
    set data "#define mask_width $width\n#define mask_height $height\nstatic unsigned char mask_bits\[\] = \{\n[join $bytes ,]\n\};"
    set result [image create pixmap -file $iconFileCache($base,xpm) -maskdata $data]
  }
  set masks($key) $result
  return $result
}

proc ::ffgraph_icons::applyMask {canvas item normal selected} {
  global unselectImage selectImage
  variable originals
  variable placeholders
  if {![info exists originals($canvas,$item)]} {return}
  # A mask belongs to the absent artwork, not to the default image. Its
  # dimensions and silhouette need not match the placeholder at all.
  if {$placeholders($canvas,$item)} {return}
  lassign $originals($canvas,$item) base selectedBase
  set wasSelected [expr {$unselectImage($canvas,$item) ne $selectImage($canvas,$item) && [$canvas itemcget $item -image] eq $selectImage($canvas,$item)}]
  if {$selected eq "__NoMask__"} {set selected $normal}
  # Resolve both before publishing a change, including on size/load errors.
  set picture [maskImage $base $normal]
  set selectedPicture [maskImage $selectedBase $selected]
  set unselectImage($canvas,$item) $picture
  set selectImage($canvas,$item) $selectedPicture
  $canvas itemconfigure $item -image [expr {$wasSelected ? $selectedPicture : $picture}]
}
