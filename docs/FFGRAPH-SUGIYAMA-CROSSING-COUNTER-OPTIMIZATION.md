# Production Sugiyama crossing-counter optimization — 2026-11-02

## Scope and baseline

The user authorized promoting the measured crossing-counter optimization and
marking the Rome graph format deprecated, with loader optimization low priority.
Work started from clean `ae45633` on `import_and_layout_profiling`. Debug CMake/
build/CTest baseline: **60/60 passed**. No commits, tags or pushes were made.
SPQR remains excluded and the parked WSLg inspector issue remains untouched.

The optimized counter is used by normal application builds without an opt-in
flag. Profiling tools remain optional. No parser, graph/container ownership,
Tcl/Tk event flow, file format or public layout-setting API was changed.

## Problem and implementation

The old `number_of_crossings` enumerated row pairs and column pairs, with
`O(rows² × columns²)` worst-case work. Prior profiles identified repeated scans
of sparse dummy-expanded matrices as the principal cost in historical graphs.

`flayout/FFreduce_crossing.cc` now uses the validated column-count approach:

1. For each row, count previously seen edges whose columns lie strictly to
   the right of each current edge.
2. Add the entire current row to column totals only after querying it. Edges
   sharing a source or target therefore do not count as crossings.
3. Preserve the existing interpretation: only matrix cells equal to **1** are
   edges; zero-based padding/dummy rows and columns are ignored.

This requires `O(rows × columns)` work and `O(columns)` temporary storage.
Zero/one-row or column cases return zero without allocation or matrix access.
The local vector owns its temporary storage; no global cache or graph index
was introduced. The barycentric sorting, tie comparisons, iteration controls,
matrix permutations and default algorithm selection remain unchanged.

`flayout/FFcrossing_count.h` is a private, non-installed implementation interface.
The kernel, multi-level sum and both two-/multi-level best-count comparisons
use `uint64_t`. This avoids signed overflow changing which arrangement is kept.
The existing global `int` helper signatures remain available, returning the
exact value when representable and **INT_MAX** otherwise. Application decisions
use the full-width functions, never those saturated compatibility wrappers.

Above the old `int` range, behavior is now defined instead of signed overflow;
matching undefined historical overflow results is not a compatibility goal.
The broader legacy integer limits for allocating/indexing huge layout arrays
were not redesigned.

## Permanent regression coverage

`tests/test_sugiyama_crossings.cpp`, registered in ordinary CTest, checks:

- All **65,536 binary 4×4 matrices** against an independent edge-pair oracle.
- **128 rectangular/nonbinary cases**, including empty dimensions, ignoring
  cells other than 1, with sentinel ones in padding and checks for no mutation.
- Complete matrices through 256×256 against closed-form crossing counts.
- A 512×512 complete matrix with **17,112,825,856** crossings, testing exact
  full-width output and the `int` wrapper's saturation.
- Three 256×256 layer matrices: individual counts fit `int`, while their sum
  **3,196,108,800** does not. The exact sum and saturated wrapper are checked.
- Degenerate null-pointer cases that must return zero without touching memory.

`tests/test_sugiyama_layout.cpp` constructs four graph fixtures with explicit
contiguous IDs, labelled nodes/edges and nonuniform weights: long edges/dummies,
complete bipartite layers, cycles/loops/parallel edges/an isolated node, and a
star. It checks object membership and edge incidence, finite geometry, graph
metadata, labels/weights and every edge route against **eight pre-optimization
oracles**, covering dimension parameters 2 and 3. It does not claim a new 3D
layout capability; the old implementation produces identical outputs for these
settings on these fixtures.

The oracles were recorded **before replacing the counter**, using the exact
`FFreduce_crossing.cc` from `ae45633`. They were checked a second time against
that implementation. Source/file hashes and provenance are in
`tests/sugiyama_oracles/README.md`. The new helper
`tests/sugiyama_layout_snapshot.h` captures geometry by imported node identity,
excluding documented temporary numeric layout IDs. Permanent comparisons allow
a small float-rounding tolerance; they remain active under `NDEBUG`.

The CMake change registers the two tests alongside the existing headless graph
tests. Existing tests and timeouts were retained.

## Complete-layout compatibility and the geometry limitation

`profile_graphs` gained optional `--dimension 2|3` and `--snapshot FILE` support.
Snapshots are outside the clock and refuse to overwrite existing files. Their
node identities, labels, XYZ/W, edge endpoints/names/weights and all route
points can be compared across executables. Unique imported IDs are required
for this diagnostic only; production loading behavior is unchanged.

