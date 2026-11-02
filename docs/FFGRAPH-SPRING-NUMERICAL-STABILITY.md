# Spring numerical stability — 2026-11-02

## Problem and root cause

The preceding profiling checkpoint found nonfinite Spring coordinates for a
1,000-node star whose nodes all start at the normal `(100,100,0)` position.
The user requested a fix. Work started from clean commit `cc6d393` on feature
branch `import_and_layout_profiling`. The Debug baseline passed **59/59**.
The original public-file-import/Spring reproducer exited **1**, and the new
analytic force-norm regression was also run before the fix and failed.

Tracing with gdb floating-point traps found two connected problems:

1. `compute_maxForce()` used `FFPoint::abs()`, which squares and sums in
   `FFCoord` (`float`). One captured finite force was approximately
   `(1.435e19, 3.952e19, 0)`. Its length is representable, but its squared
   components overflow. The resulting infinite maximum force reaches the
   cooling function, including an invalid infinity/infinity oscillation test,
   and produces an infinite movement temperature and subsequent NaNs.
2. Correcting the norm alone did **not** fix the layout. The star's growing
   oscillation continued until force calculation itself overflowed. In that
   intermediate experiment, gdb stopped in `compute_forces_edges()` at iteration
   23, squaring a displacement of approximately `(-9.893e18,-2.808e19,0)`.
   The preceding maximum force was `3.126e38`, with temperature `1.768e19`.
   The unbounded `sqrt(maxForce)` movement rule amplified the high-degree
   graph's oscillation instead of damping it.

This was a layout arithmetic issue, reproduced with FF, GML and Rome imports.
The parsers constructed the expected star before layout. Changing the initial
positions, adding jitter or dropping failed tests would have hidden the case.

## Implementation and behavior

The production change is confined to `flayout/FFspringembedder.cc`:

- Use C++17 three-component `std::hypot` for Spring's force lengths. This avoids
  intermediate square overflow/underflow when the length fits in `FFCoord`,
  includes Z, and leaves the shared `FFPoint` implementation unchanged.
- After computing the existing cooling temperature, cap it at the effective
  ideal edge length (`optimalDistance`). Smaller cooling steps remain in use.
  The effective distance retains the existing edge-weight adjustment.

Each iteration can now move a node by at most one effective ideal edge length.
This prevents the observed growing oscillation from reaching the float limit.
The same code serves weighted/unweighted, 2D/3D and local/global Spring layouts;
the regression exercises global layouts. No force formulas, weights, initial
positions, seeds, iteration settings or public APIs were changed.

Final layout coordinates and convergence paths can change wherever the old
temperature exceeded this bound; exact pixel parity is not claimed. This is an
intentional numerical-stability correction. No graph/container ownership,
parser, serialization or GUI event-management code changed. SPQR stays excluded
and the parked WSLg inspector issue remains untouched.

`CMakeLists.txt` registers the new headless `test_spring_numerics` in the existing
graph-test link group. Its timeout is 90 seconds for four instrumented
1,000-node layouts; existing tests/timeouts were not disabled or relaxed.

`tests/test_spring_numerics.cpp` checks:

- Analytic 3-4-5, Z-only and 3-4-12-13 force lengths at scales `1`, `1e19` and
  `1e-30`, plus the maximum length, zero forces and one-indexed output buffers.
  These catch both overflow and loss of small representable lengths.
- The coincident 1,000-node star at quality 2, with unit edge weights, in all
  four 2D/3D × weighted/unweighted combinations.
- Finite node and route coordinates, node/edge identity, exact star incidence,
  graph counts/directedness and straight two-point routes.
- Net displacement no greater than 30 iterations × the default ideal length
  100, with a small rounding allowance. Thus a finite-but-runaway result also
  fails the regression. Assertions remain active with `NDEBUG`.

Build instructions and the next-session plan now point to this report. The
profiling checkpoint preserves its historical measurements and failure counts.

## Verification actually executed

- Before the change: original profiling reproducer failed with nonfinite
  coordinates; the new norm regression failed. A hypot-only intermediate fix
  passed the norm check but still failed the star, as described above.
