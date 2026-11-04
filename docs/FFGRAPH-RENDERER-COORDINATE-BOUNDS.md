# Tk renderer coordinate bounds — 4 November 2026

Started from clean `import_and_layout_profiling` at `d3ed654` (real Sugiyama memory
limit tests). The user requested fixing the remaining large-offset/4x zoom
renderer overflow. Baseline Debug configure/build/full CTest passed **86/86**.
No commits or pushes made.

## Problem and reproduced root cause

A valid model X offset of 1,000,000,000, combined with
`window->scaleXYZ(4,4,1)`, produces a projected coordinate of 4,000,000,000.
`FFTclBasicWindow::updateGraph(FFGraphicNode*)` cast it directly to `int`.
UBSan/float-cast checking stopped at baseline line 1831, through
`FFGraphicNode::moveTo`, `finish_sgraph` and `sugiyama`.

Model-coordinate validation cannot bound a later camera projection. The same
unchecked casts also occurred during node/label/edge creation, edge updates,
and icon/figure conversion. Nonfinite projections were equally unsafe.
Widening only the C++ serialization is insufficient: a separate actual Tk
canvas probe accepted coordinates near 4 billion but returned no usable item
bounding box. Tk's geometry/bounding machinery still uses integer bounds.

The exact original two-node probe reproduced the baseline UBSan failure. A
second control linked the new GUI regression with the saved pre-fix
`FFTclBasicWindow.cc` from `d3ed654`; it also failed with the same conversion
error. Both intentionally failing controls are retained separately from CTest.
The original probe rebuilt against the fixed library now exits successfully
under ASan/UBSan/float-cast checking.

## Rendering policy and implementation

World coordinates, camera matrices and layout results are not clamped or
restricted to compensate for a projected-coordinate overflow. Unsupported canvas
items are temporarily hidden. They become drawable again when a zoom, pan, reset
or subsequent model update brings their projection back into range. This does
not automatically center the viewport on the graph.

- New private `fgraphic/FFCanvasCoordinates.h` checks finiteness and the
  **truncated** value against the actual `int` bounds before conversion. Supported
  values retain the previous truncation toward zero, including negative fractions.
  Node checks also cover the integer center +/- radius footprint used by Tcl.
- `fgraphic/FFTclBasicWindow.cc` guards node and label creation/update, edge and
  edge-label creation/update, and icon/figure conversion. New unsupported items
  are created at safe placeholder canvas coordinates and immediately hidden;
  their model coordinates are retained. Existing unsupported nodes keep their
  previous safe canvas geometry while hidden.
- An edge with **any unsupported projected point** is hidden in its entirety,
  along with its label during update. Unsupported points are not dropped and
  connected across, avoiding spurious segments/crossings. This is whole-item
  suppression, not geometric line clipping. Existing near-plane sentinel
  behavior is retained.
- Private Tk item tags record which items the renderer hid and whether they
  were previously disabled. Returning to supported coordinates restores the
  previous state. Items already hidden by another caller are not made visible.
  Deleted/replaced items do not leave a C++ cache of stale IDs or graph pointers;
  replacement item visibility is evaluated again.
- The visibility helper preserves the interpreter result. Creation still returns
  the actual Tk item ID rather than the result of a visibility bookkeeping call.
- Tk excludes hidden items from `bbox`. `getRange` temporarily measures their
  safe canvas representation without processing events and restores hidden
  state, allowing normal icon/shape size queries while suppressed.
- Camera scale/rotation slider refreshes use the same checked conversion. Values
  outside the slider's integer domain are not sent as wrapped integers; their
  unrepresentable UI refresh is skipped. Ordinary camera control behavior is
  retained. This is not a redesign of camera parameter validation.
- `fgraphic/FFTcl3DWindow.cc` excludes nonfinite projected sort keys from
  `_hideLines`, preserving a valid ordering relation for `std::sort`.

No public signature/class data layout, graph/container ownership, layout heuristic,
parser, graph format or Tcl resource script changed. Sugiyama's existing model
publication rollback remains in place; the new visibility state is restored by
its normal refresh after a failure.

## Regression coverage

`tests/test_renderer_coordinates.cpp` checks finite integer boundaries, positive
and negative fractional truncation, float rounding beyond INT_MAX, huge values,
NaN/infinity, and node footprints extending beyond integer bounds. A rejected
conversion must not modify its output argument.

