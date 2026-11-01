# ffgraph — Priority 2 Pass 4 — Tcl command lifetimes and label self-replacement

**Historical artifact:** `ffgraph-priority2-pass4-tcl-label-ownership.patch`
**Category:** Priority 2 / Pass 4
**Artifact availability:** retained in the prior conversation; not bundled with this docs-only patch.

> **Historical retrospective — prepared 1 November 2026.** This note documents a patch produced in the earlier ffgraph uplift conversation. It is not a new code change or evidence of fresh execution of that patch. File paths and historical verification are reconstructed from the retained patch/archive and conversation; the current checkout may contain subsequent fixes (including Passes 8 and 9).

## Scope

Review Tcl/Tk callbacks that retain raw C++ `ClientData` pointers, plus node, edge and graph label setters that transfer pointer ownership. Avoid broad changes to window hierarchy or graph-format compatibility.

## Main changes

- `FFTclBasicWindow`, `FFTclWindow`, `FFTclSubWindow`, `FFTclInspector` and derived inspector classes: record Tcl command tokens and remove commands as associated C++ objects are destroyed; use deletion callbacks so state is invalidated if Tcl removes a command itself.
- Subwindow callbacks: avoid registering a selection handler that overrides the parent window's handler; preserve/release the borrowed interpreter appropriately using `Tcl_Preserve`/`Tcl_Release`.
- `FFNodeClass`, `FFEdgeClass`, `FFGraphClass`: avoid deleting label objects or label collections on same-pointer/self-container replacement; retain existing ownership-transfer APIs.
- Updated matching declarations in the `include/ffgraph/` headers and CMake test registration.

## New tests

- `test_tcl_command_lifetime.cpp`: actual Tcl command deletion and owner-destruction registration behaviour.
- `test_label_replacement.cpp`: replacing labels with their existing pointers or containers must not double-delete or invalidate labels.

## Verification evidence

The assistant reported passing C++17 syntax checks with user-supplied Tcl/Tk headers and the Tcl 8.6 command-lifetime test; the complete application was not built in that environment. The user reported that the patch compiled, CTest passed, and initial interactive use appeared normal.

## Residual risks

`Tcl_DeleteInterp` for active Tk windows remained intentionally disabled, and graph-label parent ownership cycles remained. Those were tackled later (Passes 5–7). Inspector visibility under WSLg was separately investigated and remains parked; it should not be described as resolved by this patch.

## Patch file inventory

`CMakeLists.txt`, `fgraph/FFEdgeClass.cc`, `fgraph/FFGraphClass.cc`, `fgraph/FFNodeClass.cc`, `fgraphic/FFTclBasicWindow.cc`, `fgraphic/FFTclGraphInsp.cc`, `fgraphic/FFTclSubWindow.cc`, `fgraphic/FFTclWindow.cc`, `flayout/FFNaivInsp.cc`, `flayout/FFOrthoInsp.cc`, `flayout/FFSpringInsp.cc`, `flayout/FFSugiInsp.cc`, `include/ffgraph/FFTclBasicWindow.h`, `include/ffgraph/FFTclInspector.h`, `include/ffgraph/FFTclSubWindow.h`, `include/ffgraph/FFTclWindow.h`, `tests/test_label_replacement.cpp`, `tests/test_tcl_command_lifetime.cpp`.

## Applying or validating a historical patch

These records are for provenance. If using an old patch, first confirm its baseline and whether equivalent changes are already present:

```sh
git status --short
git apply --check /path/to/ffgraph-priority2-pass4-tcl-label-ownership.patch
# Only if the check succeeds and these changes are not already applied:
git apply /path/to/ffgraph-priority2-pass4-tcl-label-ownership.patch
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
```

Do not replay historical patches blindly on a current Pass 8/9 checkout. Use `git log`, `git show`, and source comparison to determine what is already merged.
