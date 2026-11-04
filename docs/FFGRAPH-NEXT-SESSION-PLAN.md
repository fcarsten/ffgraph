# ffgraph — current modernisation plan

Updated: 4 November 2026. The current Git checkout and its diffs are authoritative.
This replaces the earlier roadmap that still recommended beginning Pass 8.
Historical checkpoint reports retain their original test counts and scope.

## Current position

FF/sgraph, GML and Rome use modern GNU Bison/Flex parsers in every CMake
build, including BUILD_TESTING=OFF. All legacy implementations have been
retired after generated-parser, graph construction, failure and GUI testing.
There is no legacy FF/GML build selector or automatic fallback.

The Rome pilot began at committed baseline `11a9304` on
`pass12-modern-rome-parser`, with the verification checkpoint already committed.
The local annotated tag
`ffgraph-pre-legacy-ff-removal` points to
`65e5498c40116c0ec85bccb3bbab5d060f098f57`, preserving the old FF parser/scanner.
Do not move that tag to a newer commit. Nothing in this checkpoint was committed
or pushed by the agent.

Container, graph ownership and GUI lifecycle work from Passes 8–10 already has
regression coverage. Avoid restarting those reviews from this document's old
candidate lists. Reproduce any remaining suspected issue before changing code.
The parked WSLg inspector investigation stays parked; SPQR remains excluded.

## Verification checkpoint

See `FFGRAPH-PASS12-VERIFICATION-HARDENING.md` for actual measurements and commands.

- All test checks execute in Release; twenty older tests now use FF_CHECK.
  A regression verifies side effects and intentional failure under NDEBUG.
- Debug and Release CI exercise the complete suite, including Xvfb tests.
- Every CTest has exactly one label: headless or gui. Strict ASan/UBSan/leak
  checking is mandatory for headless tests. GUI runtime ASan/UBSan is mandatory.
  Strict GUI leak checks remain a separate visible diagnostic step with retained
  logs. The full `asan` preset remains strict and can still fail on toolkit leaks.
- System Ubuntu Tk 8.6.14/Xft/fontconfig shutdown leaks reproduce independently
  of ffgraph. Isolated Tk 8.6.18 substantially reduces them but leaves 320 bytes
  per affected process. No system package or application shutdown policy changed.
- The new Rome baseline exposed scanner ownership leaks. Targeted cleanup fixes
  buffer and rejected-token lifetime; the grammar/loading contract is unchanged.

Use the exact environment and reports to judge these results. Remote GitHub
Actions and interactive WSLg use are separate verification tasks.

## Rome completed: modern-only production

The user explicitly authorized skipping the opt-in integration checkpoint and
making modern Rome default while removing legacy. See
`FFGRAPH-PASS12-ROME-LEGACY-RETIREMENT.md`. All three graph importers now use
build-tree GNU Bison/Flex output; no pilot flag or runtime backend selection
is needed. The local annotated tag `ffgraph-pre-legacy-rome-removal` preserves
commit `5db5a38e9c5c4c5bb176ec08c445ee2ea9b7d248` with the complete old parser
and live differential tests. It was not pushed by the agent.

Permanent Rome tests combine frozen pre-removal snapshots, independent topology
and source-graph oracles, actual production loading, 2D/3D window state and real
Tcl File/Load/Merge/Close flows. The external collection remains outside Git;
FFGRAPH_ROME_CORPUS_DIR registers optional local verification.

Rome rejection now preserves destination IDs/directedness/content; complete
long lines, checked integers and actual locations replace old limitations.
Successful merge ID resets, warning/edge skipping and filename dispatch remain
unchanged. Native string imports remain FF. GUI File > Load still creates a
window before parsing. Allocation/callback exceptions during replay remain a
separate unverified transactional concern.

The installed runtime and portability checkpoint below is now completed; see
the November 3 follow-up results near the end of this document.
Keep all graph/corpus/GUI tests active; do not reintroduce legacy generation or
restart the already completed parser migration. Earlier reports retain their
historical baseline counts and policies.

## Dependency, install and portability checkpoint

Verify installed runtime behavior from outside the
checkout, including the ffgraph3D symlink and FF_LIBDIR/FF_ICONDIR resource lookup.
A staged install passing does not prove that launch works away from the source
working directory. Address the stated CMake minimum versus LINK_GROUP support
and test a clean BUILD_TESTING=OFF build.

Test a supported newer Tcl/Tk build through the existing GUI suite before
proposing dependency changes. Tk 8.6.18 testing so far used system Tcl 8.6.14
and fontconfig/Xft, not a fully upgraded stack. The dependency-only explicit
FcFini experiment does not justify adding process-global finalizers to ffgraph.
Keep leak policy and CI limitations visible; do not add broad suppressions.

