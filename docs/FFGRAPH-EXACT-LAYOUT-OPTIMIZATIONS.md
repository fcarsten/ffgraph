# Behavior-preserving layout performance checkpoint — 3 November 2026

This implements the three measured steps in `FFGRAPH-LAYOUT-PERFORMANCE-NEXT-STEPS.md`
after the control fixes and user decisions recorded in
`FFGRAPH-LAYOUT-CONTROLS-CHECKPOINT.md`. No aesthetic postpass or new layout class
is present. Branch/HEAD remain `import_and_layout_profiling` / `c1fc4bc`; all
changes are uncommitted.

## Implementation and compatibility evidence

1. **Spring force arithmetic** (`flayout/FFspringembedder.cc`): private inline
   XYZ difference/scale/divide/accumulation helpers replace repeated out-of-line
   FFPoint temporary operations. Keep float intermediates, operation order,
   homogeneous-coordinate rules, coincidence fallback, neighbor traversal,
   normalization and both legacy weighted formulas. Global FFPoint operators
   and graph ownership were not changed.

   `tests/spring_force_reference.inc` freezes the original kernels;
   `test_spring_force_compat` compares all four force components bit for bit at
   **384 actual-layout frames across 128 graph cases**. Cover both dimensions,
   weighted/unweighted, directed/undirected, coincident, loop, parallel, reversed
   and zero-weight edges. Independent analytic force-law tests accompany this
   compatibility comparison. An initial fixture reconstructed neighbors through
   copied edge sets and changed accumulation order, failing against the original
   kernel too. It now follows the original adjacency insertion order exactly.

2. **Sugiyama barycenter sorting** (`flayout/FFreduce_crossing.cc`): compute one
   stable ordering of nonzero-barycenter slots and apply its full-stride row/column
   permutation once. Zero slots stay fixed; ties retain order. Move metadata and
   padding, and apply the identical vertex permutation to the neighboring matrix.
   Unchanged orders avoid copying. Reversal phases and sweep scheduling are
   unchanged. `test_sugiyama_permutations` checks **55,308** rectangular/zero-hole/
   tie cases against frozen bubble operations, including no partner, a separate
   partner and aliased matrices. The earlier mathematical barycenter/crossing
   tests and all eight frozen complete-layout oracles remain unchanged and pass.

3. **Parallel-edge preprocessing** (`flayout/FFsugiyama.cc`): build temporary
   endpoint-pair candidate lists in ascending edge-index order. Retain the outer
   edge traversal, earliest representative, type-2/type-3 distinction, append order,
   adjacency deletions and degree updates. Rebuild the local index on the second
   call after reversal. `test_sugiyama_parallel_index` compares types, endpoints,
   ordered predecessor/successor lists, counts and full parallel lists against the
   old scan for **200 multigraphs**, before and after backward-edge reversal.

Frozen reference snippets are test-only and retain the original author credits.
They provide compatibility evidence in addition to independent invariants and
public-layout oracles; copied code alone is not an algorithm-correctness proof.
New temporary vectors/maps own scratch integers, not graph nodes or edges.

## Native measurements actually run

RelWithDebInfo, frame pointers, the same permitted CPU (0), quality 2 for Spring,
one warmup and five measured fresh-graph runs per case. No build or instrumented
profile ran concurrently with these timing loops. Reference executable was saved
after control fixes and before the performance changes. These are medians of the
actual layout, excluding import/validation. Host frequency and unrelated host
load were not controlled.

| Case | Reference ms | Optimized ms | Ratio |
| --- | ---: | ---: | ---: |
| Spring 2D, layered GML, 100 nodes | 8.481 | 1.722 | 4.9x |
| Spring 2D, layered GML, 1,000 nodes | 260.112 | 41.391 | 6.3x |
| Spring 3D, layered GML, 100 nodes | 9.513 | 1.720 | 5.5x |
| Spring 3D, layered GML, 1,000 nodes | 265.658 | 42.785 | 6.2x |
| Sugiyama, contrib/out.g, 499 nodes/499 edges | 93.347 | 19.365 | 4.8x |
| Sugiyama, complete bipartite GML, 128 nodes/4,096 edges | 19.354 | 4.568 | 4.2x |

Each profiler run passed object-incidence/directedness and finite-coordinate
checks. These few measurements do not establish universal speedups. Legacy
noncanonical ID normalization can vary between fresh loads; exact operation
compatibility is tested on the same constructed inputs, rather than inferred
from unrelated process screenshots. No new approximation or threading is used.

## Validation, commands and limitations

Final Debug **75/75**, Release **77/77**, strict headless ASan/UBSan/LSan
**63/63**, GUI runtime sanitizers **12/12** passed. Force compatibility executes
in optimized Release as well as Debug and sanitizers. No tests/oracles were
silently regenerated or disabled. There are no timing assertions in CTest.

```sh
cmake -S . -B build/profile -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DBUILD_TESTING=OFF -DFFGRAPH_BUILD_PROFILER=ON \
  -DCMAKE_CXX_FLAGS=-fno-omit-frame-pointer
cmake --build build/profile --target profile_graphs --parallel 2
taskset -c 0 build/profile/profile_graphs --phase layout --layout spring \
  --dimension 2 --quality 2 --repeat 5 \
  --input build/spring-numerics-profile/inputs/layered-1000.gml
taskset -c 0 build/profile/profile_graphs --phase layout --layout sugi \
  --repeat 5 --input contrib/out.g
ctest --preset debug -R 'spring|sugiyama'
ctest --preset release -R 'spring|sugiyama'
LSAN_OPTIONS= ctest --preset asan-headless
```

Use a CPU permitted by local affinity. Raw commands, samples and input/binary
hashes are in `build/spring-force-inline-benchmark/results.json` and
`build/sugiyama-exact-optimization-benchmark/results.json`. Corrected reference
sources/binary remain in `build/layout-controls-reference/`; combined validation
provenance is in `build/remaining-steps-checkpoint/`.

## Refreshed instruction profiles

Five post-change Callgrind runs completed successfully, with instrumentation
restricted to layout. Commands, input/binary hashes, raw profiles and self/inclusive
annotations are in `build/layout-hotspots-after-exact/`. These runs establish
remaining hotspots; their instrumented elapsed times are not native benchmarks.

| Case | Remaining instruction concentration |
| --- | --- |
| Spring 2D / 3D, 1,000 nodes | Weighted force kernel: 86.59% / 87.17% self, 90.14% / 90.46% inclusive |
| Sugiyama, contrib/out.g | Barycenter permutation: 54.84% inclusive; crossing counter: 26.41% self |
| Sugiyama, existing Rome example | Barycenter permutation: 86.98% inclusive |
| Sugiyama, dense bipartite example | Parallel preprocessing: 25.65% inclusive; allocation/free routines dominate self costs |

Inclusive percentages overlap child routines and must not be added together.
Full-stride matrix copying remains an exact-optimization candidate on sparse
examples. Dense preprocessing no longer monopolizes instructions. Spring's
quadratic pair interactions remain dominant after removing temporary point
overhead; approximation would require a separately authorized optional mode.
Rome supplies a shared-layout workload here; its deprecated loader remains low
priority and was not optimized.

Other hardware,
fast-math options, very large matrix dimensions, adversarial floating-point
inputs and allocation/callback exception cleanup remain unverified. Batched
permutations need extra temporary storage; original recursion/overflow limits
are not resolved by these speedups. The aesthetic Sugiyama work, SPQR and WSLg
investigation remain parked/excluded as previously agreed.
