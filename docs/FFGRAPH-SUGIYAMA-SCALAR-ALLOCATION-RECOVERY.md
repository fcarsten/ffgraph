# Sugiyama ordinary allocation recovery — 4 November 2026

Started from clean `import_and_layout_profiling` at `2f9544f` (renderer coordinate
bounds). The user requested continuing large-input correctness. Baseline Debug
configure/build/full CTest passed **88/88**. No commits or pushes made.

## Remaining gap and reproduced problems

The existing checkpoints covered scratch bounds, array allocations, real OS
address-space exhaustion and renderer callback exceptions. They did not sweep
ordinary C++ allocations used by copied node sets, traversal vectors, hash/tree
nodes, route backups and rendering strings. Large-input memory pressure can fail
these allocations as well as large scratch arrays.

Extending one-shot fault injection exposed three concrete problems:

1. **Entry argument copy escaped recovery.** Failing scalar allocation zero in
   `FFSugi::layout()` propagated `bad_alloc`. The legacy `sugiyama` helper accepts
   its node set by value; copying that argument happens before its internal `try`
   block. Its error handler therefore never reported `NO_MEMORY`.
2. **Busy-state setup itself could allocate and fail.** In an actual FFTclWindow,
   `setBusy` changed the cursor, then called `FFWindow::update`. That function
   allocated a C++ argument vector to evaluate the constant Tcl command
   `catch update`. Failure occurred before `BusyScope` finished construction,
   leaving the busy counter/cursor set. The same unnecessary allocation also
   existed in the update called during busy cleanup.
3. **Formatting could swallow allocation errors.** Default output streams can
   catch an allocation exception, set their error state and leave partial output.
   At a demonstrated GUI fault index, a rendering stream supplied an incomplete
   command to Tcl, yielding `wrong # args: should be "moveLine canv c ?arg ...?"`
   instead of the allocation error taking the model rollback path.

These were reproduced incrementally before the corresponding fixes. Additional
negative controls compiled each relevant original source from `2f9544f` and linked
it ahead of the fixed libraries into the final regression executable. The old
entry, old busy update and old formatting implementation each independently
failed the new checks. They are intentionally failing local controls, not disabled
or expected-passing CTests.

## Targeted production changes

- `flayout/FFSugi.cc` catches `std::bad_alloc` around both public layout entry
  points, including by-value argument construction, and uses the existing
  `USER_ERROR/NO_MEMORY` report. The existing busy scope still unwinds normally.
  The low-level helper signature and successful ID behavior are preserved.
- `fgraphic/FFWindow.cc` evaluates the exact fixed script `catch {update}` via
  `Tcl_EvalEx(..., TCL_EVAL_GLOBAL)`. This retains global evaluation and caught Tcl
  update errors while removing the unnecessary C++ argument-vector allocation
  from busy setup/cleanup. There is no interpolated input or new Tcl procedure.
- `fgraphic/FFTclBasicWindow.cc` enables stream error exceptions for node update,
  edge/edge-label formatting and related icon/figure coordinate formatting.
  An allocation error propagates before a malformed command is sent to Tcl.
  Under the tested libstdc++ implementation, the original `bad_alloc` reaches
  Sugiyama's existing publication rollback and `NO_MEMORY` handling. Successful
  formatting produces the same command text.

No layout heuristic, graph/container ownership, public signature, parser,
format, selection policy or Tcl resource script changed. Generic renderer
exceptions retain their existing propagation after model recovery. The previous
projected-coordinate suppression/recovery policy remains in place.

## Regression design and construction checks

`tests/test_sugiyama_failure.cpp` retains the existing array/callback sweeps and
adds `--scalar` and `--scalar-gui`. A one-shot ordinary `operator new` failure is
armed only around the actual public layout call. Matching scalar/array ordinary,
sized and nothrow allocation/deallocation families remain paired. Fault injection
turns itself off when it fires so recovery and diagnostics can run.

The fixtures contain cycles, loops, parallel and long edges, original bent
routes and sparse incoming IDs. Both public entry points are exercised:
parameterless layout and the overload accepting a caller-owned node set. The
default call uses orientation 1; the set-taking call uses orientation 2. Reduction
OFF/ON gives **four headless configurations**. Actual FFTclWindow tests add caller
updates OFF/ON, giving **eight GUI configurations**.

For a reported failure, checks run **before the test resets anything**:

- Exact original node positions and all constructed edge routes.
- Incoming node/edge IDs, graph counts and start node.
- Original route-point Pix/address validity in the live graphics.
- Caller-owned node-set membership and a retained selection Pix.
- Balanced busy counter and the actual canvas cursor restored to `left_ptr`.
- Preservation of the caller's update flag.
- Actual canvas node/edge/label coordinates, visibility states and spline flags.
  One edge begins with spline enabled to exercise restoration of that state.
- No unexpected Tcl errors or escaped `bad_alloc`.

