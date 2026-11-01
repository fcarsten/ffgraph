# Pass 12 — selectable modern FF production integration

Date: 1 November 2026. Branch: `pass12-modern-ff-parser`.
Starting HEAD: `5688f50809d1fb96b3315221bb7b55e69a1f9578`.
The checkout was clean; the earlier pilot and baseline fixes were committed
by the user. No commits, pushes, tags or legacy-parser removal were performed
in this checkpoint.

## Problem and implementation

The previous `FFGRAPH_ENABLE_MODERN_FF_PILOT` option built a parser and graph
comparison tests but could not select modern FF in the app. Production
`FFGraphClass::load()` still invoked `FFControl` for every native import.

`FFGRAPH_FF_BACKEND=legacy|modern` now selects the FF/sgraph backend at CMake
configure time. **The default remains `legacy`.** Invalid selector values
fail configuration explicitly. The configure output reports the selected
backend. There is no runtime selector and no automatic fallback after a
modern parse failure.

With `modern`, CMake generates the parser/scanner in the build tree, adds the
existing FF graph adapter to `FFGraph`, and links `ffgraph_modern_ff` through
the application and graph tests. This also works with `BUILD_TESTING=OFF` and
the pilot flag OFF. The generated archive is installed with `FFGraph`.
Legacy parser/scanner sources remain checked in and built for the comparison
oracle and explicit fallback. Legacy-default application link dependencies
do not include the modern FF archive.

`FFGraphClass::load()` dispatches both `.g` files and `load(serialized, NO)`
through the selected backend. The latter also covers the native clipboard
import path. Modern files are read into a string and parsed before graph
operations are replayed. `START_LOAD`/`END_LOAD`, window update suppression,
restoration of the previous update flag, window-size adjustment and final
redraw remain in the existing loader wrapper. File-type matching keeps the
existing GML-first, FF-second, Rome-otherwise order.

Modern FF resets destination IDs only after successful parsing, as its
adapter already did in the pilot. Syntax errors and failed file opens report
through `ffError` without resetting existing IDs or applying partial graph or
window operations. Legacy FF retains its early ID reset and partial-load
behavior. Public loading APIs and successful-input format semantics are
unchanged. Modern numeric-range checks, accurate multiline diagnostics,
per-import unmatched-label warnings and long-line handling remain the pilot's
previously documented differences.

GML dispatch/adapter, Rome parser, GUI callbacks, layout implementations,
container ownership and SPQR exclusion have no implementation changes.
The parked WSLg inspector issue was not investigated.

## Files changed

- Root `CMakeLists.txt`: backend cache selector and validation; production
  source/link/install wiring; public-loader and live-window test registration;
  modern-FF invocation of the existing Tcl GUI workflow driver.
- `fgraph/FFGraphClass.cc`: selected FF dispatch and deferred modern ID reset,
  retaining the existing loader lifecycle wrapper.
- `fgraph/modern/ff_graph_adapter.h`: update the obsolete pilot-only comment.
  The adapter implementation and generated grammar/scanner sources did not change.
- `tests/modern_ff/CMakeLists.txt`: share the adapter from `FFGraph` in modern
  production builds; continue compiling it only into the comparison test in
  isolated legacy pilot builds.
- `tests/modern_ff/test_ff_compatibility_pass12.cpp`: independently invoke
  **actual `FFControl`**, with the historical ID-reset policy, instead of using
  the now-selectable loader for the legacy oracle. Without this correction,
  modern builds could accidentally compare modern parsing with itself.
- `tests/test_ff_production_backend_pass12.cpp`: independent public-loader
  oracles and selected failure policy, registered for either backend.
- `tests/test_ff_window_parity_pass12.cpp`: actual 2D/3D window and graph
  comparisons, registered when Xvfb is present for either backend.
- `tests/modern_ff/fixtures/gui.g`: graph/window transforms, offsets, nodes,
  3D geometry, bends, weights, parallel edges and a loop. The existing valid
  differential corpus now includes this ninth fixture.
- `tests/gui_fixtures/truncated-ff.g`: intentionally invalid GUI import with
  earlier would-be graph/window mutations and missing `END`.
- `tests/test_gui_file_workflow_pass11.cpp`: format-neutral usage/error text
  and comments; the same real Tcl workflow runs with native FF fixtures.
- `.github/workflows/ci.yml`: explicit legacy/modern FF matrices for Debug and
  ASan/UBSan, with comparison tests enabled. YAML parsed locally; GitHub Actions
  was **not** executed and no code was pushed. Strict leak checks remain enabled.
- `BUILD-CMAKE.md` and the previous pilot report: build/run/fallback guidance
  and a pointer to this checkpoint. Obsolete GML fallback prose was corrected.