The old executable was archived before replacing its static library. Fourteen
public-loader/layout cases cover complete bipartite graphs at 16/32/64/128
nodes in FF/GML/Rome plus `contrib/out.g` and the real Rome graph
`north/g57/g.57.26`. Two old-executable captures per dimension first checked
baseline reproducibility; optimized captures then checked both dimensions.

**26/28 snapshots matched byte-for-byte.** The two raw `contrib/out.g` snapshots
had different coordinates. The first strict all-exact comparison consequently
exited 1; the differences were investigated and remain recorded, not hidden.

This file has sparse/out-of-range numeric IDs and unset edge IDs.
`init_sgraph` assigns unused numeric slots according to live container order.
Different temporary allocations affect later fresh-graph allocations, so the
historical "deterministic" setting does not guarantee pixel identity here:

- gdb compared the actual original-node identities in SNode slots before
  `reduce_crossings`. Warmup assignments/initial positions matched. On the next
  fresh graph, **398 of 499 node-slot assignments differed before the counter
  ran**, and the initial position array already differed.
- The unmodified old executable also produced different coordinates when run
  with only `GLIBC_TUNABLES=glibc.malloc.tcache_count=0` changed.
- To exclude a counting difference, an optional diagnostic build using
  `FFGRAPH_VERIFY_SUGIYAMA_COUNTS` compared each nondegenerate matrix result
  with the former pair-enumeration formula evaluated in 64 bits. All **28
  layout cases** (14 inputs × two dimensions, warmup plus one run each) passed.
  Degenerate fast-zero cases are independently covered by the permanent tests.

This checker is compiled out of normal builds. Existing pointer/container-order
dependent renumbering was not changed to force matching coordinates. All cases
preserved graph membership/incidence, attributes and finite valid routes. Exact
pixel reproduction for historical noncanonical IDs is therefore **not claimed**.
The eight controlled frozen fixtures passed in Debug, Release and sanitizers.

The matrix-capture script now selects the internal optimized kernel, falling
back to the former helper for archived executables. Optimized fast returns may
have unavailable debug arguments; Debug builds are recommended. An optimized
capture initially reported that limitation explicitly; a Debug capture then
successfully recorded **5,208** calls across two real Rome layouts.

## Measured performance

The same optimized `RelWithDebInfo` configuration (`-O2 -g -DNDEBUG`, frame
pointers), same input hashes, logical CPU 0, one warmup and five fresh-graph
samples per case. Normal timing runs had no concurrent ffgraph tests/builds or
instrumented runs. Host load/frequency were not controlled. The clock measures
the complete headless `FFSugi::layout()`, including crossing reduction and route
construction, excluding file import, snapshot checking and GUI drawing.

Both before/after series passed **14/14** graph cases. Median milliseconds:

| Input | Before | After | Before / after |
| --- | ---: | ---: | ---: |
| Generated GML bipartite, 16 nodes / 64 edges | 0.046965 | 0.046126 | 1.02× |
| Generated GML bipartite, 32 nodes / 256 edges | 0.221804 | 0.271978 | 0.82× |
| Generated GML bipartite, 64 nodes / 1,024 edges | 1.840169 | 1.700726 | 1.08× |
| Generated GML bipartite, 128 nodes / 4,096 edges | 22.125208 | 19.058345 | 1.16× |
| Bundled FF `contrib/out.g`, 499 nodes / 499 edges | 483.557947 | 94.499995 | 5.12× |
| Real Rome, 57 nodes / 241 edges | 452.202250 | 151.663971 | 2.98× |

Small cases do not show a universal gain; the 32-node example was about 0.05 ms
slower in this series. The FF result is the observed same-file warm-run gain,
with the initial-order variability explained above, so it is not a strictly
identical internal-workload comparison. The Rome layout snapshots matched
exactly; its approximately 3× result is the clearer same-layout comparison.
These are shared layout improvements, **not Rome-loader optimization** or GUI
latency measurements. The counting stage is only part of a complete layout.

The before/after manifests, raw samples and executable hashes are retained.
An initial optimized series and the final series agree in the large-case trend;
the table uses `crossopt-after-final` only. Snapshot, sanitizer, gdb and
reference-check timings are not used as performance results.

## Verification actually run

- Initial Debug baseline **60/60** passed. New small count tests and the eight
  frozen layouts passed against the old counter before optimization.
- Final CMake/build/CTest: **Debug 62/62**, **Release 64/64**, including existing
  GUI tests and both configured external Rome corpus tests in Release.
- Strict **ASan/UBSan/LSan headless 52/52** passed. GUI sanitizer runtime
  **10/10** passed with the existing leak-disabled GUI preset. Strict GUI
  fontconfig shutdown-leak diagnostics were not repeated.
