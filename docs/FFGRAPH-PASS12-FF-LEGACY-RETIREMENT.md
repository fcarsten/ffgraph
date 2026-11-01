# Pass 12 — modern FF default and legacy FF retirement

Date: 1 November 2026. Branch: `pass12-modern-ff-parser`.
Pre-removal commit: `65e5498c40116c0ec85bccb3bbab5d060f098f57`.
The checkout was clean when this work started. The user reported that the
integrated parser worked well and explicitly requested making it the default
and removing legacy FF. No commits, pushes or tags were made in this turn.
The pre-removal commit preserves all historical sources and comparison tests.

## Implementation and scope

Modern GNU Bison/Flex FF is now the **only** native FF/sgraph loading path in
CMake builds. This applies to `.g` files, `load(serialized, NO)` and the native
clipboard import path that calls it. GML remains modern, and Rome remains
unchanged. Supported `FFGraphClass` loading/writing APIs and successful-input
FF format semantics are preserved.

Removed the six historical Bison++/flex++ files:

- `fgraph/FFParser.{cc,h,y}`
- `fgraph/FFScanner.{cc,h,l}`

The internal generated `FFControl` API disappears with these sources; no
remaining application caller or installed public graph API depends on it.
Its grammar/scanner and comparison evidence are recoverable from the commit
above, for example:

```sh
git show 65e5498c40116c0ec85bccb3bbab5d060f098f57:fgraph/FFParser.y
```

`CMakeLists.txt` always generates/links/installs `ffgraph_modern_ff`, including
with `BUILD_TESTING=OFF`. Removed the backend selector, pilot option, production
macro, obsolete test macros and conditional GUI test registration. A stale
`FFGRAPH_FF_BACKEND=legacy` (or other non-modern value) is rejected with a cache
cleanup command rather than silently loading through a different parser. Old
cached `modern` values are harmless; fresh builds have no backend selection.

`FFGraphClass::load()` now uses the already tested modern FF path
unconditionally, retaining GML-first dispatch, Rome fallback, load lifecycle
events, deferred ID reset for modern imports, window update suppression and
restoration, size adjustment, error reporting and final redraw. The obsolete
`FFParser` friend declaration was removed from the public graph-class header.
No graph/container ownership implementation changed.

The modern grammar/scanner and graph adapter implementation did not change.
Their previously documented numeric-range rejection, diagnostic line counts,
per-import unknown-label warnings, long-line behavior and syntax-failure
atomicity are now unconditional FF behavior. There is no automatic fallback.
Allocation/callback exceptions during replay are still not transactional.

CI's old legacy/modern matrices were removed; Debug and strict ASan/UBSan jobs
exercise the mandatory importer and permanent tests. CI YAML parsed locally,
but GitHub Actions was **not** run and nothing was pushed. Historical Makefile
fragments had no FF-specific source dependencies to remove. Their generic
Bison++/flex++ variables were retained for the unchanged Rome implementation;
CMake is the supported build for active FF/GML importers.

`BUILD-CMAKE.md` documents normal default builds, launch commands and stale-cache
migration. The integration report is marked as historical/superseded. Rome,
SPQR, GUI menu callbacks and the parked WSLg inspector investigation are outside
this patch.

## Permanent graph expectations captured before deletion

Before deleting legacy FF, the existing independent-comparison executable was
built against the **real legacy parser** with a temporary output hook. It
invoked `FFControl` directly and recorded ten graph snapshots:

- All nine hand-written native FF fixtures, including the GUI fixture.
- The historical `contrib/out.g` with its 499 nodes and 499 edges.

Capture command executed before removal:

```sh
cmake --build build/ff-legacy-pilot --target test_ff_compatibility_pass12 --parallel 2
build/ff-legacy-pilot/tests/modern_ff/test_ff_compatibility_pass12 \
  --record-legacy-oracles tests/modern_ff/oracles
```

The output hook and comparison executable were then removed/replaced. The new
code cannot regenerate expected data from the production parser during a test
or build. Frozen `.snapshot` files and `tests/modern_ff/oracles/README.md` retain
provenance and SHA-256 hashes of every input and snapshot.