## Regression evidence

### Public loader and independent graph construction

`test_ff_production_backend_pass12` runs successful and rejected imports ten
times through **both file and string paths**. Independent expectations check
names, numeric IDs, directedness, endpoint membership, loops/parallel edges,
node coordinates, route bends, weights and start/end designation. Successful
merge retains existing node objects/names, resets their numeric IDs and keeps
the destination graph name. A graph label observes balanced load lifecycle
events on success and rejection.

Rejection checks prove which backend actually executed: modern preserves
original IDs/counts/directedness, while legacy exhibits its established
partial mutation. A nonexistent `.g` path checks the same ID-reset difference.
Modern-only overflow rejection and a 10,000-character node name exercise the
real selected loader rather than a standalone parse call.

Existing graph-I/O, GML, graph invariant, layout and lifetime regressions run
in the full suites. The FF differential corpus, historical `contrib/out.g`
(499 nodes/499 edges), writer-generated graphs and independent oracles also
pass with modern production selected and with an isolated legacy pilot.

### Live 2D/3D graph/window comparisons

`test_ff_window_parity_pass12` creates separate `FFTclWindow` and
`FFTcl3DWindow` pairs under Xvfb. One member loads through the production API;
the other independently invokes `FFControl` on the identical bytes. It covers
file and string imports with update initially ON and OFF.

Comparisons include constructed graph snapshots, directedness, start/end
markers, edge weights/colors/routes, scale, rotations, label offsets and both
forward/inverse transformation matrices. Independent expected scale/angle/
offset values supplement the differential comparison. All eight pairs close
and reclaim their roots. In modern builds, truncated input containing scale,
offset and node changes preserves the graph, original node identity, complete
window state and update flag.

`test_ff_gui_file_workflow_pass12` uses the existing real Tcl
`recall modPress l/m/w` driver with FF files. It checks file-picker cancellation,
four load/merge/close/reopen cycles, surviving new windows, repeated malformed
merge rejection without node-ID/name/address changes, continued window use,
parent/child cleanup and nested Tcl close callbacks. Only the file picker is
replaced by the test. The original GML workflow is still a separate test.
This FF malformed-merge preservation test is registered for the modern backend;
legacy's different invalid-load contract is checked by the public-loader test.

## Tests actually executed

Ubuntu 24.04, GCC 13.3.0, CMake 3.28.3/Ninja, Bison 3.8.2, Flex 2.6.4,
Tcl/Tk 8.6.14 and Xvfb. No dependency upgrades or leak suppressions were added.

Before source changes: current default Debug **42/42 passed**; focused FF
ASan/UBSan with leak detection **4/4 passed**. The previous full-sanitizer
Tk/fontconfig failures remain recorded in the baseline-fixes report.

| Configuration/check | Result |
| --- | --- |
| Default Debug, legacy FF, pilot OFF: configure/build/full CTest | PASS; **44/44** |
| Modern FF Debug: configure/build/full CTest | PASS; **48/48** |
| Modern FF Release: configure/build/full CTest | PASS; **48/48** |
| Modern FF ASan/UBSan: configure/build | PASS |
| Modern FF full ASan/UBSan, `detect_leaks=1` | **44/48 passed**; four GUI leak failures below |
| Modern FF full ASan/UBSan, supplemental `detect_leaks=0` | PASS; **48/48**, no runtime sanitizer errors |
| Isolated legacy pilot: build comparison target and run three FF CTests | PASS; **3/3**; the full suite was not built/run in that directory |
| Fresh modern Release, `BUILD_TESTING=OFF`, pilot OFF | PASS; real Bison/Flex generation and application link |
| Install that no-test build into `/tmp/ff-pass12-modern-install` | PASS; generated FF archive installed with the application/core libraries |
| Invalid backend selector | Rejected as expected with `Unsupported FFGRAPH_FF_BACKEND` |
| Application link dependency inspection | Modern FF archive present only in modern application build |
| CI YAML parsing and `git diff --check` | PASS |

Strict sanitizer GUI failures are all LeakSanitizer reports from the known
Tk/Xft/fontconfig dependency stack, exposed by real menus/fonts:

- `test_load_window_lifetime_pass11`: 9,367 bytes / 304 allocations.
- `test_ff_window_parity_pass12`: 103,724 bytes / 3,874 allocations.
- `test_gui_file_workflow_pass11`: 59,697 bytes / 2,208 allocations.
- `test_ff_gui_file_workflow_pass12`: 59,697 bytes / 2,208 allocations.

