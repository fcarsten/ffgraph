# ffgraph 1.0.0-rc1

A preservation release of the historical ffgraph algorithms and application,
updated for modern Linux and Windows, with documented correctness fixes.
This release candidate is intended for final compatibility testing.

## Supported configurations

- Ubuntu 24.04, GCC, C++17, CMake 3.24+, Tcl/Tk 8.6 and X11.
- Windows x86-64, native MinGW-w64/MSYS2 UCRT64 and Tcl/Tk 8.6. The installed
  application needs no MSYS2, WSL, X server or Xpm. Windows runs an initial
  portable test suite; Linux retains the full suite.
- GNU Bison 3.8+, Flex 2.6+ and Python 3 are build prerequisites; Git is
  required for the test suite and release packaging.

See `BUILD-CMAKE.md` and `BUILD-WINDOWS.md` for current instructions. Original
0.91 release notes and 1990s installation instructions are retained under
`docs/historical/`. Earlier per-patch reports under `docs/` are historical
verification records and may contain superseded counts or open issues.

## Changes since the historical distribution

- Modern C++17/CMake builds, STL compatibility containers and generated GNU
  Bison/Flex FF/sgraph, GML and Rome importers, with graph/format regressions.
- Native Windows builds, portable resource discovery and relocatable runtime
  bundles. Installed libraries include their required headers and a CMake
  `ffgraph::ffgraph` target for external consumers.
- ttk controls and inspectors, native Windows themes and the Linux `clam`
  theme. Graph canvases and widgets without ttk equivalents retain Tk.
- All 85 bundled Xpm/XBM images converted to PNG, preserving pixels and legacy
  graph references. Linux external Xpm support is opt-in (`FFGRAPH_XPM=ON`).
- Fixes for icon creation/missing artwork, file-selector mapping/navigation,
  inspector selection, colour selection and empty-region zoom.
- Graph/container lifetime and GUI callback/interpreter cleanup corrections.
- Spring stability, equilibrium, weight-toggle, pinning and iteration fixes.
- Sugiyama crossing/barycenter, termination, controls, offsets, large-input,
  numeric-bound and allocation/publication-recovery corrections.
- Layout performance improvements, optional profiling tools, expanded graph
  oracles, frozen-layout comparisons, failure injection and sanitizer coverage.

Correctness fixes can change drawings affected by historical bugs. The original
layout algorithms and their documented legacy conventions are preserved; exact
coordinates across every platform, allocator and graph are not guaranteed.

## Compatibility and limitations

- The former compatibility header `String.h` is now `FFString.h`; rebuild
  library clients with the same compiler/runtime family as the installed archive.
- CMake is the supported build system; original Makefiles remain archival.
- Windows external Xpm loading, MSVC, ARM64 and Tcl/Tk 9 are not supported.
- Linux still requires X11 (including XWayland on Wayland desktops).
- Spring retains its historical distinct weighted 2D/3D formulas and pinned
  selection convention; quality zero runs until stopped.
- Missing historical artwork is represented by a placeholder; graph references
  are preserved when saved. External images are not embedded into graph files.
- GUI sanitizer runtime tests and toolkit shutdown-leak diagnostics are separate
  partitions. Passing runtime checks does not establish toolkit leak freedom.
- Final Windows 11 Release-ZIP testing, including non-ASCII paths and display
  scaling, must be recorded before promotion to a stable release. Automated
  Windows CI and Wine tests do not replace that desktop check.

## Release artifacts

Publish the matching source archive, Windows x86-64 Release ZIP, release
manifest and `SHA256SUMS`. Preview packages made from uncommitted changes are
explicitly named `preview` and must be regenerated from the final clean commit.
See `RELEASING.md` for the repeatable packaging and verification procedure.
