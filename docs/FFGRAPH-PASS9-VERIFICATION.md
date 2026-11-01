# ffgraph Pass 9 — graph invariants, copy semantics, and headless geometry/layout regressions

**Date:** 1 November 2026
**Patch baseline:** `ffgraph-current(1).zip` + `pass8-container-correctness.patch` + `ffgraph-pass8-followup.patch`, in that order.
**Scope:** Incremental Pass 9 checkpoint; not a complete graph/layout audit.

## Changes

1. **`FFGraphClass::copy()` no longer overwrites the source graph's node or edge values.** Previously it assigned sequential integers to every source node and edge while constructing a copy, and also reset destination node values in the copy-into-existing-graph case. Both practices are removed. An `std::unordered_map<FFNode,FFNode>` now tracks source-to-copy identity directly. Existing node/edge data is copied by the existing `copyDataFrom` methods, including `value` fields. The map also correctly supports subsets, parallel edges, self-loops, start/end node designations and independent ownership of the copy. The existing `FFGraph` reference-counting model remains unchanged.
2. **Reject invalid graph members without dereferencing them.** `addEdge`, `removeEdge`, `removeNode`, `addEndNode` and `setEndNodes` now verify membership through their owning graph's node/edge set, rather than reading a field from an unverified pointer. Null and foreign members are handled without invalid pointer dereferences. Existing error-handler messages for invalid add/remove operations are retained. As with the legacy pointer collections, a *freed pointer whose address has been reused for a new object* cannot be distinguished solely by its address; this patch does not introduce generation-counted handles.
3. **Added three regression tests** and registered them in the existing `BUILD_TESTING` CMake/CTest block:
   - `test_graph_invariants_pass9`: checks graph incidence on each stored edge/node, count consistency, start/end membership, destination independence, source and copied edge/node values, subset copy, deletion of incident edges, null/foreign handling, endpoint retargeting/reversal, and 500 seeded graph-mutation steps.
   - `test_graph_geometry_pass9`: creates real graphic node/edge labels without a window and repeats insert/delete/straighten/reverse on 3 routes (including loops and coincident endpoints), checking point counts and finite coordinates.
   - `test_graph_layout_pass9`: invokes the **real** spring embedder with a bounded iteration count and a headless subclass that only suppresses Tk inspector creation, not the algorithm. Exercises 0, 1, 2, 4 and 6-node fixtures, including a cycle, parallel edges and a loop; checks coordinate finiteness and that edge routes are straightened. This includes the historic `FFGraphClass::straightenEdges()` / `MFPlex<FFPoint>::length()` path.

## Reproductions against the previous baseline

- The new invariant test failed before the fix with `graph invariant: copy must not renumber source nodes` (exit 134).
- A standalone baseline reproduction invoking `addEdge(nullptr, validNode)` crashed with SIGSEGV (exit 139). The patched implementation returns `nullptr` and invokes the existing error handler.

## Tests actually completed in this environment

Environment: Debian GNU/Linux 13, GCC 14.2.0, C++17, CMake 3.31/Ninja, supplied public Tcl/Tk 8.6 headers, installed Tcl/Tk 8.6 and X11 runtime libraries.

| Build | Completed verification | Results |
|---|---|---|
| Debug, `-g`, no optimisation | Separate headless CMake build, `ctest --output-on-failure` | **7/7 passed** |
| Optimised, `-O1 -DNDEBUG` | Separate headless CMake build, `ctest --output-on-failure` | **7/7 passed**; existing tests' `assert` checks may be disabled by `NDEBUG`; new tests use an always-on check function |
| ASan + UBSan, `-O0 -g1` | Separate headless CMake build, `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ctest --output-on-failure` | **7/7 passed**, no reported sanitizer errors |

All three runs covered the **three new tests** plus existing `test_graph_ownership`, `test_graph_reference_cycle`, `test_graph_label_initialization` and `test_label_replacement`.

**Important verification boundary:** The above tests used an **external, temporary headless test-driver CMakeLists**, not the project's complete original CMake build. The test driver compiled the real graph, layout, container and graphic C++ sources and linked the system Tcl/Tk runtimes; it intentionally skipped an obsolete, non-building `FFGeomWindow.cc` and `FFGraphContainer.cc` (also absent from the production CMake source lists) and omitted the Xpm image-loader C file. A test-only `Tcl_AppInit` linkage shim existed *outside the project patch*, was never called by these tests, and enabled linking otherwise headless paths; no GUI behaviour was tested through that shim.

The normal `cmake --preset debug` project configuration still cannot complete in this environment: the `tk.pc` development pkg-config file is missing, as are `X11/xpm.h` and the unversioned Xpm development link file. The supplied Tcl/Tk headers alone do not provide these. Debian package-network access was also unavailable. **No normal application build, normal-project full CTest, real GUI/Tk/Xvfb lifecycle tests, install staging or interactive layout session is claimed.**

## Verification on Ubuntu 24.04

From the Git checkout matching the uploaded baseline and with both Pass 8 patches applied:

```bash
# Apply the Pass 9 patch as a separate commit.
git apply --check pass9-graph-invariants.patch
git apply pass9-graph-invariants.patch

# Install development dependencies as in BUILD-CMAKE.md if necessary.
cmake --preset debug
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure

cmake --preset release
cmake --build --preset release --parallel
ctest --test-dir build/release --output-on-failure

cmake --preset asan
cmake --build --preset asan --parallel
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  ctest --preset asan --output-on-failure
```

Confirm the three newly registered tests run under each preset and that the existing GUI teardown tests still pass under Xvfb. Run a representative sgraph/GML load-layout-save smoke test in the real application before merging.

## Deliberately deferred / residual concerns

- The old Valgrind report at `MFPlex<FFPoint>::length()` has **not** been reproduced with an original triggering graph file; a subset of the implicated edge-point path passed under ASan. Further end-to-end layout/GUI input fixtures remain necessary.
- Node/edge label callbacks can trigger reentrant structural changes. Reentrant mutation and strong exception guarantees for all graph construction and copying paths have not been fully audited.
- Some other operations (`save`, `load`, subgraph construction, algorithm numbering) still deliberately overwrite `value` fields for historical file-format or algorithm compatibility. This patch changes **only copying**.
- Layout geometry and coordinates are not proven correct for every graph topology. The spring test is small and bounded, not a performance or convergence certification.
- The inspector/WSLg issue stays parked, SPQR stays excluded, and Tcl/Tk GUI source and parser/file-format implementations are unchanged.

**Recommended continuation:** Expand the end-to-end invariant/layout fixture suite with real user-provided graphs, especially the historical spring-embedder crash case; run the complete Ubuntu CMake/CTest/ASan/Xvfb matrix; then proceed into Pass 10 end-to-end memory-safety and lifecycle testing.
