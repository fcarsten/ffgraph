# CMake build (Ubuntu 24.04)

For the initial native Windows build without X11/Xpm, see
[BUILD-WINDOWS.md](BUILD-WINDOWS.md). Linux keeps its existing full test suite;
Windows starts with an explicitly selected portable suite.

The CMake build now selects the modern GNU Bison/Flex GML importer by default.
FF/sgraph also uses its modern GNU Bison/Flex importer in every build.
All three parser/scanner pairs are generated in the build tree; their legacy
implementations have been retired.
Use CMake; the historical Makefile fragments do not build the active FF/GML/Rome
importers.

The full application requires **CMake 3.24 or newer** (LINK_GROUP support).

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

The latest correctness fixes and exact layout speedups are documented in
[the layout-controls checkpoint](docs/FFGRAPH-LAYOUT-CONTROLS-CHECKPOINT.md) and
[the performance checkpoint](docs/FFGRAPH-EXACT-LAYOUT-OPTIMIZATIONS.md).
Spring keeps the selected-node pinning convention and its distinct legacy
weighted formulas. Finite quality now honors the configured maximum iteration
count; quality zero still runs until stopped. Sugiyama's configured offsets
translate the drawing and its routes consistently in either orientation.
The experimental aesthetic layouts remain parked.

The default suite now includes a broader Sugiyama corpus: 13 real FF/GML graphs
and 18 synthetic graphs, each under six configurations with reference/optimized
geometry and crossing comparisons. Inputs are stored in the repository; sibling
graph directories are not required. Run it directly with:

```sh
ctest --preset debug -R '^test_sugiyama_corpus_' -V
```

See [the corpus README](tests/sugiyama_corpus/README.md) and
[verification report](docs/FFGRAPH-SUGIYAMA-CORPUS-EXPANSION.md).

Sugiyama large-input regressions cover checked scratch dimensions, wide
barycenter sums, 20,000-deep traversals, a 1,024-level full layout and injected
array-allocation failures in headless/GUI operation:

```sh
ctest --preset debug -R 'sugiyama_large_input|sugiyama_deep_traversal|sugiyama_failure' -V
```

See [the large-input checkpoint](docs/FFGRAPH-SUGIYAMA-LARGE-INPUT-CORRECTNESS.md)
for recovery guarantees and remaining numeric/publication limits.

The numeric/publication continuation rejects unsupported numeric ranges and
restores the graphic model after renderer exceptions. Sanitizer builds also check
floating-to-integer overflow. Run its focused tests with:

```sh
ctest --preset debug -R 'sugiyama_numeric|sugiyama_publication_gui|test_mplex_stl' -V
```

See [numeric bounds and publication recovery](docs/FFGRAPH-SUGIYAMA-NUMERIC-PUBLICATION-RECOVERY.md)
for route-storage lifetime, repaint limits and measured validation coverage.

Linux native builds also exercise real OS address-space limits in isolated
subprocesses, including actual-window recovery and successful reuse:

```sh
ctest --preset debug -R '^test_sugiyama_resource_' -V
```

Sanitizer builds run the moderate successful controls and existing injected
failure tests. The OS-limit tests are native-only because `RLIMIT_AS` conflicts
with ASan's reserved shadow address space. See
[resource-limit testing and the follow-up audit](docs/FFGRAPH-SUGIYAMA-RESOURCE-LIMITS.md)
for exact coverage, refreshed performance measurements and the remaining
limits of OS resource testing.

The large-offset/4x zoom renderer overflow is now fixed at the shared Tk rendering
boundary. Unsupported projected items are temporarily hidden while world geometry
is retained; zooming or panning back into range restores them. Run its tests with:

```sh
ctest --preset debug -R '^test_renderer_coordinates' -V
```

See [renderer coordinate bounds](docs/FFGRAPH-RENDERER-COORDINATE-BOUNDS.md) for
actual 2D/3D window coverage, whole-edge suppression and remaining toolkit limits.

Ordinary C++ allocation failures are also swept through both Sugiyama entry
points and real Tk rendering, with exact model/canvas recovery checks:

```sh
ctest --preset debug -R '^test_sugiyama_scalar_failure' -V
```

See [ordinary allocation recovery](docs/FFGRAPH-SUGIYAMA-SCALAR-ALLOCATION-RECOVERY.md)
for argument-copy, busy cleanup and stream-formatting fixes, successful sorting
fallback checks and the limits of one-shot failure testing.

The latest exact Sugiyama optimization combines each crossing counter row's two
scans and accelerates active matrix comparisons. Run its focused coverage with:

```sh
ctest --preset debug -R '^test_sugiyama_(matrix_comparisons|crossings|corpus_)' -V
```

