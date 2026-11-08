# Sugiyama barycenter correctness — 2026-11-03

## Scope and baseline

The user requested the next correctness checkpoint after the termination and
equilibrium fixes. This patch addresses findings 3 and 4 and the two-level
stopping check in `FFGRAPH-LAYOUT-CORRECTNESS-REVIEW.md`. The optimization plan
remains deferred; this is a correctness patch, not the proposed barycenter-sort
performance rewrite.

Started from clean `8069c51` on `import_and_layout_profiling`. Initial current
Debug CMake/build/CTest passed **66/66**. The pre-change reduction source, Debug
layout executable, layout static library and CMake cache were saved in
`build/barycenter-reference/` before editing. No commits or pushes were made.

Published graph/layout APIs, formats, Tcl/Tk flows, graph/container ownership,
Spring behavior and layout settings remain unchanged. Rome stays deprecated,
loader optimization low priority, SPQR excluded and WSLg investigation parked.

## Defects and changes

Only `flayout/FFreduce_crossing.cc` changes production behavior:

1. **Wrong row limit during column tie reversal.** Private helper `r_c` used the
   active column count as the number of rows to scan for each column's
   barycenter. It now receives the actual row count. All three callers pass it:
   intermediate and terminal matrices in `phase2_down`, and the two-level reducer.
   The helper is not declared in installed public headers; public layout entry
   points and settings retain their signatures.

   For the 4×2 matrix with column endpoints `{1,3}` and `{1,4}`, correct
   barycenters are 2 and 2.5. The old helper saw only the first two rows,
   incorrectly declared a tie and reversed their identities. The new code
   distinguishes the columns. Correctly equal barycenters still reverse, moving
   the matching rows in the next matrix and all corresponding metadata/padding.

2. **Sortedness across zero slots.** `sorted` only compared adjacent physical
   entries, accepting `[3,0,2]`. It now remembers the previous nonzero entry,
   checking the complete nonzero subsequence. Zero slots remain ignored, as its
   contract requires. Phase-one sorting and its fixed-zero-slot convention were
   not changed. Phase-two equal-barycenter reversal, including zero ties, remains
   in place.

3. **Premature two-level termination.** After filling row barycenters, `bc_2`
   accepted either `sorted(b, rows)` or `sorted(b, columns)`. On a rectangular
   matrix the second check could accept a proper prefix, or read stale column
   barycenters beyond the populated row sequence. It now checks the full row
   sequence only. Other iteration/sweep rules and best-count retention are
   unchanged.

`tools/review_layout_correctness.cpp` updates its `r_c` declaration/call to the
new private signature. That historical manual diagnostic still expects old
failures; it is not the normal correctness regression. Current validation uses
the new CTest described below. No legacy global layout selector was changed.

## Concrete stopping-check counterexample

The seven-node directed graph has two levels `{1,2,3,4}` and `{5,6,7}`, with
edges **1→6, 2→5, 2→7, 3→7, 4→5**. Explicit contiguous IDs remove the historical
allocator-sensitive renumbering ambiguity.

The original reducer retains one crossing. Corrected reduction reaches zero.
One independently verifiable zero-crossing ordering is sources **4,2,3,1** and
targets **5,7,6**. The permanent test requires zero crossings, not that exact
permutation or a newly recorded coordinate snapshot.

To isolate the stopping condition, a temporary diagnostic kept the old OR in
`bc_2` while using the corrected sortedness and rectangular helpers. It still
retained **1 crossing**, whereas the corrected full-row check gave **0**.
Thus fixing the other two bugs does not make this change redundant. The search
found this 4×3 matrix at binary mask 810; its output and controlled source are
retained locally, not linked into the application or permanent tests.

The complete public `FFSugi::layout()` regression also failed with an avoidable
crossing against the archived original library. With the corrections it has
zero crossings in both orientations, distinct same-level node positions,
separate levels and straight finite edge routes. Node/edge identity, incidence,
directedness and edge attributes remain checked. This demonstrates an intentional
geometry correction, not merely a helper result or parsing success.

## Permanent independent regressions

`tests/test_sugiyama_barycenters.cpp` is registered in the existing headless
graph-test CMake group with its ordinary 30-second timeout. Checks execute in
Release as well as Debug and do not depend on private workspace access.

- **21,845 sortedness cases:** every length-0..7 sequence over `{0,1,2,3}`. An
  independent oracle removes zeros and uses standard sortedness checking. Input
  buffers, one-based boundaries and sentinel values must remain unchanged.
- **2,052 column-phase cases:** exhaustive binary 4×2, 2×4 and 3×3 rectangles,
  plus zero-row/column cases, each both terminal and paired with a neighboring
  matrix. The oracle uses exact integer fractions for barycenter comparisons
  and constructs the expected permutation. It checks every cell, including
  metadata and padding, both matrices' consistent identities and early return
  for genuinely unsorted barycenters. Sentinel ones outside the active rectangle
  ensure the helper cannot obtain a passing result by scanning inactive rows.
- **The seven-node stopping fixture:** independent strict endpoint-order
  inversion counts test the actual reduction and full public layouts. Shared
  source/target endpoints are excluded by strict comparisons. This is not an
  assertion that all graphs must reach their global crossing minimum.
