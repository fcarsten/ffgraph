# Sugiyama crossing scans and matrix comparisons — 4 November 2026

Started from clean `import_and_layout_profiling` at `1a20063` (ordinary allocation
recovery). The user requested optimizing repeated crossing evaluation and matrix
comparisons, with a suggested commit message. Baseline Debug configure/build/full
CTest passed **90/90**. No commits or pushes made.

## Measured cost and targeted changes

The refreshed `out.g` profile attributed about 39% of layout instructions to
crossing counting and 7.6% to matrix-array equality. Each crossing count scanned
every active row twice: once to query earlier rows, then again to update column
totals. Equality used nested scalar comparisons across the active rectangle.
Both operations repeat throughout the existing reduction sweeps.

`flayout/FFreduce_crossing.cc` now makes two small, exact changes:

1. **One crossing scan per row.** At row entry, `right` snapshots the count of
   edges in earlier rows. Each column's old count is subtracted before querying
   the current edge. That column can then be updated immediately: the scan will
   never read it again in this row, and `right` no longer includes it. Later
   queries still see only earlier-row edges. Edges sharing a source or target
   are excluded exactly as before; only cells equal to 1 represent edges.
2. **Contiguous active comparisons.** `equal_matrix` uses `std::equal` on each
   active row span. A full-width rectangle is compared as one contiguous span.
   Include row/column-zero vertex metadata; ignore inactive columns, rows and
   slab gaps. Negative row/column maxima keep the old empty-comparison result.
   Offset calculations use pointer-width arithmetic rather than an int product.

Counting remains O(rows * columns) with the same local column vector and lifetime.
The routines remain stateless between calls. No persistent count cache, extra
scratch buffer, matrix mutation, heuristic, sweep schedule, tie comparison or
best-state policy was introduced. Full-width uint64 counts and the saturating
legacy int wrappers are unchanged. No public signature, graph ownership, parser,
format or GUI flow changed.

## Compatibility coverage

New `tests/test_sugiyama_matrix_comparisons.cpp` compares against frozen nested
loops from the pre-change implementation. Its **23,005 individual-cell mutation
comparisons**, plus identical/aliased controls, cover:

- Every active rectangle within strides 1 through 9.
- Wide/tall/padded rectangles, including 513 active columns and stride 1,024.
- Mutations of every stored cell, proving which metadata/active cells matter
  and which padding cells must be ignored.
- INT_MIN/INT_MAX and other nonbinary values, with input immutability checks.
- Zero row/column maxima, negative maxima with null pointers, and empty arrays.
- Multi-level matrices with unequal/empty layer widths, node-array pitch different
  from matrix stride, inter-slab gaps and an unused extra matrix.

This test passed against the **unchanged baseline before production edits**.
`CMakeLists.txt` adds it to the existing headless suite.

The existing exhaustive **65,536 binary 4x4** crossing oracle, rectangular and
nonbinary cases, closed-form dense counts, values above INT_MAX and cross-layer
sum/saturation checks all pass. The permutation test still checks complete
matrices against the frozen stable-sort reference. All eight frozen complete
layouts and the 31 real/synthetic graph corpus under six configurations remain
unchanged, with actual graph geometry/routes and independent crossing metrics.

All ordinary/array allocation failure, renderer callback, OS resource-limit and
renderer-coordinate recovery tests remain active. No expected drawing or oracle
was regenerated, and no failing test was removed or weakened.

## Controlled native measurements

Built RelWithDebInfo controls with the same flags and frame pointers. Four
variants isolate the old implementation, crossing-only change, comparison-only
change and both changes together. Each uses a copy of the **same layout archive**,
replacing only its reduction object in the same member position; all four links
use the same archive/library order and profiler object.

The profiler-only control omits the normal warmup and performs one first layout
per process. This avoids the known allocator-history sensitivity of warmed FF
drawings. No application profiler source or default behavior was modified.

For each input, 24 rounds exercise all 24 permutations of the four variant orders,
in a fixed shuffled sequence. Each variant therefore appears equally often in
each position. Processes are pinned to one permitted CPU. Timing includes layout
only, excluding import, graph validation and snapshot serialization. No build,
CTest or Valgrind run overlapped the final native measurements.

**All 96 complete snapshots per workload match byte-for-byte across variants and
rounds: 480 measured runs total.** Object incidence, graph counts/directedness,
node coordinates and complete edge routes are checked.

| Workload | Nodes / edges | Before median ms | Combined median ms | Time reduction |
| --- | ---: | ---: | ---: | ---: |
| `contrib/out.g` | 499 / 499 | 15.133 | 13.022 | **14.0%** |
| Corpus `b4-alledges.g` | 64 / 126 | 15.602 | 13.340 | **14.5%** |
| Corpus `dataflow.g` | 158 / 234 | 2.133 | 1.903 | **10.8%** |
| Corpus `big-tree.g` | 180 / 179 | 0.771 | 0.687 | **10.9%** |
| Complete bipartite GML | 128 / 4,096 | 4.856 | 4.870 | **-0.3%** |

