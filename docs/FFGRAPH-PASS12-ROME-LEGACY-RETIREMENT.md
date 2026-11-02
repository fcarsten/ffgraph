# Pass 12 — modern Rome default and legacy retirement

Date: 2 November 2026. Branch: `pass12-modern-rome-parser`.
Starting commit: `5db5a38e9c5c4c5bb176ec08c445ee2ea9b7d248`, clean checkout.
The user explicitly requested making modern Rome default and removing legacy
without a separate opt-in production checkpoint, and requested a rollback tag.

## Rollback reference and authorization

Created the annotated **local** tag `ffgraph-pre-legacy-rome-removal` at the
starting commit before changing sources. It preserves all six old parser/
scanner files, their CMake integration and the live differential harness.
The tag was not pushed. No commit or branch push was made by the agent.

Read the prior pilot and real-corpus reports and actual Git/CMake/loading/test
sources before editing. Pre-edit configure/build/tests passed: Debug 56/56,
Release 58/58 (with external corpus), strict headless ASan/UBSan/leaks 49/49.

Recovery/inspection examples:

```sh
git show ffgraph-pre-legacy-rome-removal:fgraph/RomeParser.y
git show ffgraph-pre-legacy-rome-removal:fgraph/RomeScanner.l
git show --stat ffgraph-pre-legacy-rome-removal
```

Recover the tagged checkout/build configuration together if an actual legacy
comparison is needed; restoring only obsolete files into modern CMake does not
select them. The existing FF/GML pre-removal tags were left unchanged.

## Implementation and compatibility policy

Rome was the last production parser still depending on checked-in Bison++/
flex++ output. The modern pilot had passed actual legacy comparisons, generated
parser tests, independent graph oracles and all 13,767 supplied files. This
checkpoint makes that tested implementation unconditional and closes the
production/GUI integration gap while preserving historical expectations.

Removed `fgraph/RomeParser.{cc,h,y}` and `RomeScanner.{cc,h,l}`. Their internal
generated RomeControl/RomeParser/RomeScanner classes were not installed public
graph APIs; remaining application callers/includes have been removed. Public
FFGraphClass loading/writing interfaces, graph formats and Tcl menu dispatch
remain. No graph/container ownership implementation changed. SPQR remains
excluded; no inspector/WSLg behavior was modified.

`CMakeLists.txt` always adds the modern Rome generator, compiles its graph
adapter into FFGraph and links/installs `ffgraph_modern_rome`, even with
BUILD_TESTING=OFF. The pilot option and redundant separate pilot adapter archive
are removed. A requested non-modern FFGRAPH_ROME_BACKEND fails configure with
cache-cleanup guidance; there is no backend selector or automatic fallback.
Obsolete cached pilot values no longer control the build.

`FFGraphClass::load()` keeps existing dispatch: `.gml`, then paths containing
`.g`, then Rome. Native string imports remain FF. Rome files are read completely,
parsed, and replayed through the same tested adapter with successful-import ID
reset. The legacy early reset is removed: unreadable files and syntax/range/NUL
rejection leave destination content, IDs and directedness unchanged. Missing
endpoints still skip edges and print `Couldn't find nodes for edge` once each,
without a user error. Warning delivery occurs after replay. The existing
`Parsing Rome Graph` message is retained.

START_LOAD/END_LOAD, window update suppression/restoration, size adjustment and
final redraw remain in the loader. File > Load still creates its window before
parsing; rejection does not cancel that existing GUI behavior. Replay remains
nontransactional for allocation failure or throwing application callbacks.

Intentional changes from legacy are now production policy: syntax rejection
does not retain prefixes; overflow rejects; diagnostics use actual locations;
long lines are consumed instead of silently truncated; embedded NUL rejects.
Successful-input semantics remain compatible on the tested corpus and fixtures.
The modern grammar/scanner and row replay implementation themselves did not
change, apart from updating an adapter comment to reflect production use.

## Permanent evidence captured before deletion

A temporary output hook in the **actual legacy** compatibility driver recorded
16 snapshots: all eight accepted synthetic fixtures, each with and without
graphics, on fresh graphs. They include graph properties, IDs/names, adjacency,
weights, custom payloads, geometry/ranges/colors/icons/masks/shapes and routes.
A second process produced byte-identical files. The hook was removed afterward;
expected data are never generated from the modern parser during build/tests.

