# ffgraph — Priority 2 Pass 1 — standard containers and validated CLI parsing

**Historical artifact:** `ffgraph-priority2-pass1.patch`
**Category:** Priority 2 / Pass 1
**Artifact availability:** retained in the prior conversation; not bundled with this docs-only patch.

> **Historical retrospective — prepared 1 November 2026.** This note documents a patch produced in the earlier ffgraph uplift conversation. It is not a new code change or evidence of fresh execution of that patch. File paths and historical verification are reconstructed from the retained patch/archive and conversation; the current checkout may contain subsequent fixes (including Passes 8 and 9).

## Changes

- `ffmain.cc`: replace the GNU C++ variable-length array `FFGraph tmpG[argc]` with a `std::vector<FFGraph>`; preserve the legacy convention in which a *fully numeric positive* first argument selects empty-window mode. `std::strtol`/`errno`/end-pointer validation avoids treating filenames such as `2026.graph` as mode numbers.
- `flayout/FFOrtho.cc`: replace raw dynamic allocation for a node-pointer array with `std::vector`; replace the `qsort` call and incompatible comparator cast with a typed `std::sort` lambda over node coordinates.
- No graph formats, Tcl/Tk callbacks, parser generators, or layout algorithm designs were intentionally changed.

## Build correction required

The initial patch accidentally retained `knoten = 0;` after converting `knoten` to `std::vector<FFNode>`, which cannot be assigned an integer. The user reported the compiler error at `FFOrtho.cc:382`. The separate `ffgraph-priority2-pass1-fix.patch` removes this leftover line. **Both patches are needed if applying the historical sequence.** See `FFGRAPH-PRIORITY2-PASS1-CORRECTION.md`.

## Verification

The assistant reported verifying the initial patch's application, but not a full local GUI build because Tk development files were missing. After the correction, the user confirmed the build passed. Test outcomes are limited to those user reports; no new Pass 1 unit test was introduced.

## Targeted regression checks

Open multiple files from the command line, a filename beginning with digits, and exercise the orthogonal layout on an ordinary graph and an empty graph. Run standard CTest and ASan presets where supported.

## Patch file inventory

`ffmain.cc`, `flayout/FFOrtho.cc`.

## Applying or validating a historical patch

These records are for provenance. If using an old patch, first confirm its baseline and whether equivalent changes are already present:

```sh
git status --short
git apply --check /path/to/ffgraph-priority2-pass1.patch
# Only if the check succeeds and these changes are not already applied:
git apply /path/to/ffgraph-priority2-pass1.patch
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
```

Do not replay historical patches blindly on a current Pass 8/9 checkout. Use `git log`, `git show`, and source comparison to determine what is already merged.