Profile representative graph imports/layouts before changing containers for
performance. Preserve API/format/GUI behavior, and make focused patches with
regression tests and corresponding docs/FFGRAPH-*.md reports.

The user chose profiling as the immediate next checkpoint. See
`FFGRAPH-IMPORT-LAYOUT-PROFILING.md` for measurements and the corrected GML
sibling stack growth. The Spring coincident-star failure is now fixed by robust
force norms and bounded movement; see `FFGRAPH-SPRING-NUMERICAL-STABILITY.md`.
The iteration-control API remains a separate concern. Repeated FF/Rome ID scans
were measured performance candidates; preserve
duplicate/merge/callback semantics before changing lookup behavior. The
installation/portability work above remains a separate follow-up.

The profiling continuation is recorded in
`FFGRAPH-PROFILING-HOTSPOTS-CHECKPOINT.md`: FF/Rome import growth is approximately
quadratic; GML is approximately linear on the measured sparse graphs. A tool-only
crossing-count prototype agrees with independent oracles and captured real
matrices. The production optimization is now implemented; see
`FFGRAPH-SUGIYAMA-CROSSING-COUNTER-OPTIMIZATION.md` for permanent regression
oracles, full-width count comparisons, measured layout gains and the existing
allocator-dependent geometry limitation on historical sparse-ID inputs.
Production ID lookup remains unchanged because duplicate selection and direct
callback mutations prevent a naive cached index from preserving behavior.

The Rome graph format is deprecated. Retain its loading compatibility and
regression coverage; optimizing the Rome loader is low priority. Prefer FF/GML
for new format work, and prioritize shared layout and FF-specific performance
work over Rome loader optimization.

The earlier November 3 refresh is in `FFGRAPH-LAYOUT-PERFORMANCE-NEXT-STEPS.md`.
With the crossing counter optimized, sparse Sugiyama examples spend 91–92% of
instructions in barycenter row/column sorting; the dense example instead spends
94% in quadratic parallel-edge preprocessing. Spring spends approximately 94%
in its weighted force calculation, with repeated point arithmetic a large cost.
The optimization sequence at that point was: private Spring force arithmetic,
batched stable Sugiyama matrix permutations, then indexed parallel-edge detection.
Each needs old/new semantic comparison before promotion. Approximate Spring
repulsion remains a later explicit optional mode, not a default behavior change.

The user subsequently prioritized an algorithm correctness review, recorded in
`FFGRAPH-LAYOUT-CORRECTNESS-REVIEW.md`. The first corrective checkpoint is now
implemented: the Sugiyama missing-graphics hang and Spring zero-force NaNs are
fixed with permanent regressions. See
`FFGRAPH-LAYOUT-TERMINATION-EQUILIBRIUM-FIXES.md` for actual verification.
The barycenter checkpoint is now implemented too: actual rectangular row counts,
sortedness across zero slots, and the two-level stopping check are corrected.
See `FFGRAPH-SUGIYAMA-BARYCENTER-CORRECTNESS.md` for independent matrix checks,
the avoidable-crossing reproducer and complete validation. Existing frozen layout
oracles were retained. Remaining candidates include weight-disable behavior,
configuration errors and the weighted 2D/3D and local-selection contracts.
Resolve those contracts before freezing corrected references for optimization.

The manual-ASan default-layout prototype leak was reproduced and fixed, including
the actual GUI `exit(0)` path. See `FFGRAPH-APPLICATION-DEFAULT-LAYOUT-OWNERSHIP.md`
for strict headless and actual GUI results, including remaining dependency leaks.

## Sugiyama drawing-quality experiments parked

The user requested undoing the compact/BFS/parent-alignment/horizontal-postpass
experiments after manual review found undesirable tradeoffs. At that checkpoint production code,
layout registration, profiler options and tests were restored to `c1fc4bc`.
The committed numerical, hang, crossing-counter and barycenter fixes remain.
See `FFGRAPH-SUGIYAMA-EXPERIMENTS-PARKED.md` for the local archive and verification.
Do not resume aesthetic refinement without a new user request.

## Follow-up steps implemented after parking

The user authorized the outstanding sequence. See:

- `FFGRAPH-LAYOUT-CONTROLS-CHECKPOINT.md`: weights OFF, finite iteration caps,
  pinned/singleton coordinates, Sugiyama reduction/start controls and offsets.
  The user explicitly chose selected nodes as pinned and preservation of the
  distinct weighted 2D/3D laws; manuals and tests now document those contracts.