Attribution controls (median ms):

| Workload | Crossing-only | Comparison-only | Combined |
| --- | ---: | ---: | ---: |
| `out.g` | 13.370 | 14.915 | 13.022 |
| `b4-alledges.g` | 13.223 | 15.758 | 13.340 |
| `dataflow.g` | 1.892 | 2.130 | 1.903 |
| `big-tree.g` | 0.691 | 0.748 | 0.687 |
| Dense GML | 4.979 | 4.817 | 4.870 |

The crossing scan supplies most of the measured sparse improvement. Comparison
wall-time effects are smaller and workload dependent; their effects are not
additive. The dense example remains dominated by other work and measured 0.3%
slower overall. These local short samples do not establish a universal speedup
or statistically guaranteed equivalence. Host frequency/load and address-space
layout are not fully controlled. No whole-process peak-memory saving is claimed.

Earlier exploratory controls linked a replacement reduction object ahead of the
normal archives. They produced different short-run timing estimates, including
a larger apparent dense slowdown. Those sources/binaries/raw samples are retained
under the checkpoint's `prefix-controls/`, `native/` and `native-balanced/`
directories, but **are excluded from these performance conclusions** because
object placement was not matched. The final archive-matched data is in
`native-archives/`.

## Instruction profiles

Four additional production-equivalent first-layout Callgrind runs instrument only
layout, separately from native timings, using the archive-matched controls:

| Workload | Before instructions | After instructions | Reduction |
| --- | ---: | ---: | ---: |
| `out.g` | 199,503,926 | 157,907,474 | **20.9%** |
| Dense GML | 41,549,090 | 41,488,156 | **0.15%** |

On `out.g`, crossing-kernel self instructions fall from 78,315,506 to 49,983,922.
Matrix-array equality inclusive instructions fall from 15,094,216 to 1,829,348,
including the called comparison routines. The tested standard library uses its
optimized contiguous comparison implementation. Self and inclusive costs are
different measures and must not be summed. Instrumented elapsed time is not used
as a native performance result.

Remaining sparse costs include crossing evaluation and barycenter/permutation
work. Matrix equality is now about 1.16% inclusive in this profile. Dense
allocation/publication costs remain a separate candidate; this patch retains all
existing geometry rollback and route-storage guarantees.

## Actual builds and tests

| Check | Result |
| --- | --- |
| Baseline Debug configure/build/full CTest | **90/90 passed** |
| New matrix comparisons on unchanged baseline | Passed |
| Targeted crossing/comparison/permutation/layout/corpus tests | **7/7 passed** |
| Final Debug configure/build/full CTest | **91/91 passed** |
| Final Release configure/build/full CTest | **93/93 passed**, including two cached external Rome tests |
| Final strict headless ASan/UBSan/float-cast/leak suite | **73/73 passed**, LSAN_OPTIONS cleared |
| Final GUI runtime sanitizer suite, leaks OFF | **16/16 passed** |
| Archive-matched native runs and snapshots | **480/480 passed** |
| Archive-matched first-layout instruction profiles | **4/4 passed** |
| Final strengthened comparison test: Debug, Release, strict ASan and saved baseline | **4/4 passed** |

The full suites above exercised the final production implementation. Afterward,
the new test was strengthened to calculate its oracle before each production
call and verify both complete inputs immediately afterward. That test was rerun
in all three presets and against the saved pre-change reduction object; all pass.

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug -R '^test_sugiyama_(matrix_comparisons|crossings|corpus_)' -V
ctest --preset debug
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release
cmake --preset asan
cmake --build --preset asan --parallel 2
LSAN_OPTIONS= ctest --preset asan-headless
LSAN_OPTIONS= ctest --preset asan-gui
```

Xvfb and leak tracing used access outside the restrictive sandbox. Logs, original
source/object, four control archives/binaries, exact build/measurement commands,
raw timing data, snapshots, Callgrind annotations and source/binary hashes are
retained locally under `build/sugiyama-count-comparison-checkpoint/`. Permanent
tests require only repository sources/fixtures.

Strict GUI toolkit leak diagnostics, remote CI, interactive drawing assessment,
other platforms/toolchains and every external graph were not run. The historical
allocator/traversal-order sensitivity is not resolved by this patch; the native
table uses matching first layouts. Persistent resource exhaustion and arbitrary
callback mutation/reentrancy retain the earlier documented limits. SPQR remains
excluded; aesthetic Sugiyama and WSLg work stay parked; Rome loader optimization
remains low priority.

## Suggested commit message

```text
Optimize Sugiyama crossing scans and active matrix comparisons

- Count crossings in one pass per row with unchanged strict-edge semantics
- Compare contiguous active matrix spans while preserving metadata and padding rules
- Add differential matrix-comparison coverage and retain layout/recovery oracles
```