`tests/test_renderer_coordinates_gui.cpp` uses genuine FFTclWindow and
FFTcl3DWindow instances under Xvfb. It covers **eight combinations** of the two
window types, both Sugiyama orientations, and caller updates initially OFF/ON:

- Exact world node/route geometry compared with a headless layout on the **same
  graph objects**, before/after large-offset layouts and camera operations.
- A valid 1e9 offset at 4x zoom, plus negative out-of-range projections from pan.
- Hidden canvas state while unsupported; restoration after reversing pan/zoom,
  including previously disabled items and independently hidden labels.
- Exact ordinary canvas node/edge coordinates and label coordinates from known
  positive/negative fractional projections, checking legacy truncation.
- Independent label overflow from extreme label offsets, while its node remains
  drawable, followed by label recovery.
- Positive/negative overflow, infinity and NaN during actual node/edge/label
  creation and updates, plus figure and icon conversion while suppressed.
- A route containing one unsupported intermediate point and supported nodes:
  the whole edge is suppressed, then recovers after that point is removed.
- Large finite camera scale/angle changes and successful recovery through the
  existing reset command.
- An injected renderer exception **after hiding a newly moved node**: publication
  restores the prior node/route geometry, canvas visibility, busy/update state;
  a following successful layout reuses the graph. This runs in all eight cases.
- Original graph counts, directedness and model snapshots; no unexpected error
  handler calls; genuine Tk window teardown.

`CMakeLists.txt` registers one headless and one GUI test in the existing partitions.
All pre-existing tests remain active. Frozen layout/matrix/crossing oracles and
the real/synthetic layout corpus were not regenerated or relaxed.

## Actual verification

| Check | Result |
| --- | --- |
| Baseline Debug configure/build/full CTest | **86/86 passed** |
| Original two-node probe, old renderer | Reproduced UBSan float-cast overflow |
| New GUI regression linked with saved old renderer | Reproduced the same overflow |
| Original two-node probe, fixed renderer | Passed under ASan/UBSan/float-cast checks |
| Focused Debug numeric and GUI tests | **2/2 passed** |
| Focused GUI sanitizer test | Passed |
| Final Debug configure/build/full CTest | **88/88 passed** |
| Final Release configure/build/full CTest | **90/90 passed**, including two cached external Rome tests |
| Final strict headless sanitizers/leaks | **71/71 passed**, LSAN_OPTIONS cleared |
| Final GUI runtime sanitizers, leaks OFF | **15/15 passed** |

The sanitizer build retains ASan, UBSan and float-cast-overflow checking. GUI
runtime checks use the established leaks-OFF policy; no new suppression or disabled
test was introduced. Xvfb and leak tracing used access outside the restrictive
sandbox. Strict GUI toolkit leak diagnostics, remote CI, interactive display
assessment and other platforms/toolchains were not run.

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug -R '^test_renderer_coordinates' -V
ctest --preset debug
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release
cmake --preset asan
cmake --build --preset asan --parallel 2
LSAN_OPTIONS= ctest --preset asan-headless
LSAN_OPTIONS= ctest --preset asan-gui
```

Logs, pre-fix source/control build commands, original probes, source/binary hashes
and metadata are retained locally under `build/renderer-coordinate-checkpoint/`.
Permanent regression tests need only repository sources and standard GUI test
dependencies.

## Remaining limits

An edge crossing the visible region is still omitted if one of its projected
points exceeds the supported domain. Full clipping/tessellation or automatic view
rebasing would be separate drawing behavior changes. Hidden items do not expand
the canvas scroll bounds; zoom/pan/reset can recover the view. No claim is made
that an enormous drawing can be inspected at every zoom through Tk's integer
canvas. Float precision lost at very large world coordinates is unchanged.

These checks cover serialized projected coordinates and node footprints, not
every toolkit-internal bounding calculation for arbitrary fonts, stroke styles,
embedded windows or extreme custom images near integer endpoints. General camera
invertibility, arbitrary nonfinite camera inputs, unrelated resize callbacks,
alternate export/render backends, structural callback mutation and persistent
Tcl/resource failures remain separate concerns. Additional per-item visibility
queries were not performance-benchmarked in this correctness checkpoint.

SPQR remains excluded. Aesthetic Sugiyama and WSLg work stay parked; Rome loader
optimization remains low priority.