Some nothrow allocations inside stable sorting legitimately recover using a
fallback. Such an injected failure is not automatically treated as an error.
The complete resulting world geometry **and actual canvas drawing** must match
an uninterrupted successful layout on the **same graph objects**. Each sweep
also reaches a completely uninjected successful control.

An external test-only publication snapshot restores the fixture between budgets,
including after a successful fallback. Production failure checks and original
live-storage checks happen first; the test reset does not conceal a failed
production rollback. The successful oracle uses the same objects to avoid the
documented allocation-history sensitivity across separately constructed graphs.
The canvas baseline is redrawn once before recording it, accounting for the
existing addLabel/moveLabel offset difference.

`CMakeLists.txt` registers mandatory headless `test_sugiyama_scalar_failure` and
Xvfb `test_sugiyama_scalar_failure_gui` tests, with finite timeouts. No failing
test is removed and no oracle is regenerated. The latter adds roughly 25–55
seconds locally, depending on configuration/instrumentation.

## Results actually observed

Recorded full native sweeps:

| Build | Headless reported failures / successful fallbacks | GUI reported failures / successful fallbacks |
| --- | ---: | ---: |
| Debug | 3,284 / 280 | 11,368 / 560 |
| Release | 3,284 / 280 | 11,384 / 560 |

These are observed runs across the configurations, not a count of unique source
locations. The exact allocation counts vary with allocator/traversal history and
toolchain; the tests require semantic recovery and completion, not frozen counts.
Both corresponding instrumented sweeps also passed.

| Verification | Result |
| --- | --- |
| Baseline Debug configure/build/full CTest | **88/88 passed** |
| Initial scalar entry regression on original code | Reproduced escaped allocation error |
| GUI entry-only fix control | Reproduced unbalanced busy state |
| GUI stream control before formatting fix | Reproduced malformed Tcl command |
| Three independent saved-original-source controls | Each rejected by the final regression |
| Focused Debug scalar headless/GUI sweeps | **2/2 passed** |
| Focused strict headless sanitizer sweep | Passed with leak checking |
| Focused GUI runtime sanitizer sweep | Passed, leaks OFF |
| Final Debug configure/build/full CTest | **90/90 passed** |
| Final Release configure/build/full CTest | **92/92 passed**, including two cached external Rome tests |
| Final strict headless ASan/UBSan/float-cast/leak suite | **72/72 passed**, LSAN_OPTIONS cleared |
| Final GUI runtime ASan/UBSan/float-cast suite | **16/16 passed**, leaks OFF |
| All CTests have exactly one headless/gui label | Passed in all three builds |
| `git diff --check` and changed test/report whitespace checks | Passed |

Existing array-allocation, 772 renderer-callback failure checks, OS-limit tests,
renderer range/recovery tests, unchanged frozen layouts and the real/synthetic
crossing corpus remain active in their applicable partitions. The native-only
OS-limit policy and existing strict headless/GUI-runtime leak policy are unchanged.

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug -R '^test_sugiyama_scalar_failure' -V
ctest --preset debug
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release
cmake --preset asan
cmake --build --preset asan --parallel 2
LSAN_OPTIONS= ctest --preset asan-headless
LSAN_OPTIONS= ctest --preset asan-gui
```

Xvfb and leak tracing used access outside the restrictive sandbox. Exact logs,
saved original sources/control build commands, failure metrics and provenance are
under `build/sugiyama-scalar-allocation-checkpoint/`. Permanent regressions need
only repository sources. Strict GUI toolkit leak diagnostics, remote CI,
interactive visual assessment and other toolchains/platforms were not rerun.

## Guarantee limits and next work

This is one-shot failure coverage on the exercised C++ allocation paths. It does
not establish survival of persistent system-wide exhaustion, OS termination,
every combination of repeated failures, aligned allocation, or C allocations
inside Tcl/Tk/X11. Recoverable sorting fallback paths are checked semantically;
every later allocation on every such alternate path is not separately faulted.

Error handlers and cleanup require enough resources to run after the one-shot
fault. Arbitrary callbacks that mutate/delete graph members, reenter layout or
throw during busy cleanup remain outside the tested contract. This patch does
not add a general exception translation boundary to all Tcl callbacks. Direct
callers of the low-level by-value `sugiyama` helper can still encounter exceptions
while constructing arguments in their own call context; the production FFSugi
entry points are the recovery boundary tested here.

Canvas coordinates/state and the cursor are checked, not transactional recovery
of every camera, viewport, inspector or external application side effect. GUI
runtime sanitizer passes do not prove GUI leak freedom. Projection clipping/view
rebasing, arbitrary camera validity, Spring global offset bounds and alternate
render/export backends remain separate work.

This closes another practical C++ allocation gap in the large-input checkpoint.
Further exact Sugiyama performance work can use the refreshed profiles while
retaining these failure sweeps. SPQR remains excluded; aesthetic Sugiyama and
WSLg work stay parked; Rome loader optimization remains low priority.
