# Sugiyama resource limits and follow-up audit — 4 November 2026

Started from clean `import_and_layout_profiling` at `86b8b20` (numeric bounds and
publication recovery). Baseline Debug configure/build/full CTest passed **83/83**.
No commits or pushes made. Production layout, graph ownership, rendering and
camera code are unchanged by this checkpoint.

## Problem and test design

The preceding checkpoints checked integer-index representability and injected
allocation/renderer exceptions. They did not establish what happens when a valid
request exceeds an actual OS memory limit. Representability is not an available
memory guarantee, and successful `new` under Linux overcommit does not guarantee
the OS will let a process touch all its pages.

New `tests/test_sugiyama_resources.cpp` exercises actual production entry points
under Linux `RLIMIT_AS`. It builds the graph, optional genuine FFTclWindow and
expected snapshots first, measures its current virtual address space through
`/proc/self/statm`, then permits **8 MiB additional address space** during layout.
This is not an 8 MiB limit on total process memory or resident memory. The test
restores the inherited soft limit after each attempted public layout; the hard
limit is preserved. An incompatible inherited limit fails visibly.

A test-only paired array allocator observes real malloc successes/failures; it
does not inject failure or add a production allocation policy. The existing error
handler must report `USER_ERROR/NO_MEMORY`. Native full-graph cases run twice on
the same graph, with both layout orientations:

| Shape | Nodes / edges | Observed failing allocation |
| --- | ---: | --- |
| Two wide layers | 4,096 / 2,048 | 16,793,604-byte square matrix |
| Three wide layers | 3,072 / 2,048 | 12,607,500-byte matrix slab |
| Deep chain with 256 long shortcuts | 2,048 / 2,303 | 32,768-byte route buffer, after earlier buffers succeeded |

Each failed run verifies exact node/route geometry, original node/edge IDs,
route-point Pix/address validity, graph object membership, endpoint direction,
names, weights, graph counts, directedness and start node. A subsequent successful
layout reuses the same graph after lifting the cap. Wide cases select the existing
no-reduction option for that retry, avoiding large matrices. Route-heavy retries
retain their long-edge preparation/publication workload.

Three direct reducer cases use valid int-indexed dimensions, without constructing
huge graphs or touching huge matrices:

- A **6,400,000,000-byte** square allocation and a **4,800,000,000-byte** slab
  allocation each fail with `bad_alloc` under the additional 8 MiB cap.
- With an additional **80 MiB**, one **67,141,636-byte** matrix allocation succeeds
  and the next fails. Repeating this call under the **same unchanged cap** must
  admit the first allocation again, checking partial-allocation cleanup.

Null reducer inputs ensure an unexpected allocation success cannot be mistaken
for a passing test. These are allocation-path checks, not successful multi-GB
layout claims. Every direct case runs twice.

The actual FFTclWindow/Xvfb test runs the two-layer failure/retry case with caller
updates initially enabled and disabled. It checks balanced busy state, restoration
of the caller's update flag, exact model recovery and Tk window teardown. Across
headless and GUI tests, each native build exercises **22 OS allocation failures**.

Six moderate successful controls run in native **and sanitizer** builds: two
wide-layer shapes and a 512-node chain with 64 shortcuts, each in both orientations.
They check topology, finite actual constructed geometry, changed positions and
strictly increasing chain layer coordinates. The first fixture incorrectly
expected over 100 points on a long output route; existing `finish_sgraph` removes
collinear dummy points. The corrected check requires the reconstructed dummy
route and verifies the chain's layer order independently. Production routing was
not changed to satisfy the fixture.

## Registration and isolation

`tests/run_sugiyama_resources.py` launches each case in a separate process/session,
captures output and requires normal exit. A 45-second wall-clock deadline kills
the case's process group and fails the test; signal exits and missing cases fail.
No expected-success timing threshold is used. CTest also bounds the entire runner.

