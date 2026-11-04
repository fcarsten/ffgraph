# Sugiyama real and synthetic graph coverage — 4 November 2026

The user requested broader graph coverage after the permutation-copy optimization.
Started on `import_and_layout_profiling` at `3fad253`, preserving the preceding
uncommitted optimization/test/report changes. No commits or pushes made.

## Coverage gap and input selection

The existing suite had strong small-matrix oracles and eight complete synthetic
layouts, but only a handful of real inputs had been compared manually. Different
dummy expansion, uneven layer widths, cycles and graphic settings needed durable
whole-layout coverage. Frozen screenshots of arbitrary real graphs would also
conflate sorter changes with the existing pointer/allocation-order dependence.

Inventoried 114 FF/GML files under `../graphs` and 95 under `../demographs`.
There is substantial duplication. Selected **13 real inputs**, copied unchanged
into `tests/sugiyama_corpus/real/` (157,614 bytes total):

| Fixture | Original input | Nodes / edges |
| --- | --- | ---: |
| b4-alledges | `../graphs/b4_alledges_2d.g` | 64 / 126 |
| dataflow | `../graphs/dataflow.g` | 158 / 234 |
| hierarchy | `../graphs/hierachy.g` | 43 / 31 |
| cluster | `../graphs/cluster1.g` | 11 / 14 |
| cme | `../graphs/cme2.g` | 18 / 26 |
| ten-dm | `../graphs/10dm.gml` | 17 / 32 |
| big-tree | `../demographs/graphed/bigTree.g` | 180 / 179 |
| planar | `../demographs/graphed/planar4.g` | 57 / 148 |
| cube | `../demographs/3D/cube.g` | 8 / 12 |
| dodekaeder | `../demographs/3D/dodekaeder.g` | 12 / 30 |
| gml-sample | `../demographs/gml/sample2.gml` | 2 / 1 |
| nase | `../demographs/nase.gml` | 32 / 69 |
| process-tree | `contrib/out.g` | 499 / 499 |

`provenance.json` records original paths, SHA-256 hashes, sizes and baseline
counts; all copies were checked byte-for-byte against the originals. The counts
were recorded with the existing profiler before adding the corpus tests. They
are import baselines, not independent parser correctness proofs. Normal CTest/CI
uses the repository copies and does not depend on sibling directories or a local
build-artifact archive. Five original trailing-whitespace lines in the fixture
data are retained to preserve byte identity; hashes record the exact contents.

Added **18 synthetic graphs** covering empty/singleton/isolated graphs, loops,
96-deep chains and shortcut edges, 128-wide fans, alternating layer widths,
reversed matching, ties/zero slots, K24,24, parallel/antiparallel edges, strongly
connected and disconnected components, three seeded layered graphs, and unusual
working node/edge IDs. Their construction is explicit in C++; no random library
distribution or external generator is required. Names/weights and graphic sizes
vary, with positive, zero and negative edge weights retained as graph attributes.

Every graph runs under **six configurations** (186 paired comparisons / **372
complete layout executions**): default 2D, 3D, horizontal/grid-off/individual-size
with offsets, start-node-off/grid-off, horizontal-3D/leaves-down, and no reduction.
See the corpus README for the exact setting table. Representative Debug runs
reached 96 layers and a layer width of 265 after dummy expansion.

## Reference design and assertions

`tests/sugiyama_verified_reduction.cpp` compiles the actual production reduction
source as a test-only replacement object. Private preprocessor guards around
`b_r`/`b_c` enable verification in this translation unit. The normal application
does not define that guard, link the reference sorter, or pay verification costs.
Symbol inspection confirmed the checker in the test binary and absent from the
ordinary Debug application.

For each optimized permutation call, copy the complete incoming matrix/partner,
run the frozen bubble reference, and compare every output integer. Metadata,
padding and unchanged slots are included. Check crossings using an independent
O(edges*columns) inversion oracle, rather than the production running-right
counter recurrence.

Each configuration also executes a complete layout using the frozen bubble sorter
and then an optimized layout on the **same graph objects**. Restore incoming
IDs and node coordinates and reset random seeds between them. Global Sugiyama
straightens/reconstructs edge routes. All other reduction, ranking and position
code is the current implementation in both runs; this does not reintroduce a
legacy parser or claim to compare the entire untouched 1990s program.