No ASan invalid-access or UBSan runtime error appeared in that run. The
supplemental no-leak run is reported separately and does **not** make the
strict leak-enabled suite green. The public-loader, headless comparison,
legacy malformed-input ownership and modern rejection tests all passed with
leak detection enabled. No tests were disabled or marked as expected failures.
Existing inspector tests ran as ordinary full-suite coverage only.

## Build and verification commands

Actual current build directories used for full verification:

```sh
# Default remains legacy; this directory's pilot option is OFF.
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure

# The previous pilot directory now explicitly selects the production backend.
cmake -S . -B build/ff-pilot-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON -DFFGRAPH_FF_BACKEND=modern
cmake --build build/ff-pilot-debug --parallel 2
ctest --test-dir build/ff-pilot-debug --output-on-failure

cmake --preset release -DFFGRAPH_FF_BACKEND=modern
cmake --build --preset release --parallel 2
ctest --test-dir build/release --output-on-failure

cmake --preset asan -DFFGRAPH_FF_BACKEND=modern
cmake --build --preset asan --parallel 2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
ctest --preset asan --output-on-failure

# Separate runtime check; strict leak failures remain reported above.
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
ctest --preset asan --output-on-failure
```

Additional build/integration checks actually executed:

```sh
cmake -S . -B build/ff-legacy-pilot -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON -DFFGRAPH_FF_BACKEND=legacy \
  -DFFGRAPH_ENABLE_MODERN_FF_PILOT=ON
cmake --build build/ff-legacy-pilot --target test_ff_compatibility_pass12 --parallel 2
ctest --test-dir build/ff-legacy-pilot \
  -R '^test_ff_(compatibility|legacy_corpus|modern_failures)_pass12$' --output-on-failure

cmake -S . -B build/ff-modern-no-tests -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF -DFFGRAPH_FF_BACKEND=modern \
  -DFFGRAPH_ENABLE_MODERN_FF_PILOT=OFF \
  -DCMAKE_INSTALL_PREFIX=/tmp/ff-pass12-modern-install
cmake --build build/ff-modern-no-tests --parallel 2
cmake --install build/ff-modern-no-tests

# Expected configuration error, not a successful build:
cmake -S . -B build/ff-invalid-backend -G Ninja -DFFGRAPH_FF_BACKEND=invalid

ninja -C build/ff-pilot-debug -t query ffgraph
ninja -C build/debug -t query ffgraph
git diff --check
```

Xvfb and LeakSanitizer tests ran with approval outside sandbox display/tracing
restrictions. Logs are ephemeral `/tmp/ff-pass12-integration-*`; generated
sources, caches and binaries remain outside version-controlled source files.

To launch the tested modern Debug application yourself from the repo root:

```sh
FF_LIBDIR="$PWD/lib/ffgraph" FF_ICONDIR="$PWD/lib/ffgraph/icons" \
  ./build/ff-pilot-debug/ffgraph contrib/out.g
```

File → Load and Merge now use modern FF in that binary. To return the same
build directory to legacy, configure with `-DFFGRAPH_FF_BACKEND=legacy` and
rebuild. CMake preserves existing cached choices if the option is omitted.
`FFGRAPH_ENABLE_MODERN_FF_PILOT=ON` alone does not select modern production FF.

## Remaining limits / retirement boundary

- Full leak-enabled GUI sanitizers remain red due to the reproduced dependency
  leaks. No toolkit cleanup workaround or dependency upgrade was attempted here.
- Manual interactive WSLg use, installed-app runtime data discovery, system
  installation outside the temporary prefix and downstream static-library
  consumer linkage were not verified. Tk/Xvfb tests verify the real graph/GUI
  classes and Tcl callbacks, not an interactive desktop session.
- Actual desktop clipboard selection retrieval was not exercised; the shared
  `load(String, NO)` native import path it calls was tested, including windows.
- GUI icon/mask rendering and layout-specific imported labels were not added
  to the window fixture; existing headless icon/mask and label-callback oracles
  remain in the full differential suite.
- Syntax/file rejection preserves graph contents and IDs, and failed parsing
  causes no graph/window operation replay. Arbitrary load-event listeners can
  still act on START/END notifications. Allocation or callback exceptions during
  replay are not transactional and loader update-state restoration on such
  exceptions is not newly guaranteed.
- File → Load still creates a window before parsing; an invalid new load can
  leave a blank window, as with GML. The GUI rejection test asserts atomicity
  for Merge, not atomic creation/removal of the Load window itself.
- The modern FF pilot's documented out-of-range, diagnostic, warning-suppression
  and feeder-limit differences remain opt-in production behavior. No valid
  differential-corpus graph differences were found.
- Keep legacy as default and retain all legacy FF sources until the separate
  switch/retirement checkpoint and broader interactive validation are approved.
  Rome, SPQR and the parked inspector issue stay out of scope.