- **6,425 two-level graphs:** all binary matrices in 4×3, 3×4, 3×3, 4×2 and
  2×4 with at least one incoming edge per target. Targets with no incoming edge
  would belong in level zero, so are excluded from this two-level domain.
  Isolated sources are retained. Every reduction must terminate, preserve each
  level's identities/counts and return no more crossings than its input, measured
  independently from edge pairs and output order. No timing threshold asserts
  a performance improvement; CTest's ordinary timeout catches nontermination.

Separate test modes `sorted`, `columns`, `stopping-kernel` and `stopping-layout`
were compiled against the archived pre-change layout library. **All four exited
1 with their expected correctness failure**; results are in
`build/barycenter-reference/failing-tests.json`. Sortedness/column tests were
also run and failed directly before the first source edit. The old-OR controlled
comparison provides additional isolation for the stopping fix.

## Compatibility and actual verification

All **eight existing frozen Sugiyama layout oracles pass unchanged**. No expected
geometry was replaced and no test was disabled, relaxed or removed. The new
seven-node case deliberately changes geometry/crossing count to fix a demonstrated
error. Exact coordinates for every other historical graph are not promised;
the existing sparse-ID allocator/order limitation also still applies.

| Verification | Result |
| --- | --- |
| Initial Debug CMake/build/CTest | **66/66 passed** |
| Four regression modes against archived original layout library | **All failed as expected**, preserved diagnostics |
| Controlled old-OR variant with other two corrections present | **1 crossing**, corrected full-row condition **0** |
| Final Debug CMake/build/CTest | **67/67 passed**, including ten GUI tests |
| Final Release CMake/build/CTest | **69/69 passed**, including ten GUI and two configured external Rome tests |
| Strict headless ASan/UBSan/LSan | **57/57 passed**, `LSAN_OPTIONS` cleared |
| Existing GUI sanitizer runtime preset | **10/10 passed**, leak detection disabled by the preset |
| Existing crossing-count and eight frozen-layout regressions | Passed in all three configurations |
| Strict sanitizer public-load/Sugiyama corpus | **14/14 in dimension 2 + 14/14 in dimension 3 passed**, warmup plus one execution each |
| Whitespace checks | Passed |

All configurations were configured and built through CMake. The final suites
include the exhaustive two-level sweep and public non-overlap checks. GUI
sanitizer runtime was run after the production fixes; the subsequent test-only
coverage additions were checked in the final full Debug/Release and strict
headless sanitizer runs. Strict GUI shutdown-leak diagnostics and interactive
layout selection were not repeated. No dependency suppression was introduced.

The larger sanitizer corpus uses the retained complete bipartite fixtures at
16/32/64/128 nodes in FF/GML/Rome, historical FF `contrib/out.g`, and the cached
real Rome graph `north/g57/g.57.26`: fourteen inputs per dimension parameter.
Each public-load case runs a warmup and one measured execution, checks actual
graph membership/incidence and finite geometry, and uses independent topology
TSVs for generated inputs. Dimension 3 here checks existing behavior; it does
not establish a new 3D Sugiyama algorithm. Sanitizer timings are not performance
results or evidence of layout optimality. No before/after whole-corpus pixel
comparison was performed.

## Remaining risks and next work

The corrected row limit and stopping test alter the heuristic's search and may
produce other legitimate coordinate differences. The permanent fixtures and
corpus checks do not establish universal convergence, aesthetic quality or a
global minimum. Large integer barycenter sums and matrix-size limits, recursive
DFS depth, exception cleanup and graph mutation from callbacks are unchanged.

Spring's weight-disable bug, weighted 2D/3D force meaning, local-selection
semantics, configuration controls and offsets remain separate review findings.
The saved performance plan should use corrected reference behavior after those
contracts are resolved. No Spring force, parser or container changes here.

## Commands and artifacts

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug
build/debug/test_sugiyama_barycenters
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release
cmake --preset asan
cmake --build --preset asan --parallel 2
LSAN_OPTIONS= ctest --preset asan-headless
LSAN_OPTIONS= ctest --preset asan-gui

# Existing opt-in profiler: correctness/finite-geometry check via public loading.
LSAN_OPTIONS= ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
build/asan/profile_graphs --phase layout --layout sugi --repeat 1 \
  --dimension 2 --input contrib/out.g
git diff --check
```

The profiler was already enabled in the local Debug and sanitizer caches. For
a fresh diagnostic build add `-DFFGRAPH_BUILD_PROFILER=ON`; it remains opt-in.
Raw larger-corpus commands, exit codes, source/binary/input hashes and stdout/
stderr are in `build/barycenter-asan-corpus/`. Baseline snapshots were not moved
or overwritten. CMake/build/CTest logs are `/tmp/ff-barycenter-*.log`.

The temporary isolated old-OR driver is `build/debug/find_barycenter_stopping`,
with sources `/tmp/ff-barycenter-{old-stop,find-stop}.cpp` and search output
`build/barycenter-reference/stopping-search.txt`. It is solely a diagnostic;
the new permanent regression uses a fixed edge-list fixture and independent
crossing oracle instead of cloning the production reducer.
