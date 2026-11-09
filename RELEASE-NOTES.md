# ffgraph 1.0.0-rc1

A preservation release of the historical ffgraph algorithms and application,
updated for modern Linux and Windows, with documented correctness fixes.
This prerelease is available for public compatibility testing before 1.0.0.
Feedback on real Linux and Windows desktops is especially welcome.

## Downloads and getting started

Choose the appropriate file from this release's **Assets**:

| Platform | Archive |
| --- | --- |
| Windows x86-64 | `ffgraph-1.0.0-rc1-windows-x86_64.zip` |
| Ubuntu 24.04 x86-64 | `ffgraph-1.0.0-rc1-linux-x86_64.tar.gz` |
| Source code | `ffgraph-1.0.0-rc1-source.tar.gz` |

**Windows:** extract the complete ZIP into a fresh directory and open the
extracted `ffgraph-1.0.0-rc1` folder. Launch
`bin\ffgraph.exe` for 2D or `bin\ffgraph3D.exe` for 3D. Keep the `bin`, `lib`,
`include` and `share` directories together. Tcl/Tk and the required runtime DLLs
are bundled; no MSYS2, WSL, X server or separate Tcl/Tk installation is needed.
The Windows executables are not code-signed.

**Ubuntu:** extract the complete archive and run `bin/ffgraph` or
`bin/ffgraph3D`. This binary package targets Ubuntu 24.04 and uses system
Tcl/Tk 8.6 and X11 libraries. For building from source, follow the
[Linux build instructions](https://github.com/fcarsten/ffgraph/blob/v1.0.0-rc1/BUILD-CMAKE.md)
or [Windows build instructions](https://github.com/fcarsten/ffgraph/blob/v1.0.0-rc1/BUILD-WINDOWS.md).

Platform-specific `SHA256SUMS` and manifest files accompany the archives.
Verify the download against its checksum; manifests identify the source commit,
build configuration and packaged file hashes. The source archive is also
available for rebuilding the candidate.

## Supported configurations

- Ubuntu 24.04, GCC, C++17, CMake 3.24+, Tcl/Tk 8.6 and X11.
- Windows x86-64, native MinGW-w64/MSYS2 UCRT64 and Tcl/Tk 8.6. The installed
  application needs no MSYS2, WSL, X server or Xpm. Windows runs an initial
  portable test suite; Linux retains the full suite.
- GNU Bison 3.8+, Flex 2.6+ and Python 3 are build prerequisites; Git is
  required for the test suite and release packaging.

Original 0.91 release notes and 1990s installation instructions are retained under
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
- Fixes for icon creation/missing artwork, colour selection and node fills,
  layout-inspector selection on Windows, and zooming empty graph regions.
- File-selector startup, focus, navigation and cancellation fixes, including
  Windows GUI builds without a console. Closing a graph with an active Load
  or Save As dialog cancels the file operation before closing its owner.
- New Graph windows remain open after creation. The title-bar Close button
  closes the selected graph and its helper windows; closing the last graph
  exits the application normally.
- Windows node movement, edge creation and Alt shortcuts work with Num Lock
  enabled; lock keys no longer trigger the node-resize bindings.
- Shared formatted Info dialog for 2D/3D, with contributor acknowledgements,
  selectable text and clickable license/project links. Under WSL, links use
  Windows browser interoperability when available.
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
- Info links on native Linux need `xdg-open` or `gio` with a configured HTTP(S)
  browser handler. WSL uses `wslview` or Windows' `rundll32.exe` when available
  on its PATH. External browser setup is outside the application.
- Spring retains its historical distinct weighted 2D/3D formulas and pinned
  selection convention; quality zero runs until stopped.
- Missing historical artwork is represented by a placeholder; graph references
  are preserved when saved. External images are not embedded into graph files.
- GUI sanitizer runtime tests and toolkit shutdown-leak diagnostics are separate
  partitions. Passing runtime checks does not establish toolkit leak freedom.
- Windows 11 desktop validation, especially non-ASCII paths and different
  display-scaling settings, remains a focus of this release candidate.
  Automated Windows CI and Wine tests do not cover every desktop configuration.

## What to test and how to report problems

Please try both 2D and 3D, including:

- Loading, saving and reopening FF/sgraph, GML and Rome graphs.
- Creating/moving nodes and edges, with Num Lock on and off on Windows.
- Running layouts and opening, dismissing and reopening their inspectors.
- Icon nodes, colours, labels, zoom and copy/paste between graph windows.
- File-selector navigation and closing a graph while its file dialog is open.
- Paths containing spaces or non-ASCII characters, display scaling, Info links
  and closing the last graph window.

Report reproducible problems through
[GitHub Issues](https://github.com/fcarsten/ffgraph/issues). Include:

- Release version (`1.0.0-rc1`), operating-system version, and whether you use
  native Windows, Linux or WSL.
- Display scaling and relevant keyboard settings such as Num Lock.
- Steps to reproduce, expected/actual results and the complete error message.
- A minimal example graph and screenshots or logs where useful.

If a file selector fails to appear or respond, set `FFGRAPH_FILE_SELECTOR_LOG`
to a writable filename before launch and include the resulting diagnostic log.
The build instructions explain how to enable it.

Maintainer packaging and verification instructions are in
[RELEASING.md](https://github.com/fcarsten/ffgraph/blob/v1.0.0-rc1/RELEASING.md).
Packages labelled `preview` are working-tree snapshots; public candidate assets
are generated from the clean, tagged release commit.