`tests/modern_rome/oracles/README.md` records input/output SHA-256 hashes and tag
provenance. The snapshots total 4,335 bytes. Capture executed before removal:

```sh
cmake --build build/debug --target test_rome_compatibility_pass12 --parallel 2
build/debug/tests/modern_rome/test_rome_compatibility_pass12 \
  --record-legacy-oracles tests/modern_rome/oracles
```

`test_rome_graph_regression_pass12.cpp` replaces the old differential driver.
It checks frozen graphs, independent topology/source-graph expectations,
192 fixture combinations, 128 generated API/oracle imports, callbacks that change
IDs, duplicate lookup, sentinel collisions and rejection/recovery. Direct replay
versus public file loading is now an **API consistency comparison**, not a live
independent-parser comparison. Historical differential evidence remains in the
tag and earlier reports.

External corpus tests use the production filename loader for every file and an
independent numeric-row oracle. The matrix now has two current API paths rather
than three old controller paths: 110 selected files × graphics/bare × fresh/merge
× direct/production = **880 combinations**. No dataset file, malformed case or
duplicate copy was dropped. The 50 malformed files now preserve API destinations
as well as direct-replay destinations. Out-of-range numeric rows are modeled as
rejection without invoking any legacy conversion.

`test_rome_production_backend_pass12` replaces the legacy baseline with production
dispatch, all newline variants, warning counts, actual error locations, merge
identity, failed-open preservation and recovery assertions. A native-string
check explicitly proves Rome-looking text still goes to FF.

## GUI verification

New `test_rome_window_oracle_pass12` checks 2D/3D windows with update ON/OFF,
independent topology, preserved existing graphics, balanced load events, atomic
rejection, recovery/second merge, update/transform state and interpreter cleanup.

The existing Tcl File workflow harness now supports a templated 3D window via
its test-only `--3d` argument; GML/FF 2D flows retain their existing behavior.
New Rome 2D and 3D CTests exercise actual File/Load/Merge/Close/reopen and nested
close callbacks using the real menu commands with only the file picker stubbed.
Both modes were also run with the supplied 100-node/119-edge file
`gfinali/graficon100nodi/grafo10106.100` as load/merge input.

Root CI drops the obsolete pilot flag and runs mandatory modern production tests.
The GUI runtime/leak diagnostic separation is unchanged; no suppressions were
introduced. CI YAML parsed locally; this patch was not pushed or run remotely.

## Tests actually executed

Ubuntu 24.04, GCC 13.3.0, CMake 3.28.3, Ninja, GNU Bison 3.8.2, Flex 2.6.4 and
system Tcl/Tk 8.6.14. Xvfb/LeakSanitizer runs required execution outside sandbox
display/tracing restrictions. All Release checks remain active.

| Check | Result |
|---|---|
| Full Debug CMake/build/CTest | **59/59 passed** |
| Full Release CMake/build/CTest, corpus path configured | **61/61 passed** |
| Strict headless ASan/UBSan/leaks | **49/49 passed** |
| GUI ASan/UBSan runtime tests, leaks off | **10/10 passed** |
| Full strict ASan/UBSan/leaks | **52/59 passed**, seven toolkit leak diagnostics below |
| Whole supplied collection, production loader, strict ASan/UBSan/leaks | **13,767 expected outcomes**, no sanitizer errors/leaks |
| Real-corpus graphics/merge/API matrix, strict sanitizers | **880/880 passed** |
| 100-node real file through actual Tcl 2D/3D workflows | PASS in both modes |
| Fresh Release BUILD_TESTING=OFF build and staged install | PASS; application and all three parser archives installed |
| Standalone Rome parser build/CTest | **1/1 passed**, no Tcl/Tk/X11 dependency |
| Stale legacy backend request | Rejected as intended with cleanup guidance |
| nm on Debug/Release/ASan/fresh app | Modern Rome present; old classes absent |
| CTest label audit | 49 headless + 10 GUI; Release adds two headless corpus tests |
| Dataset manifest, YAML and whitespace | Unchanged corpus; YAML parse and git diff --check pass |