Assertions cover:

- Exact finite node coordinates and complete edge routes, with no tolerance or
  regenerated geometry oracle.
- Node/edge object identity, graph membership, predecessor/successor incidence,
  endpoints, names, weights, node sizes/colors, directedness and start/end nodes.
  Temporary node/edge scratch-ID changes remain the documented legacy behavior.
- Equal initial/final adjacent-layer crossings, dimensions and layer widths for
  the reference and optimized runs. Reduction must not increase the crossing
  total; no-reduction must keep it and invoke no barycenter permutations.
- Independently known results: reversed matching goes from **66 to 0** crossings;
  K24,24 retains **76,176 = choose(24,2)^2** under every ordering.
- At least one verified permutation is executed per test process, so a mistakenly
  linked ordinary archive cannot turn the checker into an unused assertion.

The saved verbose Debug run checked **76,836 actual permutation calls** across
**149,036 matrix/partner instances**, in addition to full-layout comparisons.
These dynamic counts can differ with allocator/traversal order; they are evidence
from that run, not frozen cross-platform thresholds. All original matrix tests
and eight complete-layout oracles remain active and unchanged.

## Files and actual verification

Added `tests/test_sugiyama_corpus.cpp`, `sugiyama_corpus_verification.h`,
`sugiyama_verified_reduction.cpp`, fixture copies, count/provenance manifests and
README. `CMakeLists.txt` registers two default headless tests with a 300-second
timeout each. The only additional production-source edits in this checkpoint
are test-only guards in `flayout/FFreduce_crossing.cc`; normal algorithm behavior
and settings are unchanged. Updated build instructions and the session roadmap.

| Check | Result |
| --- | --- |
| All 13 real-input preflight layouts | Passed |
| Fixture hashes and byte equality with local originals | Passed |
| New real/synthetic CTests, verbose Debug | 2/2 passed |
| Full Debug configure/build/CTest | **77/77 passed** |
| Full Release configure/build/CTest | **79/79 passed**, including two configured external Rome tests |
| ASan/UBSan build and strict headless/leak CTest | **65/65 passed**, LSAN_OPTIONS cleared |
| GUI runtime ASan/UBSan, leak detection OFF | **12/12 passed** |
| `git diff --check` | Passed |

Full suites used display/leak-tracing access outside the restrictive sandbox;
no failing test was disabled. The extended tests pass in optimized Release with
checks active. Strict GUI leak diagnostics and remote CI were not rerun.

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug -R '^test_sugiyama_corpus_' -V
ctest --preset debug
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release
cmake --preset asan
cmake --build --preset asan --parallel 2
LSAN_OPTIONS= ctest --preset asan-headless
LSAN_OPTIONS= ctest --preset asan-gui
```

Selection/preflight logs, final build/CTest logs, verbose case metrics and
hash/provenance metadata are saved locally in `build/sugiyama-corpus-checkpoint/`.
These ignored artifacts are supplementary; the inputs, generator logic and
reference checker required to repeat the tests are all in the repository.

## Limits and remaining risks

Crossing metrics count **adjacent-layer matrix crossings after dummy expansion**.
They do not count arbitrary routed-polyline intersections or label/node overlaps.
Loops/parallel-edge handling follows production preprocessing. Exact full-route
comparison adds regression coverage but does not prove drawing quality or every
phase of the original algorithm mathematically correct.

This is a representative selection, not a test of all 209 local files. Very large
inputs (for example the 3.3 MB maple graph), unsupported formats, extreme recursion
or integer limits, allocation exceptions, interactive GUI appearance and other
toolchains/platforms remain unverified. Some fixtures contain FFSubGraphLabel
payloads unsupported by the default/headless registry; its existing missing-label
diagnostics remain visible. Tests cover the resulting flat graphs, not external
nested subgraph loading. Source graph names/labels were not rewritten.

The same-object design avoids conflating optimized permutations with historical
allocation-history-dependent drawings from separately constructed graphs. That
legacy behavior still exists. No timing thresholds were added. SPQR, WSLg and
the aesthetic Sugiyama experiments remain excluded/parked; Rome loader and FF
import performance work remain outside this checkpoint.
