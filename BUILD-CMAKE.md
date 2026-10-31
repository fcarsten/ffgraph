# CMake build (Ubuntu 24.04)

The legacy Makefiles/configure scripts are intentionally retained during the transition.
The checked-in bison++/flex++ generated C++ sources are compiled directly; CMake
does not regenerate them. Grammar modernisation is a separate task.

Install prerequisites:

```sh
sudo apt install build-essential cmake ninja-build pkg-config tcl-dev tk-dev libx11-dev libxpm-dev
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