See [crossing scans and matrix comparisons](docs/FFGRAPH-SUGIYAMA-CROSSING-COMPARISON-OPTIMIZATION.md)
for unchanged geometry/crossing checks, controlled timing results and limits.

For reproducible headless import/layout profiling, build the optional driver:

```sh
cmake -S . -B build/profile -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DBUILD_TESTING=OFF -DFFGRAPH_BUILD_PROFILER=ON \
  -DCMAKE_CXX_FLAGS=-fno-omit-frame-pointer
cmake --build build/profile --target profile_graphs --parallel 2
python3 tools/profile_graphs.py --binary build/profile/profile_graphs \
  --output build/profile-smoke-local --sizes 100 --samples 5
```

Use a new output directory for each run. The runner writes raw samples,
CSV/JSON summaries, input hashes and build metadata. The previously failing
1,000-node Spring star is covered by the numerical regression described in
[the Spring fix report](docs/FFGRAPH-SPRING-NUMERICAL-STABILITY.md).
Failures produce a nonzero exit status. These are measurements, with no timing
thresholds in CTest. See [the profiling checkpoint](docs/FFGRAPH-IMPORT-LAYOUT-PROFILING.md)
for phase definitions, larger runs, CPU profiles, results and remaining limits.
The [hotspot follow-up](docs/FFGRAPH-PROFILING-HOTSPOTS-CHECKPOINT.md) adds import
scaling, captured Sugiyama matrices and the optional `profile_crossings`
executable. The validated counting approach is now used by the normal application;
see [the production optimization checkpoint](docs/FFGRAPH-SUGIYAMA-CROSSING-COUNTER-OPTIMIZATION.md)
for measured whole-layout gains and compatibility details.

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
remain under the library directory. The application locates them relative to
its installed executable, so it can launch from other working directories;
build-tree executables use the configured source resources. Existing FF_LIBDIR
and FF_ICONDIR overrides remain supported. Install-time --prefix and DESTDIR
are honored by the symlink step. See
[the runtime portability checkpoint](docs/FFGRAPH-RUNTIME-PORTABILITY-CHECKPOINT.md)
for actual clean production-only builds and staged 2D/3D startup checks.

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

## Rome parser (GNU Bison/Flex)

The Rome graph format is **deprecated**. Loading remains supported and tested;
loader performance optimization is low priority. Prefer FF/sgraph or GML for
new graph files.

Modern Rome is now the only production Rome importer, including in the GUI.
No pilot flag or backend selector is needed. Filename dispatch is preserved:
`.gml` first, then paths containing `.g` for FF, then Rome for other filenames.
Native string imports (`load(text, NO)`) remain FF.

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug -R '^test_rome_' --output-on-failure

FF_LIBDIR="$PWD/lib/ffgraph" FF_ICONDIR="$PWD/lib/ffgraph/icons" \
  ./build/debug/ffgraph tests/rome_fixtures/structure.rome
```

Rome files contain no coordinates; apply a layout to separate imported nodes.
For old build directories, discard obsolete options once:

```sh
cmake --preset debug -U FFGRAPH_ENABLE_MODERN_ROME_PILOT -U FFGRAPH_ROME_BACKEND
```

Invalid syntax, numeric overflow, NULs and unreadable files preserve the graph
and IDs. Successful merges reset old node IDs to -1, matching historical Rome.
Missing endpoints still warn and skip edges. Actual error locations and complete
long-line handling replace legacy line-1 diagnostics and line truncation.
GUI File > Load still creates its window before parsing, including on rejection.

The local annotated tag `ffgraph-pre-legacy-rome-removal` preserves the complete
legacy implementation and live differential tests at commit `5db5a38`. The
retirement report is `docs/FFGRAPH-PASS12-ROME-LEGACY-RETIREMENT.md`.

Optional external corpus checks now exercise production loading against independent
row/source-graph expectations and frozen pre-removal snapshots:

```sh
cmake --preset release -DFFGRAPH_ROME_CORPUS_DIR=../demographs/rome
cmake --build --preset release --parallel 2
ctest --preset release -R '^test_rome_external_' --output-on-failure
```

The path remains machine-local; ordinary CI uses committed fixtures/oracles.
There is no live legacy parser in these tests. See `tests/modern_rome/README.md`.
The parser-only target also builds without Tcl/Tk or X11:

```sh
cmake -S tests/modern_rome -B build/modern-rome -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/modern-rome --parallel 2
ctest --test-dir build/modern-rome --output-on-failure
```

## GML parser (GNU Bison/Flex)

The retired Bison++ `GTParser`/`GTScanner` has been removed from the current
source tree. The modern generated GML importer is the only CMake backend.
Bison 3.8+ and Flex 2.6+ are required for all application builds. FF also uses
its modern importer, as does Rome.

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
