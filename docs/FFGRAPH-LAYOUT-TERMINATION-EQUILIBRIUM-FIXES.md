# Sugiyama termination and Spring equilibrium fixes — 2026-11-03

## Scope and baseline

The user requested corrections for the first two findings in
`FFGRAPH-LAYOUT-CORRECTNESS-REVIEW.md`. Work began at `9b72553` on
`import_and_layout_profiling`, retaining the existing uncommitted review,
profiling and ownership documentation and manual diagnostic. No commits or
pushes were made. The preceding unchanged-checkout review had passed Debug
64/64, Release 66/66 and strict headless sanitizer 54/54 tests.

This patch fixes only the missing-graphics Sugiyama hang and zero-force Spring
NaNs. Other weighting, selection, barycenter, configuration and offset findings
remain open. The performance plan remains deferred. APIs, parser/format behavior,
graph ownership, layout settings and GUI event handling were not redesigned.
Rome remains deprecated with loader optimization low priority; SPQR is excluded
and the WSLg inspector issue remains parked.

## Root causes and implementation

### Sugiyama route construction

In `finish_sgraph`, the single-dummy loop index advanced only inside `if (ge)`.
A self-loop or one-dummy long edge without an `FFGraphicEdge` label therefore
visited the same dummy indefinitely. Both top-to-bottom and left-to-right
branches had the error. Missing edge graphics are otherwise tolerated by the
layout code.

`flayout/FFlayout_algorithmen.cc` now increments that index outside the graphics
check in both orientations. Graphics-backed routes follow the same instructions
and advance once as before. Two-dummy branches already advanced unconditionally
and were left unchanged. No edge graphics labels are manufactured by the fix.

### Spring zero-force normalization

`compute_nodePosition` chooses the unclipped path only when force length is
strictly less than temperature. At zero force and zero cooling temperature,
that comparison is false, so the clipped path computed `0/0 * 0`. This occurs
in actual continuous-mode layout at exact equilibrium. The prior robust norm
and bounded temperature corrections do not guard this separate division.

`flayout/FFspringembedder.cc` now skips displacement for zero-length forces in
both dimensions. A zero force has zero displacement at any temperature; no
epsilon threshold was introduced. Nonzero force clipping and fixed-node rules
remain unchanged. Continuous mode still runs and dispatches inspector updates
at equilibrium until stopped; the patch does not introduce early convergence
or alter the cooling/iteration policy.

## Permanent regressions and pre-fix reproduction

`tests/test_sugiyama_missing_graphics.cpp` exercises the real public layout:

- Self-loop and three-node chain with a shortcut: each requires a single dummy.
- Four-node chain with a forward shortcut, and a cycle with a backward long
  edge: two-dummy controls, including original-direction restoration.
- Both orientations, with and without edge graphics, repeated twice: **32
  layouts**. The test checks return, node/edge identity, directedness, incidence,
  start node, edge names/weights, finite coordinates, graphics availability and
  expected route point counts. Existing frozen oracles provide additional
  historical geometry coverage.

`tests/test_spring_equilibrium.cpp` exercises the real public continuous layout
in 2D/3D and weighted/unweighted modes with unit weights:

- A connected pair exactly the ideal distance apart starts at zero force. The
  last container-visited vertex is pinned to suppress the existing startup
  perturbation. This arrangement works independently of pointer iteration order.
- A graph-label-owned headless inspector checks unchanged finite positions on
  every update, including the iteration-100 write, and calls `stop()` at update
  **101**. It uses the normal callback, graph observer and public workspace
  methods; no private-access flag, production hook, Tk window or worker thread.
- While the real workspace is live, it checks zero-force displacement at zero
  and positive temperatures, signed zero, nonzero forces at zero temperature,
  ordinary clipped/unclipped displacement, and immobility of the pinned vertex.
- It verifies no invalid/divide-by-zero floating-point flags, graph identity/
  incidence/directedness, and finite edge geometry. Assertions remain active
  under `NDEBUG`.

Before production edits, both tests were compiled against the old implementation.
The Spring test exited **1**, reporting nonfinite equilibrium positions; the
Sugiyama test **timed out after three seconds** on its first missing-graphics
self-loop. Results are retained in `/tmp/ff-layout-fix-before.json`. They then
passed after the two source corrections. The additional floating-point flag
check was added for final verification.

`CMakeLists.txt` registers both tests in the existing headless graph-test link
group with its ordinary 30-second timeout. A recurrence of the hang fails CTest
by timeout. No existing tests were removed, disabled, relaxed or re-recorded.

## Verification actually executed

| Check | Result |
| --- | --- |
| New Debug tests against old production code | Spring failed; Sugiyama timed out, as above |
| New Debug tests after fixes | **2/2 passed** |
| Complete Debug CMake/build/CTest | **66/66 passed**, including ten GUI tests |
| Complete Release CMake/build/CTest | **68/68 passed**, including ten GUI tests and two configured external Rome tests |
| ASan CMake/build and strict headless ASan/UBSan/LSan | **56/56 passed**, no suppression file |
| Existing GUI sanitizer runtime preset | **10/10 passed**, leak detection disabled by that preset |
| Existing complete Sugiyama frozen-layout regression | Passed in Debug, Release and strict sanitizers; no oracle changes |
| Git whitespace checks | Passed |

The normal CTest suites now include both new regressions. The strict headless
run cleared `LSAN_OPTIONS`; GUI runtime also used the existing preset's normal
leak-disabled policy. Strict GUI shutdown leaks were not retested or suppressed.
Interactive mouse-driven continuous-mode stop behavior was not separately
tested; the regression covers the actual update/stop control flow headlessly,
and existing GUI tests passed. Arbitrary nonfinite forces, numerical extremes,
callback-driven graph mutation and other review findings remain outside scope.
No performance gain or universal layout convergence is claimed.

The review report and next-session plan now mark these two findings corrected.
The historical manual diagnostic in `tools/review_layout_correctness.cpp`
recognizes old broken behavior; use the permanent CTests for current validation.
Its old equilibrium command is now explicitly bounded in the review report,
because a corrected continuous layout legitimately continues until stopped.

## Build and verification commands

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug
ctest --test-dir build/debug --output-on-failure \
  -R '^(test_spring_equilibrium|test_sugiyama_missing_graphics)$'

cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release

cmake --preset asan
cmake --build --preset asan --parallel 2
LSAN_OPTIONS= ctest --preset asan-headless
LSAN_OPTIONS= ctest --preset asan-gui
git diff --check
```

The presets set the ASan/UBSan runtime options. Local logs are
`/tmp/ff-layout-fix-{debug,release}-*.log`,
`/tmp/ff-layout-fix-asan-{configure,build,headless,gui}.log`, plus the pre-/post-
fix focused build logs. Actual GUI tests used the existing Xvfb harness.
