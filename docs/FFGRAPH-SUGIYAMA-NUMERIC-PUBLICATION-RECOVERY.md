# Sugiyama numeric bounds and publication recovery — 4 November 2026

Continues `FFGRAPH-SUGIYAMA-LARGE-INPUT-CORRECTNESS.md`. The session began with
that checkpoint uncommitted; the user committed it as `99167d3` while this work
continued. Saved pre-change source copies were checked byte-for-byte against
that commit. The agent did not commit or push anything.

## Reproduced problems

Actual pre-change code demonstrated:

- **Nonfinite settings accepted:** `setxDistance(NaN)` returned YES because the
  comparison with the lower limit was false. The offset/distance setters had the
  same missing finiteness condition.
- **Out-of-range conversion:** instrumenting the saved coordinate implementation
  with UBSan/float-cast-overflow reported conversion of `1e30` to int in
  `upper_barycenter`. The ordinary probe returned zero despite executing the
  undefined conversion; a plausible result did not establish safety.
- **Priority overflow:** the actual priority helper with outdegree INT_MAX
  produced UBSan's signed-overflow diagnostic for `2147483647 + 1`.
- **Compaction hang:** `correct_lines` with x=16,777,216, a second point at
  16,777,218 and step 1 timed out after two seconds. Float addition rounded x+1
  back to x, so the inner scan repeatedly restarted without advancing.
- **Partial publication:** a one-shot exception in a genuine FFTclWindow renderer
  callback, after publication began, failed the unchanged-geometry assertion.
  Existing preparation cleanup restored IDs but could not undo route/node writes.

Pre-change probes, exact commands, sources, outputs and expected failing GUI
log are retained in `build/sugiyama-numeric-publication-checkpoint/`. None of
these deliberate failures is hidden or registered as an expected production pass.

## Numeric changes

Private `flayout/FFsugi_numeric.h` checks finiteness and the **truncated** value
before coordinate-to-int conversion. Bounds are compared as doubles, avoiding
the float rounding of INT_MAX to 2^31. Negative fractional truncation and valid
boundary values retain C++'s existing truncation behavior. Signed spacing sums
are promoted to int64 before evaluation and checked before narrowing.

`FFimprove_position.cc` uses these checks at its coordinate, width, displacement
and spacing conversions. Half-width truncation and the original evaluation order
are preserved for valid values. Priority and vertical-spacing integer increments
use the existing checked-add helper. `correct_lines` verifies that each float
advance is finite and strictly greater than its previous value; an unrepresentable
step reports a numeric error instead of hanging.

The six FFSugi offset/distance setters reject NaN and infinities while preserving
the last valid setting. Finite values retain their existing setter bounds. Layout
preflight validates settings, initial node coordinates, graphic dimensions and
incoming edge points; computed node/dummy/route coordinates are checked again
before publication. Model XYZ values used by integer rendering/conversion must
fit the existing int domain, and all homogeneous components must be finite.
Negative/nonfinite graphic dimensions are rejected.

Unrepresentable calculations report an existing USER_ERROR and preserve the
previous drawing and IDs. No clamping, wrapping, altered force law or new graph
size limit is introduced. Representable large spacing is tested as a successful
control. Arbitrary camera/projection calculations remain a separate concern.

Sanitizer builds now explicitly include **float-cast-overflow** alongside address
and undefined-behavior checks, for project-owned code. Existing leak partitions
and suppression policy are unchanged.

## Geometry publication and storage lifetime

Private `flayout/FFsugi_publication.h` captures node positions, spline flags and
complete edge-point containers **before any geometry write**. Preparation of that
backup can fail without changing the model. Publication then runs the existing
straighten/move/reconstruct operations on route copies while retaining original
route storage in the guard. Only after the operations and final window refresh
succeed are the backups released.

On a C++ exception, recovery restores positions, IDs, spline flags and the original
route containers without allocation or renderer callbacks. Renderer refresh is
then attempted separately, with the caller's update flag restored even if that
refresh throws again. The original exception is retained; bad_alloc uses the
existing NO_MEMORY report, while ordinary renderer exceptions propagate after
recovery. Busy handling remains balanced.

This needs narrow friend access from FFGraphicNode/FFGraphicEdge to the private
publication guard and a nonvirtual, noexcept `MPlex::swap`. The swap exchanges
the complete storage, bounds, chunk size and deleted-slot bookkeeping; Pix/slot
addresses travel with their storage. It does not change the MPlex object layout,
existing assignment semantics, or graph node/edge/graphic-label ownership.

**Lifetime contract:** a failed publication restores original route storage and
point handles. A successful layout replaces route storage; callers must reacquire
point references after that rewrite. No node, edge or graphic-label object is
replaced. Observations during the operation and reentrant structural edits are
not transactional guarantees.

The backup adds O(nodes + existing route points) storage and copying. Native
performance/peak-memory effects were not benchmarked in this correctness patch;
the earlier permutation optimizations and sorting/crossing behavior remain.