`CMakeLists.txt` registers:

- `test_sugiyama_resource_controls`: six subprocesses, all presets, headless.
- `test_sugiyama_resource_limits`: nine subprocesses, native Linux builds.
- `test_sugiyama_resource_limits_gui`: two subprocesses, native Linux with Xvfb.

`RLIMIT_AS` conflicts with ASan's large reserved virtual shadow mappings, so the
two OS-limit tests are explicitly native-only. The instrumented worker rejects
accidental limited-mode invocation. Sanitizer builds still run the six successful
resource controls and **all existing** injected allocation/publication-failure
tests with their previous strict settings. No existing test, sanitizer check or
failure diagnostic was disabled. The normal Debug/Release CI suites pick up the
new OS-limit tests automatically; workflow files are unchanged.

## Downstream numeric audit

The audit reproduced an additional **existing rendering problem**, separate from
the completed model bounds/publication-exception checks:

1. Create a two-node graph with graphics and an actual FFTclWindow.
2. Apply `window->scaleXYZ(4,4,1)`.
3. Set the valid Sugiyama X offset to `1,000,000,000`, then run layout.

Model X coordinates remain within the accepted integer range, but projection
produces **4,000,000,000**. `FFTclBasicWindow::updateGraph(FFGraphicNode*)` casts it
to `int` at `fgraphic/FFTclBasicWindow.cc:1831`. The actual instrumented library
reports float-cast overflow through `FFGraphicNode::moveTo`, `finish_sgraph` and
`sugiyama`. The same probe with offset 1,000 succeeds. These isolated probes used
GUI runtime sanitizer settings (leak detection OFF).

The intentionally failing audit probe is retained separately from passing CTests;
it is not a disabled regression or a passing extreme-coordinate claim. Similar
unchecked conversions exist in node/edge rendering, labels/icons and camera
controls. Model range validation cannot establish screen-coordinate safety;
undefined behavior is not a C++ exception that publication rollback can catch.

The next production checkpoint should define and test safe projected-coordinate
handling at the shared rendering boundary, including both 2D/3D, edges and labels,
ordinary truncation compatibility, camera controls and recovery of the view.
Do not add an arbitrary Sugiyama-only offset restriction or silently clamp world
coordinates. This report records the demonstrated problem; it is **not fixed**
by the resource-test patch.

## Publication-backup measurements

Built RelWithDebInfo profilers with frame pointers, using the same current source
and flags. A local **diagnostic-only** control removes the publication backup's
construction/begin/commit/rollback/refresh calls; every numeric/traversal/layout
operation is otherwise retained. That control has no recovery guarantee and is
not registered, installed or selectable in the application. Production safeguards
remain enabled.

Both profilers omit the standard warmup and run exactly one first layout in each
process, avoiding the documented allocation-history sensitivity of warmed FF
loads. Seven alternating control/production pairs per input are pinned to one
available CPU. Timing covers layout, excluding import/validation/snapshot writing.
All **14 full snapshots per workload are byte-identical**, including node geometry
and routes. No build, CTest or Valgrind run overlapped the final native samples.

| Input | Nodes / edges | No-backup control ms | Production ms | Median peak RSS control / production KiB |
| --- | ---: | ---: | ---: | ---: |
| `contrib/out.g` | 499 / 499 | 15.679 | 15.636 | 18,324 / 18,424 |
| Corpus `b4-alledges.g` | 64 / 126 | 16.322 | 16.119 | 8,096 / 8,256 |
| Corpus `dataflow.g` | 158 / 234 | 2.128 | 2.159 | 7,804 / 7,832 |
| Complete bipartite GML | 128 / 4,096 | 4.349 | 5.008 | 14,660 / 15,172 |

