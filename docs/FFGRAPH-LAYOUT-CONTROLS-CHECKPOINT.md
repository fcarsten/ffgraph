# Layout controls and compatibility contracts — 3 November 2026

Started on `import_and_layout_profiling` at `c1fc4bc`, with only the parking
report/roadmap uncommitted. Pre-change Debug CMake/build/CTest passed **67/67**.
The user authorized the outstanding sequence and explicitly chose to preserve
selected/pinned Spring nodes and the distinct legacy weighted 2D/3D formulas.
The aesthetic Sugiyama experiments remain parked. No commits or pushes were made.

## Demonstrated bugs and fixes

- **Weights OFF:** `change_optDist` unconditionally inspected weights before
  unweighted forces were calculated. A new public-layout trajectory regression
  failed when only edge weights changed. The helper now returns immediately when
  weighting is disabled. Tests compare every coordinate of eight frames exactly
  for unit, zero, negative and larger weights, in directed/undirected 2D/3D runs;
  weighted controls still respond to weights. The configured distance remains
  unchanged; weighted normalization is retained.
- **Finite iteration cap:** positive quality derived a limit without consulting
  `maxIteration`; the branch using it was unreachable through public setters.
  Finite runs now use the smaller of the quality-derived limit and configured
  maximum. Quality zero remains continuous until stopped. Default maximum 1000
  now applies even to small finite graphs; runs previously exceeding it may stop
  earlier. The profiler reports the corrected cap. A counting inspector verifies
  an explicit cap of one and continuous updates beyond that cap.
- **Pinning:** `layout(set)` retains the historical fixed-set meaning selected by
  the user. Position publication no longer overwrites pinned coordinates, including
  Z in 2D. A pinned singleton is left alone; a movable singleton uses its offsets
  and gets Z zero in 2D. Graph membership/ownership is unchanged. GUI selection
  dispatch still invokes the local overload only when more than one node is
  selected; that existing threshold was not changed.
- **Sugiyama no reduction:** selector 2, already emitted by the inspector and
  described in the constructor, is now accepted. Selector 1 retains its legacy
  bypass behavior. Tests check actual reducer arrays remain unchanged for both
  bypass selectors. Inspector synchronization now reflects the selected mode.
- **Sugiyama start flag:** DFS and rank propagation now receive the selected
  start only when enabled. Their historical graph-taking helper functions remain
  available. Tests compare a cycle with the flag OFF against a graph without a
  start, and verify ON has an effect, in both orientations. The stored graph start
  is never temporarily cleared or replaced.
- **Sugiyama offsets:** the level-axis offset was overwritten and left-to-right
  cross-axis handling was inconsistent. Layout now computes relative coordinates
  with zero offset, then translates real/dummy coordinates and long-edge arrays
  before route reconstruction. Eight offset comparisons cover both orientations,
  grid modes and size modes, including loops/parallel/long edges. Default-zero
  geometry still passes all eight unchanged frozen oracles. An initial attempt to
  add offsets inside compaction exposed its one-unit normalization tolerance;
  final translation keeps offsets separate from that rounding behavior.

## Preserved contracts

Weighted planar pairs retain `delta * (r*w/k - k*k/(r*r*w))`; weighted 3D pairs
retain `delta * (r*w*w/k - k*k/(r*r*w))`. Independent weight-2, r=k=100 tests
verify forces 150 and 350, both edge directions, and repeated contributions for
parallel edges. Unweighted neighbor sets collapse parallel adjacency and ignore
edge weights. No new interpretation of nonpositive weighted edges is introduced.

Spring offsets retain their legacy role in movable-singleton placement;
multi-node layout does not enforce coordinate bounds. A trajectory regression
records that changing them has no effect on a multi-node layout. Changing this
would require a separate behavior decision. Historical manuals now describe
pinning, dimensions 2/3, the cap and singleton offsets accurately.

## Files and verification

Production changes are in `flayout/FFspringembedder.cc`, `FFSugi.cc`,
`FFsugiyama.cc`, `FFSugiInsp.cc`, the Spring header and profiler cap calculation.
`test_spring_weight_toggle`, `test_layout_controls` and
`test_layout_controls_gui` add public-API, mathematical and actual Tcl command
coverage. The Sugiyama Tcl resource-path guard makes the inherited controls load
outside the source working directory. Manuals are `doc/ffLayout.tex` and
`doc/ffLabelClasses.tex`; no PDF regeneration was attempted.

The final combined checkpoint passed Debug **75/75**, Release **77/77**, strict
headless ASan/UBSan/LSan **63/63**, and GUI runtime sanitizers **12/12** with leaks
disabled. These totals include the separate exact-performance and runtime-path
patches. Earlier pre-fix new regressions failed as expected on weight-dependent
unweighted trajectories, rejected selector 2 and missing offset translation.
An extended weight fixture initially checked normalization after resetting all
weights to one; its final normalization control now explicitly restores weight
100 before expecting effective distance 500. No failing test was disabled.

Logs, commands and provenance are under `build/remaining-steps-checkpoint/`.

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

Strict GUI leak diagnostics, remote CI, extreme/nonfinite settings and callback
mutation during layout were not validated here. Weighted formulas and local
selection semantics intentionally remain legacy conventions. Parser formats,
SPQR exclusion and the parked WSLg investigation are unchanged.