## Regression coverage

`test_sugiyama_numeric` checks:

- Positive/negative fractional truncation, int endpoints, NaN/infinity, signed
  spacing bounds and an unrepresentable Z coordinate.
- The exact non-advancing compaction case and both demonstrated cast/priority
  errors, without large graph or matrix allocations.
- All six nonfinite-setting setters, preserving their last valid value.
- Six rejected public layouts across both orientations, preserving exact prior
  node/route geometry and incoming IDs: overflow-producing spacing/width,
  nonfinite dimensions and excessive offsets.
- Successful controls with large but representable spacing.

The MPlex regression adds self/round-trip swaps with different bounds and chunk
sizes, a deleted slot and live Pix references. It asserts noexcept operation and
slot-address stability as storage moves between containers.

The existing allocation-failure executable now also tests publication:

- **772 one-shot renderer failures:** bad_alloc and runtime_error; original
  update flag NO and YES; crossing reduction off and on. The eight combinations
  exercise 96 failure points with updates disabled and 97 with them enabled.
- Failure points cover edge/node updates, spline updates, full redraw and window
  adjustment. A real FFTclWindow is used under Xvfb.
- Every failed run restores exact geometry, incoming node/edge IDs, busy/update
  state, spline state and a saved route-point Pix/address that normal straightening
  would delete. A following successful run completes each sweep.
- Direct rollback checks with both scalar and array allocations forbidden verify
  zero allocation attempts during model restoration.
- The existing **44 array-allocation failures** and successful controls continue
  to pass in both headless and GUI operation.

All original frozen geometry, matrix/crossing and 31-graph/six-configuration corpus
tests remain active. No oracle was regenerated and no failing test was disabled.

## Actual builds and verification

| Check | Result |
| --- | --- |
| Targeted Debug geometry/numeric/container tests | Passed after fixture correction below |
| Final Debug configure/build/full CTest | **83/83 passed** |
| Final Release configure/build/full CTest | **85/85 passed**, including two external Rome tests |
| Final strict headless ASan/UBSan/float-cast/leak tests | **69/69 passed**, LSAN_OPTIONS cleared |
| Final GUI runtime ASan/UBSan/float-cast tests, leaks OFF | **14/14 passed** |
| `git diff --check` and new-file whitespace checks | Passed |

The first swap fixture incorrectly expected `low()` to return the reserved lower
bound after its first slot was logically deleted. Its expected value was corrected
to the first present index; container behavior was not changed to satisfy it.

The first strict sanitizer run caught an allocation-family mismatch in the new
fault-injection harness. Stable sorting used nothrow allocation that bypassed the
custom allocator, then reached its custom free-based deallocator. Added matching
scalar/array nothrow allocation/deallocation overloads. Targeted strict tests and
the complete suites then passed. **Allocator-mismatch checking was not disabled.**
The initial failing sanitizer log is retained separately.

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug -R 'sugiyama_numeric|sugiyama_publication_gui|test_mplex_stl' -V
ctest --preset debug
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release
cmake --preset asan
cmake --build --preset asan --parallel 2
LSAN_OPTIONS= ctest --preset asan-headless
LSAN_OPTIONS= ctest --preset asan-gui
```

Xvfb/leak tracing required execution outside the restrictive sandbox. Strict GUI
toolkit leak diagnostics, remote CI, interactive visual assessment and other
platforms/toolchains were not run. Build/CTest logs, baseline controls, case
metrics, source/binary hashes and commands are saved locally under
`build/sugiyama-numeric-publication-checkpoint/`; permanent tests need only the
repository sources and fixtures.

## Limits and next work

- Model recovery covers exceptions during geometry writes and their explicit
  final refresh. It does not roll back arbitrary callback changes to graph
  membership, label objects, colors or external application state. Callbacks
  that delete/reenter the graph need a separate ownership/reentrancy design.
- A repeated repaint failure can leave the canvas stale even though the model
  and update flag are restored. Camera, viewport and other GUI side effects
  are not snapshotted. Cleanup methods are assumed to return normally. Tcl
  error return codes that do not become C++ exceptions keep existing behavior.
- Fault injection is not exhaustive for every scalar/C allocation or external
  callback. No claim is made that all external toolkit failures are recoverable.
- Model bounds do not prove all downstream camera/projection/font calculations
  safe under extreme zoom, custom icons or arbitrary geometry settings.
- Representable but huge memory/time requests, OS termination under memory
  pressure, every external input graph and non-Linux behavior remain unverified.
  Resource-budget/stress tests and refreshed performance measurements are later
  checkpoints; no new timing thresholds were added to CTest.

Public layout signatures, graph formats, normal settings, default registration
and Spring formulas remain unchanged. SPQR stays excluded; aesthetic Sugiyama
and WSLg investigations remain parked; Rome loader optimization is low priority.
