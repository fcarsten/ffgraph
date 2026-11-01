# ffgraph — Priority 2 Pass 5 — graph-label and inspector reference cycles

**Historical artifact:** `ffgraph-priority2-pass5-graph-reference-cycles.patch`
**Category:** Priority 2 / Pass 5
**Artifact availability:** retained in the prior conversation; not bundled with this docs-only patch.

> **Historical retrospective — prepared 1 November 2026.** This note documents a patch produced in the earlier ffgraph uplift conversation. It is not a new code change or evidence of fresh execution of that patch. File paths and historical verification are reconstructed from the retained patch/archive and conversation; the current checkout may contain subsequent fixes (including Passes 8 and 9).

## Defect and design decision

`FFGraphClass` owns its labels, while an `FFGraphLabel` previously retained an *owning* `FFGraph` handle back to its parent. This circular reference could prevent destruction after the last external handle was released. `FFTclGraphInsp` had another internal owning handle to the inspected graph.

## Implemented changes

- Introduced `FFGraphObserver` as a non-owning `FFGraph`-compatible observer in `include/ffgraph/FFGraph.h`.
- Changed `FFGraphLabel` and `FFTclGraphInsp` to retain observers instead of owning graph handles. External `getGraph()` callers still receive a normal owning handle.
- Protected `FFGraphClass` construction/destruction around temporary owning handles created by label `init`/destruction paths; avoid refcount-induced recursive destruction.
- Added `tests/test_graph_reference_cycle.cpp` and its CMake/CTest target; verifies last-owner destruction, copied-out owning handles, default labels, copying, and temporary handles during teardown.

## Evidence

The assistant reported passing the focused reference-cycle test, the earlier graph-ownership/label/Tcl tests, C++17 syntax/translation-unit compilation, and patch applicability. The assistant could not finish the complete Xpm-dependent CMake application build. The user subsequently reported that the program continued to work.

## Important contract

This is an **internal observer** scheme: do not allow observer references to outlive their parent graph. A returned ordinary `FFGraph` owns a reference, but an internal `FFGraphObserver` does not. Later Pass 6 initializes replacement graph labels so their observers are valid.

## Patch file inventory

`CMakeLists.txt`, `fgraph/FFGraphClass.cc`, `fgraphic/FFTclGraphInsp.cc`, `include/ffgraph/FFGraph.h`, `include/ffgraph/FFGraphLabel.h`, `include/ffgraph/FFTclGraphInsp.h`, `tests/test_graph_reference_cycle.cpp`.

## Applying or validating a historical patch

These records are for provenance. If using an old patch, first confirm its baseline and whether equivalent changes are already present:

```sh
git status --short
git apply --check /path/to/ffgraph-priority2-pass5-graph-reference-cycles.patch
# Only if the check succeeds and these changes are not already applied:
git apply /path/to/ffgraph-priority2-pass5-graph-reference-cycles.patch
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
```

Do not replay historical patches blindly on a current Pass 8/9 checkout. Use `git log`, `git show`, and source comparison to determine what is already merged.
