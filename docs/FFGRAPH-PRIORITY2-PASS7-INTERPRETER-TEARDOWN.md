# ffgraph — Priority 2 Pass 7 — deferred Tk interpreter teardown

**Historical artifact:** `ffgraph-priority2-pass7-tk-interpreter-teardown.patch`
**Category:** Priority 2 / Pass 7
**Artifact availability:** retained in the prior conversation; not bundled with this docs-only patch.

> **Historical retrospective — prepared 1 November 2026.** This note documents a patch produced in the earlier ffgraph uplift conversation. It is not a new code change or evidence of fresh execution of that patch. File paths and historical verification are reconstructed from the retained patch/archive and conversation; the current checkout may contain subsequent fixes (including Passes 8 and 9).

## Failure reproduced

The historical workaround in `FFWindow::~FFWindow()` deliberately avoided deleting Tk-initialised Tcl interpreters. A naïve destructor call to `Tcl_DeleteInterp()` was tested under Xvfb and aborted with:

```text
DeleteInterpProc called with active evals
Aborted
```

The relevant case is destroying a graph/window from inside its own Tcl command or a nested `update` callback. Immediate interpreter deletion can run while Tcl still evaluates commands on that interpreter.

## Implemented design

- `FFWindow` accumulates Tk-initialised interpreters requiring deletion rather than deleting them synchronously from a C++ window destructor.
- Added `FFWindow::collectPendingInterpreters()` and `FFWindow::mainLoop()`. The latter processes Tcl/Tk events and collects pending interpreters *after* `Tcl_DoOneEvent()` returns to the top-level loop, outside the active callback.
- Updated `ffmain.cc` to use the new loop and preserve initial graph handles through GUI event processing; release and collect cleanly at shutdown.
- Guarded Tk selection-handler teardown against already-destroyed Tk roots.
- Updated the existing inspector visibility test for the new interpreter ownership, and documented the contract in `BUILD-CMAKE.md`.
- Added `tests/test_ffwindow_tk_teardown.cpp`, conditionally registered for display-backed Xvfb testing.

## Verification

The assistant reported real Tcl/Tk Xvfb tests for repeated creation/destruction, multiple interpreters, already-destroyed roots, and deletion from active Tcl callbacks. Focused ASan/UBSan and patch-applicability checks passed. The assistant did **not** claim a full installed application build in that environment. The user reported that the updated application worked.

## Contract and remaining limitations

This guarantees safe collection only at the new top-level loop boundary. **Any embedding application that runs a different event loop must call `collectPendingInterpreters()` at an equally safe point.** GUI lifecycle testing under the user's Ubuntu/WSLg machine remains advisable, and the inspector visibility incident stays parked.

## Reproduction

```sh
cmake --preset debug
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
ctest --test-dir build/debug -R test_ffwindow_tk_teardown --output-on-failure
cmake --preset asan
cmake --build --preset asan --parallel
ctest --preset asan --output-on-failure
```

## Patch file inventory

`BUILD-CMAKE.md`, `CMakeLists.txt`, `ffmain.cc`, `fgraphic/FFTclBasicWindow.cc`, `fgraphic/FFWindow.cc`, `include/ffgraph/FFWindow.h`, `tests/test_ffwindow_tk_teardown.cpp`, `tests/test_inspector_visibility.cpp`.

## Applying or validating a historical patch

These records are for provenance. If using an old patch, first confirm its baseline and whether equivalent changes are already present:

```sh
git status --short
git apply --check /path/to/ffgraph-priority2-pass7-tk-interpreter-teardown.patch
# Only if the check succeeds and these changes are not already applied:
git apply /path/to/ffgraph-priority2-pass7-tk-interpreter-teardown.patch
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
```

Do not replay historical patches blindly on a current Pass 8/9 checkout. Use `git log`, `git show`, and source comparison to determine what is already merged.
