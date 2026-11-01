# ffgraph — Priority 2 Pass 6 — replacement-label initialization and early Tcl cleanup

**Historical artifact:** `ffgraph-priority2-pass6-label-and-tcl-lifecycle.patch`
**Category:** Priority 2 / Pass 6
**Artifact availability:** retained in the prior conversation; not bundled with this docs-only patch.

> **Historical retrospective — prepared 1 November 2026.** This note documents a patch produced in the earlier ffgraph uplift conversation. It is not a new code change or evidence of fresh execution of that patch. File paths and historical verification are reconstructed from the retained patch/archive and conversation; the current checkout may contain subsequent fixes (including Passes 8 and 9).

## Graph-label fix

`FFGraphClass::setGraphLabel()` previously transferred/replaced a label without consistently invoking its `init()` callback. This mattered after Pass 5, because the new non-owning graph observer must be attached to the owner before it is used. Both index-based and type-based replacements now initialise the incoming label while preserving self-replacement guards.

## Initial Tcl cleanup fix

`FFWindow` creates an interpreter before its Tk root is initialised. Destroying a never-Tk-initialised `FFWindow` previously leaked that interpreter; the destructor now deletes this safe category of interpreter. **This pass deliberately does not enable immediate `Tcl_DeleteInterp()` for windows with an active Tk root.**

## Regression tests

- `tests/test_graph_label_initialization.cpp`: ensures the replacement `init()` is invoked, `getGraph()` resolves the parent, self-replacement doesn't reinitialise, and graph-owned label containers are handled safely.
- `tests/test_ffwindow_interpreter_lifetime.cpp`: uses a Tcl interpreter deletion callback to verify deletion when the `FFWindow` never initialises Tk.

Both tests are registered in CMake/CTest.

## Verification

The assistant reported passing both focused tests, existing ownership/reference-cycle tests and C++ translation-unit compilation; no complete display-backed application lifecycle run. The user reported the updated program appeared to work, but noted intermittent inspector disappearance. That inspector issue was investigated separately and not treated as evidence of this patch's cause.

## Subsequent work

Pass 7 handles safe teardown of Tk-initialised interpreters using event-loop deferred cleanup; do not substitute the early-only destructor approach for the later lifecycle design.

## Patch file inventory

`CMakeLists.txt`, `fgraph/FFGraphClass.cc`, `fgraphic/FFWindow.cc`, `tests/test_ffwindow_interpreter_lifetime.cpp`, `tests/test_graph_label_initialization.cpp`.

## Applying or validating a historical patch

These records are for provenance. If using an old patch, first confirm its baseline and whether equivalent changes are already present:

```sh
git status --short
git apply --check /path/to/ffgraph-priority2-pass6-label-and-tcl-lifecycle.patch
# Only if the check succeeds and these changes are not already applied:
git apply /path/to/ffgraph-priority2-pass6-label-and-tcl-lifecycle.patch
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
```

Do not replay historical patches blindly on a current Pass 8/9 checkout. Use `git log`, `git show`, and source comparison to determine what is already merged.
