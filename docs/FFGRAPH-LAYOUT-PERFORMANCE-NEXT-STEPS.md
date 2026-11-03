# Layout performance priorities — 2026-11-03

**Implemented checkpoint:** the three exact optimization steps below are now
implemented after control fixes and explicit user decisions. See
`FFGRAPH-EXACT-LAYOUT-OPTIMIZATIONS.md` for frozen-kernel/permutation comparisons,
actual timings and full verification. The remainder of this report records the
original deferred plan and pre-optimization measurements.

## Status: deferred implementation plan

The user requested a correctness review before further optimization. Complete
the corrective checkpoints in `FFGRAPH-LAYOUT-CORRECTNESS-REVIEW.md` first.
The measurements below remain the performance baseline; the three optimization
steps are deferred, not implemented or promised speedups.

Before starting step 1, resolve the intended weighted-force and local-layout
contracts, add correctness regressions, and record a corrected reference
implementation. Keep bug fixes separate from performance patches so reviewers
can distinguish intentional geometry changes from optimization regressions.
For each later performance patch:

1. Save the reference executable, input/source hashes and per-step or matrix
   oracles before editing. Cover the review's edge cases as well as the large
   profiling inputs.
2. Compare actual forces, permutations, graph construction and complete routes
   with the corrected reference. Do not silently re-record historical oracles.
3. Run Debug/Release CTest and strict headless ASan/UBSan/LSan; exercise affected
   Tcl/Tk layout controls and GUI sanitizer runtime where the patch touches them.
4. Repeat native timings with the same inputs, settings and affinity. Report
   small-case regressions and large-case gains, then refresh the hotspot profile.
5. Document and review each patch independently. Retain existing APIs and formats;
   Rome loader optimization stays low priority, SPQR excluded, WSLg issue parked.

## Scope and current measurements

Refreshed profiles at `9b72553` on `import_and_layout_profiling`, after the
production crossing-counter optimization and default-layout ownership fix.
No further solver, parser, container, GUI or public API changes were made.
Rome remains deprecated, with loader optimization low priority. SPQR stays
excluded and the parked WSLg inspector investigation stays parked.

The existing optional profiling tools were rebuilt in `build/profile` with
RelWithDebInfo, `-O2 -g -DNDEBUG` and frame pointers. Callgrind instrumentation
covers the actual layout, excluding import and validation. Each case has an
uninstrumented warmup and one instrumented sample. Percentages below are
**instruction counts**, not percentages of native elapsed time.

| Layout and input | Largest instruction costs |
| --- | --- |
| Spring 2D, layered GML, 1,000 nodes / 1,906 edges | `compute_forces_edges`: 93.94% inclusive; 42.46% self |
| Spring 3D, same graph | `compute_forces_edges`: 94.10% inclusive; 44.21% self |
| Sugiyama, `contrib/out.g`, 499 / 499 | `b_r`: 68.61% self; `b_c`: 22.39%; crossing counter: 5.81% |
| Sugiyama, real Rome `north/g57/g.57.26`, 57 / 241 | `b_r`: 46.99% self; `b_c`: 45.47%; crossing counter: 6.23% |
| Sugiyama, complete bipartite GML, 128 / 4,096 | `rem_par_edges`: 94.48% self |

Raw profiles, annotations, stdout/stderr and binary/source hashes are in
`build/profile3-cpu/`. These generated artifacts are not committed. The Rome
case measures the shared layout, not its loader.

Separate native runs used logical CPU 0, one warmup and five fresh-graph
samples, with no concurrent instrumented run. Median complete-layout times:

| Case | Median ms |
| --- | ---: |
| Spring 2D, layered GML, 500 nodes | 66.228 |
| Spring 2D, layered GML, 1,000 nodes | 262.086 |
| Spring 2D, layered GML, 2,000 nodes | 1,049.269 |
| Spring 3D, layered GML, 1,000 nodes | 267.857 |
| Sugiyama, `contrib/out.g` | 91.177 |
| Sugiyama, real Rome case above | 147.296 |
| Sugiyama, complete bipartite GML, 128 nodes | 18.186 |

Native samples, exact commands, input hashes and executable provenance are in
`build/profile3-native/`. Spring uses quality 2; the reported iteration cap is
30 for all these sizes, not a measurement of actual iterations. The timings
are consistent with quadratic pairwise work. Host frequency/load were not
controlled. These are current baselines, not gains from a new optimization.
The pre-existing allocator-sensitive node-order limitation of historical FF
inputs still applies; see the crossing-counter optimization report.

## Recommended sequence

### 1. Spring: remove point-operation overhead from the force loop

The five out-of-line `FFPoint` operations for scalar multiplication, subtraction,
assignment, addition assignment and subtraction assignment together account
for approximately 49.6% of all instructions in the 2D case. They construct/copy
temporaries and process XYZ even in the 2D force loop. Sparse graphs still visit
essentially every unordered node pair for repulsion.