- After both corrections: the new regression passed. Under gdb, all its cases
  also exited normally with invalid-operation, divide-by-zero and overflow
  traps enabled in the debuggee's SSE/x87 controls. No application floating-point
  environment changes were added.
- CMake configure/build and **Debug 60/60** CTests passed.
- CMake configure/build and **Release 62/62** passed, including both optional
  external Rome corpus tests already configured on this host.
- CMake configure/build and strict **ASan/UBSan/LSan headless 50/50** passed.
- GUI sanitizer runtime **10/10** passed with the existing `asan-gui` preset,
  whose leak detection is disabled. This is not a GUI leak-freedom result;
  strict GUI shutdown-leak diagnostics were not rerun.
- Through the real public file loader, the previously failing 1,000-node star
  passed for **FF, GML and Rome** under strict ASan/UBSan/LSan: two measured runs
  and one warmup per format, nine layouts in total.

- The complete original native profiling matrix now records **159 passed,
  zero failed, 33 skipped**, exiting **0**. The skips retain the original
  documented layout-size ceiling: 27 generated 5,000-node layouts and six
  historical-file layouts above 1,000 nodes. No new cases were excluded.
  All 32 input hashes match the earlier matrix; the current executable hash
  matches the new manifest. There are 795 measured samples plus 159 warmups.
  This run used the same optimized build settings and logical CPU 0, without
  concurrent ffgraph builds, tests or debugger/profiler runs.
- Selected native median Spring times for the formerly failing star are
  **288.985 ms FF**, **292.057 ms GML**, **287.782 ms Rome**, five samples each.
  These measure successful layouts; the old failed warmups provide no valid
  timing comparison. This patch makes no performance-improvement claim.
- Git whitespace checks passed. The small standalone reproduction command
  below uses inputs generated by the original profiling checkpoint.

## Commands and local artifacts

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release
cmake --preset asan -DFFGRAPH_BUILD_PROFILER=ON
cmake --build --preset asan --parallel 2
ctest --preset asan-headless
ctest --preset asan-gui

# Just the permanent regression, including 2D/3D and both weight modes.
ctest --test-dir build/debug -R '^test_spring_numerics$' --output-on-failure

cmake -S . -B build/profile -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DBUILD_TESTING=OFF -DFFGRAPH_BUILD_PROFILER=ON \
  -DCMAKE_CXX_FLAGS=-fno-omit-frame-pointer
cmake --build build/profile --target profile_graphs --parallel 2

build/profile/profile_graphs --phase layout --layout spring --repeat 1 \
  --input build/profile-import-layout-final/inputs/star-1000.g \
  --oracle build/profile-import-layout-final/inputs/star-1000.tsv

# Fresh output directories are required; no timing thresholds are used.
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
python3 tools/profile_graphs.py --binary build/asan/profile_graphs \
  --output build/spring-numerics-asan-stars-local --sizes 1000 --shapes star \
  --phases layout --layouts spring --samples 2

taskset -c 0 python3 tools/profile_graphs.py --binary build/profile/profile_graphs \
  --output build/spring-numerics-profile-local --sizes 100 1000 5000 --samples 5 \
  --real-ff contrib/out.g --real-ff /home/carsten/work/graphs/falk2.g \
  --real-gml /home/carsten/work/graphs/nase2.gml \
  --real-rome /home/carsten/work/demographs/rome/gfinali/graficon100nodi/grafo10106.100 \
  --real-rome /home/carsten/work/demographs/rome/north/g57/g.57.26
```

Local outputs: `build/spring-numerics-asan-stars/` and
`build/spring-numerics-profile/`. Inputs and outputs remain outside Git;
external source graphs were not modified. Logs/debugger traces are
`/tmp/ff-spring-*.log` (plus the original reproducer's `.stdout`/`.stderr`).

## Remaining limits

The regression establishes finite, bounded output and preserved graph
construction, not aesthetic quality or convergence to an optimum. It does not
promise exact coordinate reproducibility across allocator/container orders.
Arbitrary extreme coordinates/weights whose actual forces exceed the float
range are not made universally safe by this patch. Animation and local/fixed
node interactions were not separately exercised; existing GUI tests passed.
The iteration-control API discrepancy documented in the profiling checkpoint
remains a separate follow-up. No commits or pushes were made.