Sparse differences are approximately within ±1.5%; they do not establish a
speedup from the backup. The dense case shows about **0.66 ms / 15%** added layout
time and a 512 KiB difference in median whole-process peak RSS. This is a small
local sample, not an exact allocator/backup memory measurement or universal cost.
Host load/frequency and address-space layout are not fully controlled. Large
pre-existing bent-route collections were not separately benchmarked; backup
storage remains O(nodes + existing route points). Initial samples that overlapped
sanitizer testing are retained as diagnostics and excluded from this table.

Two refreshed production Callgrind runs instrumented only the first layout:

- `out.g`: **199,503,926 instructions**; crossing counting is **39.26% self**,
  barycenter permutation **29.91% inclusive**, matrix-array equality **7.57% self**.
  Inclusive and child percentages overlap and must not be added.
- Dense bipartite: **41,549,090 instructions**; allocator operations dominate
  individual self-cost entries. Publication copying is visible, alongside graph
  sets and preprocessing; crossing counting is a small contributor on this case.

This favors further exact crossing/matrix work for sparse graphs and allocation
reduction for dense graphs. Any future cheaper publication strategy must retain
the existing exception and original-route-storage recovery tests. Removing backup
from production is not proposed.

Exact native samples, input/binary hashes and snapshots are in
`build/sugiyama-resource-checkpoint/native-benchmark/`; commands and local control
sources are alongside it. The dense input is the previously generated complete
bipartite 128-node GML profiling workload, not an external Rome collection.

## Actual verification

| Check | Result |
| --- | --- |
| Pre-change Debug configure/build/full CTest | **83/83 passed** |
| Focused native controls, memory limits and real-window recovery | **3/3 passed** |
| Final Debug configure/build/full CTest | **86/86 passed** |
| Final Release configure/build/full CTest | **88/88 passed**, including two cached external Rome tests |
| Final strict headless ASan/UBSan/float-cast/leak CTest | **70/70 passed**, LSAN_OPTIONS cleared |
| Final GUI runtime sanitizers, leaks OFF | **14/14 passed** |
| All CTests have exactly one headless/gui label | Passed in all three builds |
| `git diff --check`, new-file whitespace and runner Python syntax | Passed |
| First-layout control/production compatibility | **56/56 native runs passed**, matching snapshots |
| Current production first-layout Callgrind runs | **2/2 passed** |
| Normal-range projection audit control | Passed |
| Extreme-offset/zoom projection audit | **Reproduced UBSan failure**, as described above |

The final resource tests additionally repeat partial reducer failure under one
unchanged cap; complete suites were rerun after strengthening that check. Xvfb
and leak tracing used access outside the restrictive sandbox.

## Verification commands and limits

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug -R '^test_sugiyama_resource_' -V
ctest --preset debug
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release
cmake --preset asan
cmake --build --preset asan --parallel 2
LSAN_OPTIONS= ctest --preset asan-headless
LSAN_OPTIONS= ctest --preset asan-gui
```

The strict headless partition retains ASan/UBSan/float-cast overflow and leak
checking. GUI runtime tests retain the existing leaks-OFF policy. Actual OS-limit
failure paths cannot be claimed sanitizer-tested from those instrumented controls;
the previous fault-injection tests separately exercise cleanup under sanitizers.

This does not test Linux OOM-killer termination, actual system-wide memory pressure,
every C/scalar allocation or Tcl error path, structural callback mutation,
reentrancy, interactive display quality or other platforms/toolchains. `RLIMIT_AS`
provides controlled allocation failure rather than proof that every representable
graph will fit. No supported graph-size maximum or production memory quota is
introduced. Strict GUI toolkit leak diagnostics and remote CI were not rerun.

Local logs, exact control-build commands/sources/binaries, audit probe outputs,
benchmark samples/snapshots and metadata are under
`build/sugiyama-resource-checkpoint/`. These ignored artifacts supplement the
permanent report; the new regression tests need only repository sources.
SPQR remains excluded; aesthetic Sugiyama and WSLg work stay parked; Rome loader
optimization remains low priority.
