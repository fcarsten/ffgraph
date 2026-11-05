# Native Windows build (initial port)

The initial Windows target is 64-bit MinGW-w64 with native Tcl/Tk 8.6.
MSYS2 supplies development tools; the installed application uses native Windows
DLLs and does not require MSYS2, WSL, an X server, X11, or Xpm at runtime.
MSVC is not yet supported. Windows 11 desktop validation is still required;
see [the verification report](docs/FFGRAPH-WINDOWS-NATIVE-PORT.md).

Install [MSYS2](https://www.msys2.org/) and open its **UCRT64** terminal. Update
it following the MSYS2 installation instructions, then install:

```sh
pacman -S --needed bison flex \
  mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-pkgconf \
  mingw-w64-ucrt-x86_64-python \
  mingw-w64-ucrt-x86_64-tcl \
  mingw-w64-ucrt-x86_64-tk
```

From the checkout in that terminal:

```sh
cmake -S . -B build/windows -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build/windows --parallel 2
ctest --test-dir build/windows --output-on-failure --parallel 1
./build/windows/ffgraph.exe
```

Windows defaults to `FFGRAPH_INITIAL_TESTS_ONLY=ON`. This registers the initial
23 tests, including containers, graph invariants and geometry, layouts and
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

Xpm icons and the old pixmap mask extension are omitted. Loading an Xpm icon
returns an unsupported-image diagnostic; the icon chooser skips those files.
Existing GIF/PPM/XBM handling remains. Windows uses the system clipboard for
graph copy/paste. Linux keeps its existing selection behavior and defaults to
building its Xpm extension.

The new [Windows workflow](.github/workflows/windows.yml) builds Debug and
Release, runs the initial suite, and uploads the staged application. Workflow
execution requires the changes to be present on GitHub; adding the workflow
does not establish a successful CI run.
