# ffgraph — CMake migration — initial build-system uplift

**Historical artifact:** `ffgraph-cmake-migration.patch`
**Category:** Build-system migration / initial patch
**Artifact availability:** retained in the prior conversation; not bundled with this docs-only patch.

> **Historical retrospective — prepared 1 November 2026.** This note documents a patch produced in the earlier ffgraph uplift conversation. It is not a new code change or evidence of fresh execution of that patch. File paths and historical verification are reconstructed from the retained patch/archive and conversation; the current checkout may contain subsequent fixes (including Passes 8 and 9).

## Purpose

Replace recursive legacy Makefile compilation with a CMake/CTest workflow while retaining the source hierarchy and Tcl/Tk GUI. The generated legacy parser/scanner C++ sources remain checked in and are compiled directly; **normal builds do not regenerate** them via obsolete bison++/flex++ tools.

## Changes in the original patch

- Added root `CMakeLists.txt`: four static targets (`FFcontainer`, `FFGraph`, `FFlayout`, `FFGraphic`), executable, Tcl/Tk discovery via pkg-config, X11 and Xpm discovery, installation rules, and CTest registration of the existing standalone tests.
- Added `CMakePresets.json` with `debug`, `release`, `asan` (ASan/UBSan), and `warnings` configurations, using Ninja.
- Added `BUILD-CMAKE.md` with Ubuntu package prerequisites, configure/build/test/install instructions, parser-generation limitations and runtime-resource notes.
- Updated `.gitignore` for `/build/` and `CMakeUserPresets.json`.
- Retained legacy Makefiles and configure scripts temporarily for transitional verification.

## Important integration corrections made interactively (not in this original patch)

1. The first CMake version gave `PkgConfig::TK` only to selected targets. Because legacy headers transitively include `tk.h`, the user added Tk include propagation in the common target loop. This fixed missing Tk headers under `/usr/include/tcl8.6` or `/usr/include/tcl`.
2. `test_tcl_compat` originally relied on a mocked Tcl structure. Attempting to compile it with real Tcl headers exposed incompatible fields/macros; the test was temporarily disabled, then replaced by the separate `ffgraph-tcl-test.patch` documented elsewhere.
3. Static libraries had circular cross-references. The user adopted CMake `$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>` to resolve `FFSubGraphLabel::getGraph()` and related symbols. This requires **CMake 3.24+** even though the original configuration declared 3.21.
4. The source list initially included incomplete SPQR sources; those were subsequently excluded by a separate corrected patch.
5. The user confirmed CMake builds, CTest and an install test, then retired legacy build files manually. The present source tree may therefore differ from the original migration patch.

## Verification and constraints

Original CMake configuration was not fully executable in the assistant environment (Tk and Xpm development dependencies were unavailable). Successful full builds, tests and installation were reported by the user after the follow-up fixes. The migration patch itself **is not** a representation of the final working CMake configuration.

## Useful commands on Ubuntu 24.04

```sh
sudo apt install build-essential cmake ninja-build pkg-config tcl-dev tk-dev libx11-dev libxpm-dev xvfb xauth
cmake --preset debug
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
```

Preserve checked-in generated parser sources and Tcl/Xpm runtime resources. For dependency inventory, see `FFGRAPH-DEPENDENCIES.md` if present in the repository.

## Patch file inventory

`.gitignore`, `BUILD-CMAKE.md`, `CMakeLists.txt`, `CMakePresets.json`.

## Applying or validating a historical patch

These records are for provenance. If using an old patch, first confirm its baseline and whether equivalent changes are already present:

```sh
git status --short
git apply --check /path/to/ffgraph-cmake-migration.patch
# Only if the check succeeds and these changes are not already applied:
git apply /path/to/ffgraph-cmake-migration.patch
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
```

Do not replay historical patches blindly on a current Pass 8/9 checkout. Use `git log`, `git show`, and source comparison to determine what is already merged.
