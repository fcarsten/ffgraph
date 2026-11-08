# Sugiyama large-input correctness: first checkpoint — 4 November 2026

Started from clean branch `import_and_layout_profiling`, HEAD `b74626b`.
The user chose large-input correctness after the performance/corpus checkpoints.
Pre-change Debug configure/build/full CTest passed **77/77**. No commits or pushes
made. This checkpoint covers scratch dimensions, barycenter sums, deep traversal
and preparation-phase allocation recovery; remaining limits are explicit below.

## Reproduced failures

Small probes linked to the actual baseline library demonstrated:

| Probe | Baseline result | Required result |
| --- | --- | --- |
| One row with 70,000 unit entries | Barycenter approximately **-26,356.2** | **35,000.5** |
| Reduction with maximum layer index 2 and width 50,000 | Uncaught `bad_array_new_length`, process abort | Reject dimensions before allocation/indexing |
| DFS on a 20,000-node chain, 512 KiB stack | SIGSEGV | Complete with historical visitation/completion order |
| Rank propagation on that chain/stack | SIGSEGV | Ranks 0 through 19,999 |

The traversal failures also reproduced with 256 KiB stack. Core dumps were
disabled for the isolated negative probes. They did not allocate giant matrices.

Additional controls compiled the real baseline `FFsugiyama.cc` and
`FFimprove_position.cc` from `b74626b`, linking each ahead of the current archive
into the new regression binaries. Expected failures were observed:

- The old layout straightened existing bent routes before its first scratch
  allocation. An injected allocation failure failed the original-geometry check.
- The old coordinate barycenter overflowed when summing three coordinates of
  1,000,000,000, failing the expected average of 1,000,000,000.

These intentionally failing local controls are not registered as CTests. Commands,
old source copies, outputs and exit codes are retained under
`build/large-input-checkpoint/`. No failing production test was disabled.

## Bounds and numeric changes

Private `flayout/FFsugi_sizes.h` checks nonnegative addition/multiplication before
evaluation, int-indexed entry counts, and element-count-to-byte representability.
It covers `N + 3*E + 1`, edge arrays, `(maxLevel+2)*(levelPitch+1)`, matrix squares
and multi-level matrix slabs. Checks use the storage actually indexed: two-level
reduction has separate square arrays; multi-level reduction uses contiguous slabs.
Disabled/zero-level reduction does not impose an unused square-matrix constraint.

Existing indexing is still `int`; requests whose offsets cannot be represented
are rejected rather than cast into size_t allocations with unsafe int indexing.
No arbitrary graph-size or guessed available-memory limit was introduced.
Ordinary allocation exhaustion remains a separate `NO_MEMORY` path.

`reduce_crossings` returns its existing error status for invalid dimensions before
touching buffers. The graph-loading/layout entry catches size/allocation failures
and reports `GRAPH_TOO_LARGE`/`NO_MEMORY` through the existing error mechanism.
Dimension checks precede route-table/level/matrix allocation and result publication.

Matrix row/column barycenter sums and position-phase neighbor-coordinate sums now
use signed 64-bit accumulators. Retain the existing float conversion/division order
for matrix barycenters and integer truncation for coordinate averages. Original
in-range results are unchanged; the reproduced signed-overflow results are fixed.
This does not validate every float-to-int conversion or spacing accumulation.

## Traversal and recovery

DFS and rank propagation use explicit heap-backed frames holding the current
vertex and next successor iterator. DFS completion occurs on frame exit, and the
caller still assigns the root completion number. Rank propagation preserves the
original immediate descent and later revisits when a longer path raises a rank.
It is not replaced with a differently ordered topological traversal.

The main layout, reduction phases, level-width counting, coordinate preparation
and vertical positioning now own temporary arrays through local `unique_ptr`
owners. Partial allocations and early returns clean up automatically. Raw
`SNode::posList` remains a borrowed scratch pointer owned by a layout-local array
of owners; the public SNode layout/destructor and graph/container ownership did
not change. Removed ineffective null checks after throwing `new[]` and duplicated
manual cleanup, including partial position-list failure paths.

A local guard snapshots node/edge IDs before scratch renumbering and restores them
if preparation fails. Original edge routes are not straightened until scratch
preparation succeeds. Successful layouts retain their historical scratch-ID
behavior. Scoped window-update and busy handling restores the caller's update
flag and balances busy/unset calls on returns and exceptions. Window refresh on
successful publication remains in place.

**Recovery guarantee tested here:** failures during preparation leave IDs, node
positions and existing bent routes intact, clean scratch memory, and restore
window state. Publication still uses the existing node-move/edge-point APIs and
is not a transactional commit; see remaining risks.

## Regression tests

`tests/test_sugiyama_large_input.cpp` checks:

- Checked-arithmetic boundaries at INT_MAX, square/slab limits, negative sizes,
  and rejection through the actual reducer without allocating large buffers.
- Two-level storage acceptance independently of unused multi-level slab bounds,
  and disabled/zero-level reducer controls.
- Wide row/column barycenters and upper/lower coordinate averages.
- **200 cyclic DFS cases and 200 DAG rank cases** against frozen recursive
  helpers, comparing exact visitation/completion numbers, ranks and attributes.
- **20,000-deep** traversals with at most **512 KiB process stack**. The stack
  limit is applied inside the test after process/sanitizer startup; graph/frame
  storage is on the heap. These POSIX tests ran on Linux.
- A complete **1,024-level public layout**, including shortcut-edge dummy routing,
  object endpoints, graph counts and finite coordinates.

`tests/test_sugiyama_failure.cpp` injects a one-shot failure at each successive
C++ array allocation during layout. The injection disables itself when it throws
so cleanup and diagnostics can execute. In the tested graph, **16 failures with
reduction disabled and 28 with reduction enabled** preserve original IDs,
coordinates and bent routes; a following successful run proves recovery. The
test uses cycles, loops, parallel and long edges, plus sparse incoming IDs.

The same executable's `--gui` mode uses a genuine FFTclWindow/Xvfb instance,
verifies balanced busy calls and preservation of an already-disabled update flag
after every failure and success, then checks Tk window teardown. This is runtime
state testing, not a reopening of the parked WSLg visibility investigation.

All eight frozen complete layouts, matrix/crossing oracles, and **31 real/synthetic
graphs under six reference/optimized configurations** remain active and pass.
No geometry oracle was regenerated. The expanded corpus compares the current
pipeline with two sort implementations; the independent recursive-helper tests
and unchanged older snapshots separately cover traversal compatibility.

## Actual verification and commands

| Check | Result |
| --- | --- |
| Baseline Debug configure/build/full CTest | 77/77 passed |
| New numeric/recovery/deep regressions | 3/3 passed |
| Actual-window recovery regression | Passed |
| Final Debug configure/build/full CTest | **81/81 passed** |
| Final Release configure/build/full CTest | **83/83 passed**, including two external Rome tests |
| Final ASan/UBSan build, strict headless/leak CTest | **68/68 passed**, LSAN_OPTIONS cleared |
| Final GUI runtime ASan/UBSan, leak detection OFF | **13/13 passed** |
| `git diff --check` | Passed |

Full suites used access outside the restrictive sandbox for Xvfb and leak tracing.
Strict GUI toolkit leak diagnostics and remote GitHub Actions were not rerun.

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug -R 'sugiyama_large_input|sugiyama_deep_traversal|sugiyama_failure' -V
ctest --preset debug
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release
cmake --preset asan
cmake --build --preset asan --parallel 2
LSAN_OPTIONS= ctest --preset asan-headless
LSAN_OPTIONS= ctest --preset asan-gui
```

Implementation files: `flayout/FFsugiyama.cc`, `FFreduce_crossing.cc`,
`FFimprove_position.cc`, `FFlayout_algorithmen.cc`, `FFSugi.cc`, private bounds
header, two regression sources and CMake registrations. Build/CTest logs, baseline
controls, commands and source/binary metadata are in `build/large-input-checkpoint/`.
Ignored local artifacts supplement this report; permanent tests need only the
repository sources/fixtures.

## Remaining limits

- **Publication exceptions:** allocation failure while actually moving graphic
  nodes, inserting final edge points or running GUI callbacks can still leave a
  partially published drawing. No full geometry rollback was implemented. Fault
  injection targets `new[]`, not every scalar allocation, C allocator or callback.
  Window-state unwinding and scratch cleanup do not prove atomic publication.
- **Extreme coordinate arithmetic:** coordinate-to-int casts, spacing/degree
  accumulations and displacement calculations elsewhere still need separate
  reproduced boundary tests. Nonfinite settings/coordinates and arbitrary extreme
  icon dimensions were not validated by these bounds checks.
- **Practical resources:** checked dimensions prevent integer overflow, not huge
  but representable memory/time requests. Tests do not establish a maximum
  supported graph size; the largest traversal has 20,000 nodes while the complete
  deep integration layout has 1,024 levels. Iterative propagation keeps legacy
  revisits and can remain expensive on large DAGs.
- **Other phases/platforms:** arbitrary callback mutation, nested graphs, all
  external real graphs and non-Linux toolchains remain unverified. Existing
  allocator/traversal-order sensitivity across separately constructed graphs
  remains. No new performance claim is made for this correctness patch.

FF/GML/Rome formats, public layout signatures, Spring weight formulas and ordinary
GUI controls are unchanged. Aesthetic Sugiyama refinement and WSLg remain parked;
SPQR remains excluded and Rome loader optimization remains low priority.
