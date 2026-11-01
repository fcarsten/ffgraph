# ffgraph — Tcl compatibility integration test restoration

**Historical artifact:** `ffgraph-tcl-test.patch`
**Category:** Build/test correction
**Artifact availability:** retained in the prior conversation; not bundled with this docs-only patch.

> **Historical retrospective — prepared 1 November 2026.** This note documents a patch produced in the earlier ffgraph uplift conversation. It is not a new code change or evidence of fresh execution of that patch. File paths and historical verification are reconstructed from the retained patch/archive and conversation; the current checkout may contain subsequent fixes (including Passes 8 and 9).

## Problem reproduced by the user

The migrated CMake test target `test_tcl_compat` failed to compile against installed Tcl 8.6 because its old test source defined fake Tcl internals. The mock assumed `Tcl_Obj::refs` and `Tcl_Obj::backing`, assigned `const char*` into `Tcl_Obj::bytes`, and attempted to define symbols (`Tcl_IncrRefCount`, `Tcl_DecrRefCount`) that Tcl supplies as macros.

An interim suggestion to use `tests/tcl_stub/tcl.h` could not work: that directory/header did not exist. The target was temporarily disabled until the real-Tcl test was prepared.

## Source change

`tests/test_tcl_compat.cpp` was rewritten to use the *real* `Tcl_CreateInterp()` and `Tcl_DeleteInterp()` APIs and to call the project's `FF_TclEvalObjv()` wrapper. The test checks:

- Tcl command arguments containing spaces, semicolons, square brackets, braces and dollar signs remain literal data rather than script substitutions.
- A `set` command stores the exact input, and a `list` command preserves argument boundaries and result values.
- Tcl object lifetimes use the actual Tcl 8.6 interface rather than a manually duplicated struct.

The patch changes only this test source. The test target must include Tcl/Tk headers and link `PkgConfig::TK` (or its correct Tcl target) in CMake; unlike the old mock, it **must not** be compiled without the Tcl API.

## Verification

The user confirmed that the restored Tcl test compiles and passes under Ubuntu after applying the patch and CMake target correction. Independent local coverage and specific sanitizer results were not stated for this exact patch.

## Follow-up

Keep this test in normal CTest and CI. Tcl GUI lifecycle and callback ownership were addressed separately in Priority 2 Passes 4, 6 and 7.

## Patch file inventory

`tests/test_tcl_compat.cpp`.

## Applying or validating a historical patch

These records are for provenance. If using an old patch, first confirm its baseline and whether equivalent changes are already present:

```sh
git status --short
git apply --check /path/to/ffgraph-tcl-test.patch
# Only if the check succeeds and these changes are not already applied:
git apply /path/to/ffgraph-tcl-test.patch
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
```

Do not replay historical patches blindly on a current Pass 8/9 checkout. Use `git log`, `git show`, and source comparison to determine what is already merged.