Snapshots describe actual graph construction with graphic and custom `Probe`
labels installed. They preserve graph name/directedness, numeric node IDs,
names (including raw multiline text), start/end nodes, label payloads, node
coordinates/ranges/colors/icons/masks/shapes, edge endpoints/multiplicity,
weights/colors and complete routes. Node/edge records are sorted to avoid
allocation/iteration-order assumptions. Existing full-precision floating
serialization is retained; observed fixture values are deterministic across
the tested Debug and Release builds.

`test_ff_graph_regression_pass12` replaces the obsolete differential driver.
It checks the production file **and string** loaders against these frozen
legacy observations, plus independent hand-written expectations for topology,
geometry, weights, labels, icon/mask behavior and unusual inclusive-scanner
comments. It retains comparisons between public loading and direct modern
replay with/without graphics and with/without an existing merge graph, as well
as 24 writer-generated source-graph round trips and reserialization checks.

The direct-modern-versus-production comparisons are **API consistency tests**,
not an independent parser compatibility proof. Frozen pre-removal data and
hand-written/source-graph expectations provide the independent references.

`test_ff_corpus_oracle_pass12` replaces the old legacy-corpus CTest; modern
rejection/recovery/stack/numeric tests remain separately registered as
`test_ff_modern_failures_pass12`. All three now run in every testing build.

## Other preserved and strengthened regressions

- `test_ff_production_backend_pass12` now always requires modern rejection
  behavior, successful append/name retention, balanced load events, original
  node identity/ID preservation on failure, failed-file-open preservation,
  overflow rejection and long-line support. Legacy conditionals are gone.
- `test_graph_io_pass10` retains all eight malformed cases, twenty repetitions
  and both input paths (**320 rejections**), now through the public file/string
  loader instead of removed FFControl APIs. Each rejection must preserve an
  existing graph and numeric ID. Twenty valid recovery parses and all prior
  FF/GML/historical round trips remain. Error-handler replacement is scoped.
- `test_ff_window_oracle_pass12` replaces the obsolete live legacy-pair test.
  It covers both 2D and 3D windows, file/string imports and update initially
  ON/OFF (**eight cases**). Independent expectations check node/edge topology,
  names, geometry, color/radius, bends, weights, start/end designation, scale,
  rotations and label offsets. Independent double-precision matrix arithmetic
  checks transform order/axis signs and the forward/inverse product, with
  `1e-5` tolerance for the float-based window matrices. It preserves complete
  graph/window state on malformed input and verifies root reclamation.
- The real Tcl FF File → Load/Merge/Close/reopen workflow is now unconditional
  when Xvfb is available. GML workflows and lifecycle/invariant/container tests
  remain registered. No failing test was disabled or suppressed.

## Tests actually executed

Host/toolchain: Ubuntu 24.04, GCC 13.3.0, CMake 3.28.3/Ninja, Bison 3.8.2,
Flex 2.6.4, Tcl/Tk 8.6.14 and Xvfb.

Before removal: integrated modern Debug **48/48 passed**, and five headless FF
ASan/UBSan tests passed with leak detection. The actual legacy oracle-capture
build and capture command succeeded before the six source files were deleted.

| Post-removal check | Result |
| --- | --- |
| Default Debug configure/build/full CTest | **48/48 passed** |
| Release configure/build/full CTest | **48/48 passed** |
| ASan/UBSan configure/build | PASS |
| Full ASan/UBSan with `detect_leaks=1` | **44/48 passed**; four GUI font leaks below |
| Supplemental full ASan/UBSan with `detect_leaks=0` | **48/48 passed**, no runtime sanitizer errors |
| Fresh default Release, `BUILD_TESTING=OFF`, no FF selector/pilot flags | PASS; real GNU Bison/Flex generation and full application link |
| Install the fresh no-test build into `/tmp/ff-pass12-retirement-install` | PASS; application/core/generated parser archives installed |
| Existing cached legacy configuration | Rejected as expected with explicit cache-cleanup guidance |
| `nm -C` of Debug `libFFGraph.a` and `ffgraph` | No FFControl/FFParser/FFScanner symbols |
| Active-source/build/CI reference scan | No callers/includes/targets depending on retired files/classes |
| CI YAML and whitespace checks | PASS |

