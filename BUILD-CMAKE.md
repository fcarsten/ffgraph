# CMake build (Ubuntu 24.04)

The CMake build now selects the modern GNU Bison/Flex GML importer by default.
It generates that parser/scanner in the build tree. FF/sgraph has an explicit
modern backend option, with legacy FF still the default. Rome uses its
checked-in legacy parser. The old Makefiles have **not** been switched;
use CMake for the modern backends.

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

## FF/sgraph backend (Pass 12 integration)

The CMake cache selector `FFGRAPH_FF_BACKEND=legacy|modern` controls the real
application loader for `.g` files and FF string/clipboard imports. Its default
is `legacy`. It does not affect GML or Rome. This is a build-time choice;
there is no runtime environment-variable selector or automatic fallback.

To build and run the application with modern FF, from the repository root:

```sh
cmake -S . -B build/ff-modern -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON -DFFGRAPH_FF_BACKEND=modern
cmake --build build/ff-modern --parallel 2
ctest --test-dir build/ff-modern --output-on-failure

FF_LIBDIR="$PWD/lib/ffgraph" FF_ICONDIR="$PWD/lib/ffgraph/icons" \
  ./build/ff-modern/ffgraph contrib/out.g
```

Modern selection generates/links the parser even with `BUILD_TESTING=OFF`.
Generated sources stay in the build tree. When installing a modern FF build,
the generated parser archive is installed with `FFGraph`.

To return an existing directory to the legacy FF backend, reconfigure with
`-DFFGRAPH_FF_BACKEND=legacy` and rebuild. Unspecified options retain cached
values. `FFGRAPH_ENABLE_MODERN_FF_PILOT=ON` still enables isolated compatibility
tests in a legacy build; enabling the pilot alone does not switch the app.
Modern builds include these tests automatically when testing is enabled.

Native FF GUI regressions run with Xvfb when available, including actual
File → Load/Merge/Close/reopen and 2D/3D transform comparisons to `FFControl`.
Malformed modern FF input reports an error and leaves existing graph contents
and node IDs intact; legacy partial-load behavior is retained for that backend.
The GUI still creates a new window before File → Load parses its input.
Known system Tk/fontconfig leaks can fail leak-enabled GUI sanitizer checks;
see the Pass 12 baseline-fixes and integration reports for exact results.

## GML parser (GNU Bison/Flex)

The retired Bison++ `GTParser`/`GTScanner` has been removed from the current
source tree. The modern generated GML importer is the only CMake backend.
Bison 3.8+ and Flex 2.6+ are required for all application builds. FF selection
is independent; Rome remains unchanged.

```sh
# Configure a fresh Debug build
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON

# Existing Pass 11 build directory: discard obsolete cached backend selections
cmake -S . -B build/gml-modern-default -G Ninja \
  -U FFGRAPH_GML_BACKEND -U FFGRAPH_ENABLE_MODERN_GML \
  -DBUILD_TESTING=ON

cmake --build build/gml-modern-default --parallel 2
ctest --test-dir build/gml-modern-default --output-on-failure
```

The generated-file corpus can be configured with
`FFGRAPH_GML_DIFFERENTIAL_PROFILE=smoke|standard|stress` (the historical cache
variable name is retained for build-directory compatibility). Tests called
`test_gml_corpus_oracle_pass11` and `test_gml_metamorphic_oracle_pass11` now
compare the production parser against independent graph oracles rather than
against a second legacy implementation.

Historical GT source files can be recovered with `git show` using a commit/tag
from before removal; see `docs/FFGRAPH-PASS11-GML-LEGACY-RETIREMENT.md`.
