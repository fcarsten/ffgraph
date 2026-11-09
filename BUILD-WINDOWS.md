# Native Windows build — 1.0.0-rc1

The initial Windows target is 64-bit MinGW-w64 with native Tcl/Tk 8.6.
MSYS2 supplies development tools; the installed application uses native Windows
DLLs and does not require MSYS2, WSL, an X server, X11, or Xpm at runtime.
MSVC is not yet supported. Windows 11 desktop validation is still required;
see [the verification report](docs/FFGRAPH-WINDOWS-NATIVE-PORT.md).

The controls use ttk's native Windows theme (`vista`, with available native
fallbacks). ttk is included with Tk 8.6; no separate theme package is needed.
See [the widget port report](docs/FFGRAPH-TTK-WIDGET-PORT.md).

Install [MSYS2](https://www.msys2.org/) and open its **UCRT64** terminal. Update
it following the MSYS2 installation instructions, then install:

```sh
pacman -S --needed bison flex git \
  mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-pkgconf \
  mingw-w64-ucrt-x86_64-python \
  mingw-w64-ucrt-x86_64-tcl \
  mingw-w64-ucrt-x86_64-tk
```

Git must be available on PATH to Python subprocesses for the test suite and
release packaging, even if CMake can locate a Git for Windows installation
through the registry. The CI workflow preserves the runner PATH inside MSYS2
and verifies this before configuration. Git is a build/test dependency; it is
not needed to run the installed application.

From the checkout in that terminal:

```sh
cmake -S . -B build/windows -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build/windows --parallel 2
ctest --test-dir build/windows --output-on-failure --parallel 1
./build/windows/ffgraph.exe
```

Windows defaults to `FFGRAPH_INITIAL_TESTS_ONLY=ON`. This registers the portable
suite, including containers, graph invariants and geometry, layouts and
allocation recovery, FF/GML loading and serialization, GML/Rome parsers,
resource lookup, native 2D/3D windows, cross-window graph copy/paste, inspector
controls, and staged application startup. GUI tests use native Tk directly.
The complete Linux suite is retained on Linux; it has not all been ported.

Install into a dedicated directory to collect the application, required native
DLLs, and Tcl/Tk scripts:

```sh
cmake --install build/windows --prefix "$PWD/build/windows-stage"
```

Copy the whole `windows-stage` directory to the Windows machine, then start
`bin/ffgraph.exe` or `bin/ffgraph3D.exe`. Keep `bin` and `lib` together when
moving it. Neither program requires administrator rights or symlink privileges.
The install test checks startup outside the checkout, automatic resource lookup
and explicit `FF_LIBDIR`/`FF_ICONDIR` overrides, with MSYS2 removed from the
child application's `PATH`. It also inspects bundled PE imports for X11/Xpm,
MSYS2, and Cygwin dependencies.

Bundled icons and masks use PNG on Windows and Linux. Graphs referencing
historical bundled Xpm/XBM filenames resolve to the converted PNG assets;
normal/selected images and masks work without an Xpm library. External Xpm
files remain an optional Linux-only compatibility feature (`FFGRAPH_XPM=ON`).
Windows uses the system clipboard for graph copy/paste. See
[the PNG migration report](docs/FFGRAPH-PNG-ASSET-MIGRATION.md).

The [Windows workflow](.github/workflows/windows.yml) builds Debug and Release,
runs the initial suite and uploads the staged application. Release builds also
produce source/binary packages, manifests and checksums. Check CI results for
the exact commit being released.

Package a verified native Release build with Python 3.11+ and Git:

```sh
python scripts/package-release.py --build build/windows --output build/release-artifacts
```

Use `--preview` for uncommitted work. See [RELEASING.md](RELEASING.md) for
artifact names, notices and the final Windows 11 desktop check. Keep `share`
with `bin` and `lib`; it contains documentation and license notices.

The installed library provides `find_package(ffgraph 1.0 CONFIG REQUIRED)` and
the `ffgraph::ffgraph` target. Configure consumers with the same native MinGW
toolchain and `-DCMAKE_PREFIX_PATH=/path/to/staged-prefix`; Tcl/Tk development
packages are required to build clients. `ffgraph --version` reports `1.0.0-rc1`.

For file-selector diagnostics, launch from PowerShell in the extracted package:

```powershell
$env:FFGRAPH_FILE_SELECTOR_LOG = "$env:TEMP\ffgraph-selector.log"
& ".\bin\ffgraph.exe"
```

The UTF-8 log appends window visibility/stacking, focus, directory-scan and Tcl
error records, including directory/resource paths. It is readable while the GUI
is waiting. Use `ffgraph3D.exe` for the 3D application. Disable logging for the
next launch with `Remove-Item Env:FFGRAPH_FILE_SELECTOR_LOG`.
See [the diagnostics report](docs/FFGRAPH-FILE-SELECTOR-DIAGNOSTICS.md).