- Strict sanitizer public-load/Sugiyama series: **14/14 in dimension 2** and
  **14/14 in dimension 3**, warmup plus one measured execution each. These are
  correctness checks, not performance numbers.
- Optional former-formula reference-check series: **14/14 + 14/14**, without
  mismatches, and the eight ordinary frozen layout regressions passed.
- The 28 larger snapshot comparisons have the explicit **26 exact / 2 existing
  allocator-sensitive FF geometry differences** described above.
- Input hashes matched between performance series; final measured executable
  hash, oracle hashes and Git whitespace checks were verified. No tests were
  removed, disabled or suppressed.

Allocation-failure injection, giant real graph layouts with totals above
INT_MAX, other toolchains, interactive Sugi controls/GUI drawing latency and
bitwise pixel reproducibility across arbitrary allocator states remain
unverified. The kernel's buffer is automatic storage via `std::vector`; this
patch does not retrofit transactional cleanup for pre-existing allocation
failures throughout the layout implementation.

## Rome deprecation and next work

Per the user's direction, **Rome format support is deprecated and optimizing
its loader is low priority**. The next-session plan, BUILD-CMAKE.md and Rome
test README state this. Existing loading compatibility and all regression/
external-corpus checks remain supported. Prefer FF/GML for new format work.
No Rome loader/parser code or runtime deprecation warning was introduced.

Future import optimization should prioritize FF and preserve duplicate-ID,
merge and callback semantics. The Sugi selector discrepancy, Spring iteration
control and installation/portability work remain separate checkpoints.

## Build and reproduction

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug
ctest --test-dir build/debug -R '^test_sugiyama_' --output-on-failure
cmake --preset release -DFFGRAPH_ROME_CORPUS_DIR=/home/carsten/work/demographs/rome
cmake --build --preset release --parallel 2
ctest --preset release
cmake --preset asan -DFFGRAPH_BUILD_PROFILER=ON
cmake --build --preset asan --parallel 2
ctest --preset asan-headless
ctest --preset asan-gui

cmake -S . -B build/profile -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DBUILD_TESTING=OFF -DFFGRAPH_BUILD_PROFILER=ON \
  -DCMAKE_CXX_FLAGS=-fno-omit-frame-pointer
cmake --build build/profile --target profile_graphs profile_crossings --parallel 2
taskset -c 0 python3 tools/profile_graphs.py --binary build/profile/profile_graphs \
  --output build/crossopt-after-local --sizes 16 32 64 128 --shapes bipartite \
  --phases layout --layouts sugi --samples 5 --real-ff contrib/out.g \
  --real-rome /home/carsten/work/demographs/rome/north/g57/g.57.26

# Optional full-layout checking of the former enumeration formula. Deliberately
# slow: keep separate from timing and do not run giant count tests in this build.
cmake -S . -B build/crossopt-verify -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DBUILD_TESTING=OFF -DFFGRAPH_BUILD_PROFILER=ON \
  -DCMAKE_CXX_FLAGS=-DFFGRAPH_VERIFY_SUGIYAMA_COUNTS
cmake --build build/crossopt-verify --target profile_graphs --parallel 2
python3 tools/profile_graphs.py --binary build/crossopt-verify/profile_graphs \
  --output build/crossopt-verify-local --sizes 16 32 64 128 --shapes bipartite \
  --phases layout --layouts sugi --samples 1 --real-ff contrib/out.g \
  --real-rome /home/carsten/work/demographs/rome/north/g57/g.57.26
# Repeat into a new directory with --dimension 3 for the second setting.

# Optional snapshot; the destination must not already exist.
build/profile/profile_graphs --phase layout --layout sugi --dimension 2 --repeat 1 \
  --input contrib/out.g --snapshot build/out-layout-local.txt
```

Local artifacts: `build/crossopt-baseline/` retains the original executable and
configuration; `build/crossopt-before/`, `build/crossopt-after-final/` retain timing
data. `build/crossopt-layout-{before,after}/` retains snapshot comparisons,
including the two differences. The first optimized timing executable is also
retained under `build/crossopt-after/`. Strict runtime cases are under
`build/crossopt-asan-layouts-{2,3}/`, reference checks under
`build/crossopt-verified-counts-{2,3}/`, and the successful diagnostic capture
under `build/crossopt-capture-debug/`. Logs and pre-counter assignment/allocator
controls are `/tmp/ff-crossopt-*`. External source datasets were not modified.
Build and diagnostic artifacts remain ignored; only generated fixture oracles
are proposed as data.