Whole-corpus outcomes remain 13,717 accepted graphs (694,355 nodes/935,442 edges)
and 50 correct rejections. Its raw manifest remains
`a09ea82623d21f26289c0b7e05e977ed38d93061414902eb2487b27c22f33941`.

Strict GUI leak totals:

| Test | Bytes / allocations |
|---|---:|
| test_load_window_lifetime_pass11 | 9,367 / 304 |
| test_ff_window_oracle_pass12 | 53,396 / 1,970 |
| test_rome_window_oracle_pass12 | 28,242 / 1,018 |
| test_gui_file_workflow_pass11 | 59,697 / 2,208 |
| test_ff_gui_file_workflow_pass12 | 59,697 / 2,208 |
| test_rome_gui_file_workflow_2d_pass12 | 59,697 / 2,208 |
| test_rome_gui_file_workflow_3d_pass12 | 59,697 / 2,208 |

These match the known Tk/Xft/fontconfig allocation pattern; the three additional
failures correspond to newly registered GUI coverage. No ASan invalid-access or
UBSan runtime error appeared. GUI runtime passes do not establish leak freedom,
and strict leak CTest is still not green. Existing diagnostic CI policy remains.

## Clean install and application startup limits

The fresh no-test build installed under `/tmp/ff-rome-retire-install`, including
the ffgraph3D symlink and `libffgraph_modern_rome.a`. Installed 2D/3D applications
were launched from `/tmp` with explicit FF_LIBDIR/FF_ICONDIR pointing at the
unmodified installed assets. Both reached `ready loading` for the real 100-node
file, then were stopped externally as controlled startup probes.

An earlier auto-close probe using a temporary copied resource script timed out
after 20 seconds and was **not counted as a successful shutdown test**. A follow-up
compared a cached pre-retirement executable (confirmed to contain RomeControl)
with the modern executable: both loaded the file, ran the close-window callback,
and remained running. This probe therefore does not demonstrate a parser-switch
regression or verify application exit. No product resource or shutdown code was
changed. Installed startup with stock resources passed; interactive application
shutdown and default asset discovery without overrides remain unverified. The
library/Tcl workflows separately verify normal window close/reopen cleanup.

## Reproduction commands

From the repository root:

```sh
cmake --preset debug -U FFGRAPH_ENABLE_MODERN_ROME_PILOT
cmake --build --preset debug --parallel 2
ctest --preset debug --parallel 2
cmake --preset release -U FFGRAPH_ENABLE_MODERN_ROME_PILOT \
  -DFFGRAPH_ROME_CORPUS_DIR=/home/carsten/work/demographs/rome
cmake --build --preset release --parallel 2
ctest --preset release --parallel 2
cmake --preset asan -U FFGRAPH_ENABLE_MODERN_ROME_PILOT
cmake --build --preset asan --parallel 2
ctest --preset asan-headless --parallel 1
ctest --preset asan-gui --parallel 1
ctest --preset asan-gui-leaks --parallel 1  # Seven known toolkit leak failures
ctest --preset asan --parallel 1            # Same seven, full strict suite

ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
build/asan/tests/modern_rome/test_rome_graph_regression_pass12 \
  --corpus /home/carsten/work/demographs/rome
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
build/asan/tests/modern_rome/test_rome_graph_regression_pass12 \
  --corpus-matrix /home/carsten/work/demographs/rome

cmake -S . -B build/rome-modern-clean -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=/tmp/ff-rome-retire-install
cmake --build build/rome-modern-clean --parallel 2
cmake --install build/rome-modern-clean
```

Standalone commands remain in BUILD-CMAKE.md. Real-menu commands used the same
Xvfb runner and FF_LIBDIR/FF_ICONDIR as CTest, with both real-file arguments
pointing to the 100-node path above and the invalid argument pointing to
`tests/rome_fixtures/partial-error.rome`; the 3D run appended `--3d`.
Ephemeral logs use `/tmp/ff-rome-retire-*.log`.

## Remaining scope

The source corpus, generated outputs and test logs are not vendored into Git.
Allocation/callback exception injection, I/O failure mid-read, alternate toolchains,
huge inputs near Flex's int cap, TSan and optional large GUI stress profiles were
not tested here. Resource-path portability and the CMake minimum/LINK_GROUP
mismatch remain follow-up work. The parked inspector issue and SPQR remain
outside this checkpoint.
