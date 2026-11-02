# Springembedder and Sugiyama correctness review — 2026-11-03

**Follow-up:** findings 1 and 2 are now corrected; see
`FFGRAPH-LAYOUT-TERMINATION-EQUILIBRIUM-FIXES.md` for permanent regressions and
verification. The diagnostics/results below record the original `9b72553`
behavior. For current verification use the new CTests; the historical manual
diagnostic intentionally recognizes pre-fix failures.

## Scope and outcome

Reviewed checkout `9b72553` on `import_and_layout_profiling`. Existing uncommitted
profiling and ownership documentation was retained. The user requested a review
before further optimization. The deferred implementation plan is now explicit in
`FFGRAPH-LAYOUT-PERFORMANCE-NEXT-STEPS.md`; the next-session plan points here.

There are reproducible correctness defects despite passing existing tests.
This checkpoint changes documentation and adds a manual diagnostic source,
`tools/review_layout_correctness.cpp`. **No production algorithm, header, CMake
target, normal test, parser or GUI behavior was changed.** No commits or pushes.
SPQR remains excluded, the WSLg investigation parked, and Rome deprecated with
loader optimization low priority.

Read the numerical-stability and crossing-counter optimization reports first.
Their tests establish useful finite-geometry and compatibility properties, not
complete mathematical correctness. Frozen historical coordinates can preserve
old bugs; they must not be treated as the sole oracle for corrective patches.

## Algorithm comparison and evidence limits

