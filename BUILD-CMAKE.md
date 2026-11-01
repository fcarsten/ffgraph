# CMake build (Ubuntu 24.04)

The CMake build now selects the modern GNU Bison/Flex GML importer by default.
It generates that parser/scanner in the build tree. The FF and Rome parsers,
and the temporarily retained GT GML fallback, still use their checked-in
legacy bison++/flex++ generated C++ sources. The old Makefiles have **not**
been switched; use CMake for the modern default.

Install prerequisites:

```sh
sudo apt install build-essential cmake ninja-build pkg-config bison flex tcl-dev tk-dev libx11-dev libxpm-dev
```

Build and test:

```sh
cmake --preset debug
cmake --build --preset debug --parallel
ctest --preset debug
```

Release:

```sh
cmake --preset release
cmake --build --preset release --parallel
```

Sanitizers (GCC/Clang):

```sh
cmake --preset asan
cmake --build --preset asan --parallel
ctest --preset asan
```

Install with a custom prefix:

```sh
cmake -S . -B build/install -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build/install --parallel
cmake --install build/install
```

The `ffgraph3D` symlink is installed alongside `ffgraph`. Runtime assets
are installed under the library directory as in the original Makefiles.
Check application-specific runtime asset lookup on your target installation.

## Tcl/Tk window teardown

`FFWindow` owns its Tcl interpreter, except for `FFTclSubWindow`, which
explicitly borrows and preserves its parent's interpreter. Do **not** call
`Tcl_DeleteInterp()` directly from a C++ window destructor: a window can be
removed inside its own Tcl command callback, where Tcl still has active evals.

The application now calls `FFWindow::mainLoop()` instead of `Tk_MainLoop()`.
This runs the usual Tk event loop and calls `FFWindow::collectPendingInterpreters()`
**after** each top-level `Tcl_DoOneEvent()` returns. Embedders using their own
event loop must likewise call the collector at a safe point **outside** Tcl
callbacks and nested Tcl `update` evaluations.

The display-backed `test_ffwindow_tk_teardown` checks interpreter/root
creation and destruction, repeated cycles, already-destroyed roots, two
simultaneous interpreters, and destruction from an active Tcl callback. It is
registered with CTest automatically when `xvfb-run` is installed. Use the
`asan` preset for additional memory/undefined-behaviour checks.

GML production backend selection (CMake):

```sh
# Default modern GML backend (requires Bison >=3.8 and Flex >=2.6)
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DFFGRAPH_GML_BACKEND=modern -DFFGRAPH_ENABLE_MODERN_GML=ON

# Temporary fallback to GTControl; keep new parser built for parity tests
cmake -S . -B build/legacy-gml -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DFFGRAPH_GML_BACKEND=legacy -DFFGRAPH_ENABLE_MODERN_GML=ON

# Fallback without any generator packages (disables modern GML tests)
cmake -S . -B build/legacy-no-generators -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DFFGRAPH_GML_BACKEND=legacy -DFFGRAPH_ENABLE_MODERN_GML=OFF
```

To exercise the actual production load path, run:

```sh
cmake --build build/debug --target test_gml_production_backend_pass11
ctest --test-dir build/debug -R '^test_gml_production_backend_pass11$' --output-on-failure
```

Keep the legacy parser in a tagged Git commit before deleting its checked-in
files. See `docs/FFGRAPH-PASS11-GML-PRODUCTION-SWITCH.md` for migration gates.
