# ffgraph 1.0.0-rc1

A preservation release of the historical ffgraph graph library and editor,
updated for modern Linux and Windows, with documented correctness fixes.
Originally developed in the early 1990s, ffgraph provides C++ graph structures,
FF/sgraph, GML and Rome loading, 2D/3D visualization, and the original layout
algorithms, including Spring, Sugiyama and orthogonal layouts.

This is a release candidate for compatibility testing. See RELEASE-NOTES.md
for changes, preserved conventions and known limitations.

- Ubuntu 24.04 build/test/install: BUILD-CMAKE.md
- Native Windows x86-64 build and portable installation: BUILD-WINDOWS.md
- Release packaging and final verification: RELEASING.md
- License and bundled component notices: LICENSE and THIRD-PARTY-NOTICES.md
- Historical manuals: doc/ (some describe older behavior and toolchains)

The supported build uses C++17, CMake 3.24+, Tcl/Tk 8.6, GNU Bison 3.8+,
Flex 2.6+ and Python 3. Linux requires X11. Windows uses native Tcl/Tk and
requires no MSYS2, WSL or X server at runtime.

Bundled images use PNG and preserve historical graph references. Optional
Linux loading of external Xpm images can be enabled with -DFFGRAPH_XPM=ON.

The installed C++ library includes a CMake package: find_package(ffgraph 1.0
CONFIG REQUIRED), then link ffgraph::ffgraph. Rebuild clients with a compatible
compiler/runtime. The old String.h compatibility header is now FFString.h.

# Authors and contributors to the historic code:

Main authors: Carsten Friedrich, Falk Schreiber

Contributors: Dirk Koschützki, Andreas Holzmann, Achim Dannecker, Andreas Claßen, Volker Braun, Barbara Zündorf, others at the Lehrstuhl für Programmiersysteme University of Passau 

Project lead: Bernhard Steffen, Head of Lehrstuhl für Programmiersysteme at the University of Passau 

# License

GPL 3

Github: https://github.com/fcarsten/ffgraph