The unweighted Spring force calculation is a Fruchterman–Reingold-style solver,
not an implementation of Eades' logarithmic-spring model or Kamada–Kawai energy
minimization. For separation vector delta, distance r and ideal distance k, its
pair force is delta times `(r/k - k*k/(r*r))` for adjacent vertices, with only
repulsion for non-neighbors. This agrees with attraction magnitude `r*r/k` and
repulsion magnitude `k*k/r` in the original
[Fruchterman–Reingold paper, Figure 1 and modelling section](https://www.reingold.co/force-directed.pdf).
Opposite endpoint updates have the correct signs. Weighted forces, 3D, the fixed
distance setting, deterministic coincidence fallback, force-based cooling and
unbounded canvas are ffgraph extensions/choices. Different cooling or absence
of frame clipping alone does not demonstrate a bug. The recent temperature cap
is a documented numerical correction.

Sugiyama has the expected layered pipeline: temporary cycle-edge reversal,
longest-path levels, dummy expansion, barycenter sweeps with equal-barycenter
reversals, best crossing-count retention, coordinate assignment, and restoration
of original edge direction in routes. Its compressed two-dummy representation
also repeats long-edge identities in intermediate layer arrays. This is a
legitimate implementation strategy rather than automatically a missing-dummy bug.
The optimized strict crossing counter is independently covered by exhaustive
small-matrix and full-width tests; this review found no new counter defect.

The original Sugiyama–Tagawa–Toda publication is
[IEEE, 1981, DOI 10.1109/TSMC.1981.4308636](https://doi.org/10.1109/TSMC.1981.4308636).
Its full text could not be retrieved through the available web access (IEEE
robot/JavaScript gate). Therefore an exact line-by-line comparison with its
original two-phase pseudocode is **not claimed**. The accessible primary paper
[Gansner et al., A Technique for Drawing Directed Graphs](https://graphviz.org/documentation/TSE93.pdf)
provides a related layered implementation with different rank/ordering methods;
those differences are not evidence that ffgraph should implement Graphviz's
network simplex or median heuristic. Findings below instead have direct code,
mathematical or documented-contract evidence. Historical local documentation
`doc/ffLayout.tex` and `doc/ffLabelClasses.tex` was also examined.

## Confirmed defects: fix before optimization

### 1. Sugiyama can hang indefinitely without edge graphics — high priority

`flayout/FFlayout_algorithmen.cc:244` and `:340`, in `finish_sgraph`, advance the
single-dummy index only inside `if (ge)`. If a real edge lacks an `FFGraphicEdge`
label, the same dummy is visited forever. Other branches already tolerate
missing graphics, and a graph need not have an edge graphics label.

The public API reproducer is one node with a self-loop, node graphics enabled
and edge graphics absent. Both orientations timed out after three seconds in
Debug and ASan builds. A Debug gdb run hit line 251 repeatedly with **i=2,
ge=null, anzDum=1**. This confirms a non-advancing loop rather than simply slow
crossing reduction. The same branch can affect a single-dummy long edge.

Suggested fix: advance independently of graphics availability. Add bounded
return/graph-preservation regressions for self-loops and long edges, with/without
edge graphics, both orientations. Check the two-dummy branch as a control.

### 2. Spring zero force and zero temperature generate NaNs — high priority

`flayout/FFspringembedder.cc:521` and the corresponding 3D branch use
`absForce < temperature`. At zero/zero, the else branch divides each zero force
component by zero. Cooling legitimately returns zero at equilibrium.

The direct kernel diagnostic produces NaN in both dimensions. A public-API
reproducer places a unit-weight connected pair exactly k=100 apart, pins the
last visited vertex to suppress the startup perturbation, and selects continuous
quality 0. The loop runs at equilibrium and reaches the invalid operation.
With floating-point traps enabled **in the diagnostic process only**, Debug
terminates with SIGFPE; ASan reports FPE at `compute_nodePosition`, line 529.
Without traps, the kernel diagnostic confirms NaN. This is not a new memory
ownership failure or a third-party dependency report.

Suggested fix: explicitly handle zero force before normalization; verify both
dimensions, zero-temperature behavior, a public equilibrium case and fixed
vertices. Preserve continuous-mode stop/event handling. The previous robust
norm fix does not cover this separate division-by-zero case.

### 3. Sugiyama column tie reversal uses columns as the row limit

`flayout/FFreduce_crossing.cc:372–391`: `r_c` receives no row count and passes
`max_c` to `column_barycenter`. On a rectangular matrix this omits real rows or
includes inactive rows. All callers must supply the actual active row count.

A 4×2 example has column edges at rows `{1,3}` and `{1,4}`. True barycenters
are **2 and 2.5**, already sorted and unequal. `r_c` only scans rows 1..2,
sees a false tie, and changes column identities **101,102 to 102,101**. This
violates the helper's stated equal-barycenter-only operation. Best-count
retention may avoid keeping a worse final layout, but the search itself is wrong.

Suggested fix: pass both dimensions through every two-/multi-level call; test
rectangular ties/non-ties and paired neighboring matrix consistency.

### 4. Sugiyama sortedness does not actually skip zero slots

`flayout/FFreduce_crossing.cc:94–111`: `sorted` tests only adjacent entries.
For **[3,0,2] it returns true**, although its documented nonzero subsequence
is [3,2]. Phase-two control flow can therefore proceed on an unsorted sequence.

Suggested fix: compare consecutive nonzero values. Cover intervening/leading/
trailing zero runs, descending values and equal nonzero values. Do not change
the separate phase-one rule that zero-barycenter slots stay fixed.

### 5. Disabling Spring edge weights still lets weights change ideal distance

`flayout/FFspringembedder.cc:713` calls `change_optDist` unconditionally before
selecting weighted versus unweighted force calculation. That helper inspects
the maximum edge weight. With weighting disabled, k=100 and an edge weight of
100, it changes effective **k to 500**. This contradicts the weight toggle's
documented contract and changes unweighted forces and the temperature cap.

Suggested fix: make weight normalization conditional on weighted mode. Add an
actual-layout comparison with weighting disabled and only edge weights varied.
The present reproducer checks the exact helper used unconditionally by layout.

## Confirmed discrepancies needing a behavior decision

### 6. Weighted Spring 2D/3D attraction uses different powers of weight

`flayout/FFspringembedder.cc:300` versus `:398–406` (and the incoming-edge copies):
with weight w, the planar pair force is
`delta * (r*w/k - k*k/(r*r*w))`; in 3D it is
`delta * (r*w*w/k - k*k/(r*r*w))`.

The same planar geometry r=100, k=100, w=2 gives endpoint X force **150 in
2D and 350 in 3D**. For a positive-weight isolated pair, the corresponding
equilibria are `k / pow(w, 2/3)` and `k/w`. Unit weights conceal the difference;
the existing weighted/unweighted star regressions use unit weights.

This is a confirmed dimensional inconsistency, not a proven violation of an
original-paper weighted formula: the original FR model does not specify this
extension. Decide the intended weight meaning, document it and add analytic
non-unit-weight tests in both dimensions before changing either formula. Do
not freeze both inconsistent formulas as the desired optimization contract.

### 7. Local-selection semantics conflict with the manual and singleton path

`init_FSSpringNodes`, lines 36–40, treats nodes in `layout(set)` as **fixed**.
The shipped `doc/ffLayout.tex` and Spring section of `doc/ffLabelClasses.tex`
describe that set as the subgraph to lay out. In a two-node public reproducer,
selecting node a leaves a.x=0 while unselected b moves from x=1000 to about
102.54. The GUI passes its selection to this overload; the intended UX matters.

Even under the implementation's fixed-set convention, Spring's singleton path
ignores the set: **(321,456,17) becomes (100,100,100)**. This also assigns nonzero
Z in nominal 2D mode. Sugiyama's local set is entirely unused, but its source
explicitly says local layout is unavailable. Do not treat that acknowledged
missing feature as a newly implemented local solver.

Resolve whether the public set means selected/movable or fixed vertices before
altering GUI behavior. Then test selected and unselected positions for zero,
one and multiple nodes. A mechanical arithmetic optimization must not silently
change these contracts.

### 8. Layout settings can report values that the algorithms do not honor

- **Sugiyama no-reduction:** `FFSugiInsp.cc` sends selector 2 for `noneRed`, but
  `FFSugi::setCrossingAlg` accepts only 0 and retired selector 1. The diagnostic
  confirms selector 2 is rejected and the setting remains 0. The visible command
  therefore cannot disable crossing reduction. Retired selector 1 currently
  bypasses it through the reducer's fall-through.
- **Spring maximum iterations:** the setter accepts positive values, but
  `springembedder_layout` uses `maxIteration` only in its negative-quality branch.
  `setQuality` rejects negative values. Positive quality derives its own cap;
  zero selects continuous mode. Thus the manual iteration branch is unreachable
  through the public setters. Setter rejection was reproduced; iteration-cap
  behavior was established by source inspection, not a new instrumented counter.
- **Sugiyama start-node toggle:** `setUseStartNode` stores a flag, but neither
  `sdfs` nor `slevel` reads it; both use the graph's start node whenever present.
  This is source-confirmed; no interactive toggle test was performed.

Fix these as separate API/GUI contract patches with focused tests. This is
ordinary setting correctness, not reopening the parked WSLg inspector issue.

## Additional source findings and intentional limitations

- `bc_2`, lines 1058–1062, fills row barycenters but checks sortedness with
  **both row and column counts joined by OR**. On rectangular matrices, one
  check can inspect only a prefix or stale column-barycenter entries. Its
  termination decision should use the relevant complete sequence. A concrete
  final-layout-quality counterexample was not established in this review.
- Spring offsets are used only by its singleton path. The manual describes
  them as minimum coordinates, but multi-node movement neither translates nor
  clamps to them. Sugiyama's `set_positions` overwrites the level-axis coordinate
  without adding the corresponding offset. These are source findings; offset
  geometry/GUI regressions remain to be written before any fix.
- Sugiyama source explicitly marks 3D/local layout, end-node constraints and
  dummy-offset handling unavailable/ignored. Their presence in old API docs
  does not establish a working feature. Existing 2D/3D frozen fixtures matching
  each other are not proof of a 3D Sugiyama implementation.
- Spring's weighted path accumulates an attraction-minus-repulsion contribution
  per nonzero parallel edge, then suppresses the generic pair repulsion. Original
  FR repulsion is once per vertex pair. The unweighted path collapses neighbors
  into a set. Multigraph/weight semantics need explicit decisions and tests;
  neither behavior should be changed silently as an optimization.
- Recursive Sugiyama DFS and level propagation can exhaust the stack on deep
  inputs. Large integer matrix dimensions and barycenter sums, extreme/nonfinite
  coordinates/weights, callback-driven live mutations and exception cleanup
  remain limits. No new overflow, stack-depth or callback reproducer was run.
- Historical noncanonical IDs remain allocator/container-order sensitive,
  already demonstrated in the crossing-counter report. That known limitation
  is separate from crossing-count arithmetic.

## Verification actually executed

Debug CMake configure/build and all **64/64 CTests passed**, including ten GUI
tests. Release CMake configure/build and **66/66 CTests passed**, including ten
GUI tests and the two configured external Rome tests. ASan CMake configure/build
and strict headless **ASan/UBSan/LSan 54/54 passed**, with `LSAN_OPTIONS` cleared.
GUI sanitizer runtime and interactive layout-control testing were not repeated
in this review. Existing test outcomes must be read alongside the reproduced
defects; no normal test was disabled or relaxed.

The standalone diagnostic was compiled against the actual Debug and ASan
static libraries. Its Spring workspace access uses `-fno-access-control` only
for this source. No production source/header instrumentation or build flag was
added. It is deliberately not registered in CTest: most cases check whether
known broken behavior reproduces, and are not correctness regressions that
would protect a fix. Convert each into tests asserting corrected behavior in
the corresponding future patch.

In **both builds**, eight finite diagnostic cases reproduced: weighted force,
unweighted distance, zero force, local selection, fixed singleton, rectangular
ties, zero-hole sortedness, and rejected controls. All exited 0, meaning
**observation confirmed**, not bug fixed. The ASan runs used strict leak/UB
checking with no suppression file and emitted no sanitizer report in these
normal-return cases. Sanitizers do not by themselves detect incorrect forces,
ordering or ordinary floating-point NaNs.

The two missing-graphics cases per build were killed at three seconds. Those
four timeouts are reproduced failures, not passing leak tests; process cleanup
cannot be assessed after forced termination. The public equilibrium case
deliberately enabled floating-point traps: Debug exit -8 (SIGFPE), ASan exit 1
with FPE at line 529. Both are reproduced failures; they are not successful
sanitizer runs. No broad suppression or production FP trap was introduced.

Results: `build/layout-correctness-review/observations.json` (22 records), plus
`missing-graphics.gdb.txt`; `metadata.json` records source/executable hashes and
the Git commit. Local build commands are retained at
`build/{debug,asan}/layout-review-build-command.txt`. CMake/CTest logs are
`/tmp/ff-layout-review-*.log`. The process survived a VS Code restart; these
artifacts were checked before continuing. No performance gains are claimed.

## Reproduction commands

Build the existing libraries/tests, then link the manual diagnostic without
changing the main CMake configuration:

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug

c++ -std=c++17 -g -fno-access-control -Iinclude -I. \
  $(pkg-config --cflags tk) tools/review_layout_correctness.cpp \
  -o build/debug/review_layout_correctness \
  -Wl,--start-group build/debug/libFFGraph.a build/debug/libFFGraphic.a \
  build/debug/libFFlayout.a build/debug/libFFcontainer.a \
  build/debug/tests/modern_gml/libffgraph_modern_gml.a \
  build/debug/tests/modern_ff/libffgraph_modern_ff.a \
  build/debug/tests/modern_rome/libffgraph_modern_rome.a -Wl,--end-group \
  $(pkg-config --libs tk) -lX11 -lXpm -ldl -lm

build/debug/review_layout_correctness weighted-force
build/debug/review_layout_correctness unweighted-distance
build/debug/review_layout_correctness zero-force
build/debug/review_layout_correctness local-selection
build/debug/review_layout_correctness fixed-singleton
build/debug/review_layout_correctness rectangular-ties
build/debug/review_layout_correctness zero-hole
build/debug/review_layout_correctness controls
timeout 3s build/debug/review_layout_correctness no-edge-graphics-1
timeout 3s build/debug/review_layout_correctness no-edge-graphics-2
# Expected timeout exit 124 for each of the two hang reproductions.

ulimit -c 0
timeout 3s build/debug/review_layout_correctness public-zero-force
# Pre-fix: expected SIGFPE. With the fix, this bare continuous-mode diagnostic
# instead times out; use test_spring_equilibrium for bounded current verification.

cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release
cmake --preset asan
cmake --build --preset asan --parallel 2
LSAN_OPTIONS= ctest --preset asan-headless
```

For the sanitizer diagnostic, substitute `build/asan` for `build/debug` in the
link command and add `-fsanitize=address,undefined -fno-omit-frame-pointer`.
Run with `LSAN_OPTIONS=` and `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1`,
`UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`. The public trap case will
emit an intentional ASan FPE report. `feenableexcept` is Linux diagnostic code;
this harness is not a portable installed program or production API.

## Proposed corrective checkpoints

1. Fix the null-graphics non-advancing loops and zero-force normalization, with
   new passing regressions demonstrating the old failures.
2. Correct rectangular tie computation, zero-hole sortedness and the two-level
   stopping check. Compare intermediate matrices with independent invariants;
   review intentional historical snapshot changes explicitly.
3. Resolve weighted force meaning and weight-disable behavior, then add analytic
   positive non-unit-weight and multigraph tests in both dimensions.
4. Resolve local-selection and configuration contracts; test public setters and
   affected Tcl commands without changing the parked inspector workstream.
5. Freeze corrected reference behavior and resume the deferred performance plan.

Each correction should have its own minimal diff, report, CMake/CTest and
appropriate sanitizer checks. GUI latency, aesthetic optimality, universal
convergence and exact original Sugiyama pseudocode fidelity remain unverified.