Strict LeakSanitizer failures are the known Tk/Xft/fontconfig dependency leaks:

- `test_load_window_lifetime_pass11`: 9,367 bytes / 304 allocations.
- `test_ff_window_oracle_pass12`: 53,396 bytes / 1,970 allocations.
- `test_gui_file_workflow_pass11`: 59,697 bytes / 2,208 allocations.
- `test_ff_gui_file_workflow_pass12`: 59,697 bytes / 2,208 allocations.

The new window oracle uses one independently checked window per case rather
than a production/legacy pair; its lower leak total reflects eight roots
instead of sixteen. The same input/update/dimension combinations remain.
The earlier dependency-only reproducer and analysis are in the baseline-fixes
report. No invalid-access ASan error or UBSan runtime error appeared in the
strict run. The supplemental no-leak run is a separate result and does not
make the strict suite green. Headless graph/oracle/rejection/ownership tests
all passed **with leak detection enabled**. No test was skipped by CTest;
optional large GUI/layout corpus profiles remained OFF.

## Commands and rebuilding existing directories

Commands executed for the preset builds (clear old cached selectors once):

```sh
cmake --preset debug -U FFGRAPH_FF_BACKEND -U FFGRAPH_ENABLE_MODERN_FF_PILOT
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure

cmake --preset release -U FFGRAPH_FF_BACKEND -U FFGRAPH_ENABLE_MODERN_FF_PILOT
cmake --build --preset release --parallel 2
ctest --test-dir build/release --output-on-failure

cmake --preset asan -U FFGRAPH_FF_BACKEND -U FFGRAPH_ENABLE_MODERN_FF_PILOT
cmake --build --preset asan --parallel 2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
ctest --preset asan --output-on-failure

# Separate runtime verification; strict leak failures remain reported above.
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
ctest --preset asan --output-on-failure
```

Additional actual build/installation checks:

```sh
cmake -S . -B build/ff-retirement-clean -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
  -DCMAKE_INSTALL_PREFIX=/tmp/ff-pass12-retirement-install
cmake --build build/ff-retirement-clean --parallel 2
cmake --install build/ff-retirement-clean

# Expected failure: this existing directory still cached backend=legacy.
cmake -S . -B build/ff-legacy-pilot

git diff --check
```

Tests needing Xvfb/LeakSanitizer ran with approval outside sandbox display and
tracing restrictions. Logs are ephemeral `/tmp/ff-pass12-retirement-*`.
Generated parser sources, old build objects and binaries were not added to
Git. The frozen snapshots are intentional permanent test data.

Normal fresh builds now need **no FF-specific options**. To launch the rebuilt
modern-default application from the repository root:

```sh
FF_LIBDIR="$PWD/lib/ffgraph" FF_ICONDIR="$PWD/lib/ffgraph/icons" \
  ./build/debug/ffgraph contrib/out.g
```

## Remaining limits

Full leak-enabled GUI sanitizer CTest remains red due to the reproduced system
font leaks; no dependency upgrade or suppression was introduced. Manual desktop
coverage beyond the user's positive report, installed runtime asset discovery,
alternative compilers/platforms and downstream static-library consumers were
not verified. Legacy FF sources and live differential tests are now historical
Git artifacts, not runtime fallbacks. Golden snapshots must be reviewed as
format-contract data when intentionally changing input fixtures or graph
construction behavior. Public importer/direct-replay equality alone must not
be advertised as independent compatibility evidence.

The modern importer's existing exception/callback limitations and GUI behavior
on a failed new-window Load remain as documented in the integration report.
Rome, experimental SPQR and the parked inspector/WSLg issue remain untouched.
