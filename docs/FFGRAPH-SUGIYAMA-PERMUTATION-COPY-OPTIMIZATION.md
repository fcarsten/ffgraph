# Sugiyama permutation copying — 4 November 2026

Started from a clean `import_and_layout_profiling` checkout at `3fad253`.
The user prioritized further Sugiyama performance over FF import acceleration.
This checkpoint optimizes the existing algorithm's matrix permutations; the
parked drawing-quality experiments remain parked. No commits or pushes made.

## Cost and implementation

The previous checkpoint applied the correct stable barycenter permutation via a
`nonzeroSlots * stride` integer buffer. Each selected row/column was copied into
that buffer and back, including slots whose position did not change. Column
copies also traversed the square matrix with a large physical stride. Profiles
showed this dominated several sparse layout workloads. This was a performance
cost, not a demonstrated error in the resulting vertex order.

`flayout/FFreduce_crossing.cc` now decomposes the same destination-to-source
permutation into disjoint cycles:

- Skip fixed nonzero slots as well as the existing fixed zero-barycenter slots.
- Move contiguous rows around each cycle using one saved row and `std::copy_n`.
- Move columns around the same cycles, visiting one physical row at a time.
- Reuse the permutation for the neighboring matrix's opposite axis. Identical
  matrix/partner pointers still receive both axis operations in the same order.

Retain full-stride metadata and padding. Stable sorting, float barycenters,
tie order, reversal phases, sweep scheduling, crossing calculations and position
assignment are unchanged. Scratch space is now O(stride + active axis size),
rather than O(nonzero slots * stride). Graph/container ownership is untouched.

## Compatibility and crossing regressions

`tests/test_sugiyama_permutations.cpp` retains the original exhaustive rectangles
and frozen bubble reference. It adds wider/padded matrices, explicit disjoint
cycles of lengths 2/3/4, fixed nonzero slots, ties, zero holes and three alternating
row/column passes. Each covers no partner, a separate partner and an aliased
partner. The expanded test passed against the pre-change implementation before
the production edit.

The test checks **55,884 cases / 57,036 permutation applications**. After each
application, every integer in both complete matrices matches the frozen reference.
It also compares the production crossing counter with an independent edge-pair
crossing oracle on the reference result. Existing crossing-count, barycenter,
parallel-edge and control regressions remain active. All **eight unchanged frozen
complete layouts**, including node coordinates and edge routes in 2D/3D, pass.

## Native measurements and the allocation-history control

Built before/after RelWithDebInfo profilers with the same compiler flags, no
fast-math, and frame pointers. Initial standard-profiler runs (one warmup followed
by seven fresh-graph layouts) produced different FF snapshots between versions.
This is consistent with the existing allocation/traversal-order limitation:
changing scratch allocations during warmup changes subsequent graph allocation
history. `init_sgraph` traverses pointer-ordered edge sets and assigns missing
scratch IDs in traversal order. These warmed FF runs are retained as diagnostics;
their timings are not used for the compatibility-controlled table below.

A local profiler-only control omits the warmup and initializes its topology oracle
on the first sample. Both controls use the same modified profiler object; the
before control links the saved pre-change crossing-reduction object ahead of the
current layout archive. No production profiler or importer was changed for this.

For each workload, seven alternating before/after processes each perform exactly
one layout, pinned to permitted CPU 0. Layout timing excludes import, validation
and snapshot writing. **All 14 complete snapshots per workload are byte-identical
across versions and repetitions**, including node coordinates, edge routes and
object metadata. The profiler also checks object incidence and directedness.
No build, CTest or Valgrind run overlapped these native measurements.

| Workload | Nodes / edges | Before median ms | After median ms | Time reduction |
| --- | ---: | ---: | ---: | ---: |
| `contrib/out.g` | 499 / 499 | 17.151 | 15.083 | 12.1% |
| Existing Rome layout workload | 57 / 241 | 73.787 | 53.303 | 27.8% |
| Complete bipartite GML | 128 / 4,096 | 3.731 | 3.761 | -0.8% |
| `../graphs/b4_alledges_2d.g` | 64 / 126 | 20.869 | 15.196 | 27.2% |
| `../graphs/dataflow.g` | 158 / 234 | 2.824 | 1.994 | 29.4% |

The dense case has effectively unchanged time and almost identical instruction
counts; it does not benefit from this copying optimization. Rome here is an
existing input to the shared layout algorithm; its loader was not optimized.
These small local samples do not establish universal speedups. Host frequency
and unrelated host load were not controlled. Matrix scratch savings do not prove
a comparable reduction in whole-process peak RSS.

Six first-layout Callgrind runs passed, restricting instrumentation to layout:

| Workload | Before instructions | After instructions | Reduction |
| --- | ---: | ---: | ---: |
| `contrib/out.g` | 294,911,281 | 192,053,658 | 34.9% |
| Existing Rome workload | 1,684,157,083 | 542,483,591 | 67.8% |
| Dense bipartite | 32,808,618 | 32,808,744 | approximately zero |

The crossing-counter self instruction counts are identical before/after on the
two sparse profiles. Remaining costs include barycenter calculation/permutation,
crossing evaluation and matrix-array equality. For `out.g`, crossing evaluation
now accounts for 40.78% self instructions and barycenter permutation 31.07%
inclusive. Inclusive and child percentages overlap and must not be added.
Instrumented elapsed times are not used as native performance measurements.

## Builds and tests actually executed

| Check | Result |
| --- | --- |
| Pre-change Debug CMake/build/full CTest with Xvfb access | 75/75 passed |
| Expanded permutation test before production change | Passed |
| Final Debug CMake/build/full CTest | 75/75 passed |
| Final Release CMake/build/full CTest | 77/77 passed, including external Rome tests |
| Final ASan/UBSan build and strict headless/leak tests | 63/63 passed, LSAN_OPTIONS cleared |
| Final GUI runtime ASan/UBSan, leak detection OFF | 12/12 passed |
| Targeted headless Sugiyama/control tests | 7/7 passed |
| First-layout snapshots and instruction profiles | Passed as described above |
| `git diff --check` | Passed |

The first sandboxed baseline CTest invocation passed all 63 headless tests but
failed 12 GUI tests because Xvfb displays were inaccessible. Running the unchanged
baseline suite with display access passed all 75. Final complete suites used
that access too. No tests were disabled or expected layouts regenerated.

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release
cmake --preset asan
cmake --build --preset asan --parallel 2
LSAN_OPTIONS= ctest --preset asan-headless
LSAN_OPTIONS= ctest --preset asan-gui
```

Raw logs, before/after source and binaries, snapshots, benchmark/profile scripts,
exact compile/link/run commands, input/binary hashes and summary metadata are
under `build/sugiyama-permutation-copy-checkpoint/`. Native measurements are in
`first-native-benchmark.json`; `native-benchmark.json` retains the initial warmed
diagnostics. `instruction-profiles.json` records Callgrind totals, with full raw
profiles and self/inclusive annotations alongside. These ignored local artifacts
will not survive deleting the build directory; this report is the persistent
record. Use an available CPU when repeating measurements on another host.

Strict GUI leak diagnostics, remote CI, interactive visual assessment, other
toolchains/platforms, very large matrix dimensions and allocation-exception
recovery were not validated here. The historical allocator-dependent traversal
behavior is not fixed by this patch; whole-process warmed drawings are therefore
not guaranteed identical. SPQR, WSLg and aesthetic Sugiyama work remain excluded
or parked. FF import indexing remains a later task at the user's preference.