Prototype private component-wise arithmetic in `FFspringembedder.cc`, retaining
the exact force law and pair/accumulation order. Check both `compute_forces`
and `compute_forces_edges`; the fresh profiles exercise the weighted path.
Preserve float intermediate rounding, coincident-node fallback, homogeneous
coordinates, edge weights, parallel-edge behavior, pinned/local nodes and the
recent norm/temperature stability fix. Avoid global changes to `FFPoint`.

Before promotion, compare per-step forces and complete trajectories against
the current implementation in 2D/3D, with weighted/unweighted, coincident,
isolated and fixed-node fixtures. Use finite/topology checks and native
benchmarks on sparse and dense cases. A measured speedup is not yet available.

### 2. Sugiyama: batch the barycenter permutations

`b_r` and `b_c` now dominate the sparse examples. They bubble-sort barycenters,
swapping full matrix rows/columns and the corresponding neighboring matrix on
each inversion. Calculate the final stable ordering once and apply its
permutation once per matrix instead of moving the same data repeatedly.

Compatibility requirements are stricter than simply sorting the active cells:
zero-barycenter slots stay fixed; equal barycenters preserve their order;
metadata row/column zero and full-stride data move consistently; paired matrices
must receive the same node permutation. The separate equal-barycenter reversal
phases and sweep/iteration schedule must remain unchanged.

Use independent old/new matrix-permutation comparisons, including zero holes,
ties, rectangular matrices, metadata/padding and neighboring matrices. Retain
the existing frozen layout oracles and full-width crossing regressions, then
compare complete layouts on the real and generated corpus. Do not optimize the
crossing counter further before addressing this larger measured cost.

### 3. Sugiyama: index parallel-edge candidates

`rem_par_edges` scans all edges for every surviving edge, even when the graph
has no parallel edges. It runs before and after backward-edge reversal. This
quadratic preprocessing, rather than crossing reduction, dominates the dense
complete bipartite case.

A temporary endpoint-pair index can find duplicates without the full nested
scan. Preserve the earliest surviving edge, edge type 2 versus 3, append order
in `parEdges`, and the exact degree/list updates. Merely appending duplicates
as a hash scan discovers them can change the existing group order. Test genuine
parallel edges, edges made parallel by reversal, loops and dense graphs with
no duplicates. Keep the index local to preprocessing; no graph ownership or
live graph-index redesign is required.

## Larger follow-ups and limits

Barnes–Hut repulsion could reduce Spring's asymptotic cost for large graphs,
but approximates forces and changes trajectories. Treat it as a separately
validated, explicit optional mode after the exact arithmetic optimization;
test coincident-point termination and both dimensions. Do not promise identical
geometry. Broad threading or cached live-graph adjacency is a higher-risk first
step because of summation order and GUI animation/callback behavior.

No new algorithm has been implemented or benchmarked here. Expected savings
remain hypotheses. GUI drawing/event latency was not profiled. The weighted
Spring workload and three Sugiyama examples do not establish universal hotspot
percentages. Rome loader work is not part of this sequence.

## Verification and reproduction

The profiling CMake configure and both profiler target builds succeeded.
All five Callgrind cases and all seven native cases exited successfully,
including each tool's graph membership/incidence and finite-coordinate checks.
Generated inputs additionally use independent topology TSV oracles. Samples
are not a new independent oracle for exact coordinates.

The Release Spring/Sugiyama/default-layout CTest subset passed **5/5**.
No implementation changes in this profiling checkpoint require new sanitizer
coverage. The separately completed ownership
patch's Debug/Release/sanitizer results are documented in
`FFGRAPH-APPLICATION-DEFAULT-LAYOUT-OWNERSHIP.md`.

```sh
cmake -S . -B build/profile -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=OFF \
  -DFFGRAPH_BUILD_PROFILER=ON -DCMAKE_CXX_FLAGS=-fno-omit-frame-pointer
cmake --build build/profile --target profile_graphs profile_crossings --parallel 2
valgrind --tool=callgrind --instr-atstart=no \
  --callgrind-out-file=/tmp/spring-next.callgrind \
  build/profile/profile_graphs --phase layout --repeat 1 --layout spring \
  --input build/spring-numerics-profile/inputs/layered-1000.gml \
  --oracle build/spring-numerics-profile/inputs/layered-1000.tsv
callgrind_annotate --auto=no --inclusive=no /tmp/spring-next.callgrind
taskset -c 0 build/profile/profile_graphs --phase layout --repeat 5 \
  --layout sugi --input contrib/out.g
ctest --preset release -R 'sugiyama|spring|application_default'
```

The generated/native metadata retains all seven commands. Use a CPU permitted
by the local affinity mask if CPU 0 is unavailable. Prior workload-generation
instructions are in `FFGRAPH-IMPORT-LAYOUT-PROFILING.md` and the production
counter report. No commits or pushes were made.