- `FFGRAPH-EXACT-LAYOUT-OPTIMIZATIONS.md`: private Spring force arithmetic,
  stable batched barycenter permutations and indexed parallel preprocessing.
  All eight frozen default Sugiyama layouts are unchanged. Measured cases are
  approximately 4.2–6.3 times faster, with per-kernel/permutation differential
  checks; no approximate solver or aesthetic postpass is introduced.
- `FFGRAPH-RUNTIME-PORTABILITY-CHECKPOINT.md`: CMake minimum 3.24, runtime
  resource lookup outside the checkout, install-time prefix/DESTDIR symlink
  handling, source-build overrides and a clean BUILD_TESTING=OFF build.
- `FFGRAPH-FF-IMPORT-INDEX-AUDIT.md`: a new actual-label callback regression
  proves that pointer/value-validated ID caches can still select the wrong
  duplicate. Production replay/indexing remains unchanged.

Final combined local results: Debug 75/75, Release 77/77, strict headless
ASan/UBSan/LSan 63/63 and GUI runtime sanitizers 12/12. See the reports for
commands, limitations and saved provenance. No commits or pushes were made.

## Remaining priorities

The user prioritized Sugiyama optimization over import acceleration on November 4.
The first continuation is implemented in
`FFGRAPH-SUGIYAMA-PERMUTATION-COPY-OPTIMIZATION.md`: cycle-based permutations
skip fixed slots, improve column locality and replace the large scratch buffer
with one saved row. Expanded matrix/crossing checks and all frozen layouts pass.
Matching first-layout workloads measured 12–29% less time on sparse examples;
the dense example was essentially unchanged. Final suite counts remain
75 Debug / 77 Release / 63 strict headless sanitizers / 12 GUI runtime sanitizers.
The report distinguishes matching first-layout snapshots from the existing
allocation-history-dependent warmed FF drawings.

The user then requested broader regression coverage. See
`FFGRAPH-SUGIYAMA-CORPUS-EXPANSION.md`: 13 repository-contained real FF/GML inputs
from the sibling collections/contrib and 18 synthetic graphs now run under six
configurations, with full reference/optimized layouts on the same objects and
independent crossing oracles. The original geometry oracles remain unchanged.
Latest local totals are Debug 77/77, Release 79/79 (two external Rome tests),
strict headless sanitizers 65/65 and GUI runtime sanitizers 12/12.

The user next chose large-input correctness. The first checkpoint is implemented
in `FFGRAPH-SUGIYAMA-LARGE-INPUT-CORRECTNESS.md`: checked scratch dimensions,
64-bit barycenter sums, iterative DFS/rank propagation and scoped preparation
cleanup with ID/route/window-state recovery. Reproduced overflow, stack crashes
and allocation-failure route loss now have permanent regressions. Traversal order
matches recursive references; existing geometry/crossing coverage still passes.
Latest results: Debug 81/81, Release 83/83, strict headless sanitizers 68/68 and
GUI runtime sanitizers 13/13. No commits or pushes made by the agent.

1. Continue large-input correctness with reproduced extreme-coordinate casts,
   spacing/displacement and degree-accumulation boundaries. Then address scalar
   allocation/callback failures during geometry publication; the current recovery
   guarantee covers preparation, not a transactional final drawing update.
2. Use the refreshed profiles to select further exact Sugiyama work later.
   Remaining costs include barycenters, crossing evaluation and matrix-array
   comparisons; retain crossing and geometry compatibility.
3. Design mutation-aware FF ID indexing later, covering direct setValue,
   membership, callbacks and duplicate first-match policy. Import acceleration
   is currently lower priority at the user's request. Spring remains dominated
   by exact quadratic force interactions; approximation needs a separate mode.
4. Validate a complete newer Tcl/Tk/font stack and strict GUI leak behavior
   before proposing dependency changes. Keep the existing diagnostic policy.
Spring global offset bounds and arbitrary camera/spline/label geometry remain
separate features, not part of the completed layout-correctness checkpoint.

Aesthetic Sugiyama refinement remains parked. SPQR stays excluded, the WSLg
inspector investigation remains parked, and Rome optimization is low priority.

## Starting commands

```sh
git status --short --branch
git log -5 --oneline
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release
cmake --preset asan
cmake --build --preset asan --parallel 2
ctest --preset asan-headless
ctest --preset asan-gui
ctest --preset asan-gui-leaks  # Known system toolkit leak failures remain visible
```

Read `BUILD-CMAKE.md`, the FF/Rome legacy-retirement reports and the verification
report before editing. Record uncommitted changes, actual
test counts/results and dependency versions. No commits or pushes without an
explicit user request; never silently disable a failing test.
