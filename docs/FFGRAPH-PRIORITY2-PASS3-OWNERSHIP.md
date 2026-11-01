# ffgraph — Priority 2 Pass 3 — graph-handle self-assignment and simple leaks

**Historical artifact:** `ffgraph-priority2-pass3-ownership.patch`
**Category:** Priority 2 / Pass 3
**Artifact availability:** retained in the prior conversation; not bundled with this docs-only patch.

> **Historical retrospective — prepared 1 November 2026.** This note documents a patch produced in the earlier ffgraph uplift conversation. It is not a new code change or evidence of fresh execution of that patch. File paths and historical verification are reconstructed from the retained patch/archive and conversation; the current checkout may contain subsequent fixes (including Passes 8 and 9).

## Defects addressed

- `FFGraph::assign()` could release the last reference to its current graph before acquiring the replacement, including `graph = graph`. The new implementation handles identical handles and acquires the new ownership *before* releasing the old one; it preserves lazy construction behaviour.
- `FFTclGraphInsp.cc`: free the previous dynamically allocated label-type string before replacing it in graph, node and edge inspector paths.
- `FFTclSubWindow.cc`: avoid leaking a `strdup`-allocated Tcl command name; pass a valid `String::c_str()` to `Tcl_CreateCommand` instead.
- `FFWindow.cc`: remove unnecessary `strdup` allocations when setting Tcl geometry and invoking `Tcl_EvalFile`.

## Added regression coverage

`tests/test_graph_ownership.cpp` exercises sole-owner self-assignment, reassignment between shared handles, reassignment between different graphs, and lazy graph creation. The test is registered in the `BUILD_TESTING` section of `CMakeLists.txt` and links real graph/library targets with static-archive rescanning.

## Verification boundary

The assistant reported that nine existing standalone tests passed and the patch applied to the relevant baseline. The new graph-ownership test and full application still required Ubuntu compilation at issue time. The user continued to Pass 4 without posting an explicit separate Pass 3 CTest transcript.

## Deferred work

This patch does **not** attempt to fix Tcl command unregistering, interpreter destruction or graph/label ownership cycles; those were handled in Passes 4–7. Keep the tests to prevent self-assignment regressions.

## Patch file inventory

`CMakeLists.txt`, `fgraph/FFGraph.cc`, `fgraphic/FFTclGraphInsp.cc`, `fgraphic/FFTclSubWindow.cc`, `fgraphic/FFWindow.cc`, `tests/test_graph_ownership.cpp`.

## Applying or validating a historical patch

These records are for provenance. If using an old patch, first confirm its baseline and whether equivalent changes are already present:

```sh
git status --short
git apply --check /path/to/ffgraph-priority2-pass3-ownership.patch
# Only if the check succeeds and these changes are not already applied:
git apply /path/to/ffgraph-priority2-pass3-ownership.patch
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
```

Do not replay historical patches blindly on a current Pass 8/9 checkout. Use `git log`, `git show`, and source comparison to determine what is already merged.
