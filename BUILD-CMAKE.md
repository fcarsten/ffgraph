# CMake build (Ubuntu 24.04)

The CMake build now selects the modern GNU Bison/Flex GML importer by default.
FF/sgraph also uses its modern GNU Bison/Flex importer in every build.
Both parser/scanner pairs are generated in the build tree; their legacy
implementations have been retired. Rome still uses its checked-in parser.
Use CMake; the historical Makefile fragments do not build the active FF/GML
importers.

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
ctest --preset release
```

Sanitizers (GCC/Clang):

```sh
cmake --preset asan
cmake --build --preset asan --parallel
ctest --preset asan
```

All test checks remain active in Release builds. With `xvfb` and `xauth`
installed, CTest also registers GUI tests. Every test has either the `headless`
or `gui` label; `ctest -N -L gui --test-dir build/asan` lists GUI coverage.

The strict `asan` preset enables leak detection for the entire suite. On the
Ubuntu 24.04 system Tk 8.6.14 build, four GUI tests report Tk/Xft/fontconfig
shutdown leaks, also reproduced without ffgraph. Use these explicit partitions
to distinguish application runtime checks from toolkit leak diagnostics:

```sh
ctest --preset asan-headless   # Strict ASan/UBSan and leak detection
ctest --preset asan-gui        # GUI ASan/UBSan runtime checks; leaks disabled
ctest --preset asan-gui-leaks  # Strict GUI leak diagnostics; known failures
```

CI requires Debug, Release, strict headless sanitizers and GUI runtime checks.
Only the separate strict GUI leak diagnostic step permits failure; its output
is retained in sanitizer log artifacts. There are no leak suppressions. A GUI
runtime pass does not establish leak freedom. See
`docs/FFGRAPH-PASS12-VERIFICATION-HARDENING.md` for measured results and limits.

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

## FF/sgraph parser (GNU Bison/Flex)

Modern FF is the only native/sgraph importer. Both `.g` file loads and
`load(serialized, NO)` (including clipboard imports) use it automatically.
There is no backend selector or pilot option. GML and Rome dispatch remain
independent.

From the repository root:

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure

FF_LIBDIR="$PWD/lib/ffgraph" FF_ICONDIR="$PWD/lib/ffgraph/icons" \
  ./build/debug/ffgraph contrib/out.g
```

For an existing Pass 12 build directory with obsolete cached options:

```sh
cmake --preset debug -U FFGRAPH_FF_BACKEND -U FFGRAPH_ENABLE_MODERN_FF_PILOT
cmake --build --preset debug --parallel 2
```

A stale `FFGRAPH_FF_BACKEND=legacy` selection fails with this reconfiguration
hint rather than silently choosing a different backend. An old `modern` value
is harmless, but clearing obsolete options is recommended.

Bison 3.8+ and Flex 2.6+ generate the parser even with `BUILD_TESTING=OFF`.
The generated archive is installed with `FFGraph`. Retired internal generated
parser/scanner classes are available in Git history at
`65e5498c40116c0ec85bccb3bbab5d060f098f57`.

Permanent tests check frozen pre-retirement graph snapshots, independent graph
and 2D/3D transform oracles, source-graph writer round trips, malformed-input
rejection, and real Tcl File → Load/Merge/Close/reopen under Xvfb. There is no
live legacy comparison backend. Syntax rejection preserves existing graph
contents and IDs. GUI File → Load still creates its window before parsing.
Known Tk/fontconfig leaks can fail strict leak-enabled GUI sanitizer checks;
see `docs/FFGRAPH-PASS12-FF-LEGACY-RETIREMENT.md` for measured results and limits.

## GML parser (GNU Bison/Flex)

The retired Bison++ `GTParser`/`GTScanner` has been removed from the current
source tree. The modern generated GML importer is the only CMake backend.
Bison 3.8+ and Flex 2.6+ are required for all application builds. FF also uses
its modern importer; Rome remains unchanged.

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
