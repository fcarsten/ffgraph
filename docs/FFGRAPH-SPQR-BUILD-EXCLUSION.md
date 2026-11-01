# ffgraph — Exclude incomplete SPQR algorithms from the default build

**Historical artifact:** `ffgraph-spqr-exclusion-corrected.patch`
**Category:** Build-scope change
**Artifact availability:** retained in the prior conversation; not bundled with this docs-only patch.

> **Historical retrospective — prepared 1 November 2026.** This note documents a patch produced in the earlier ffgraph uplift conversation. It is not a new code change or evidence of fresh execution of that patch. File paths and historical verification are reconstructed from the retained patch/archive and conversation; the current checkout may contain subsequent fixes (including Passes 8 and 9).

## Reason for exclusion

Two historical implementations, `flayout/FFSpqr.cc` and `flayout/FFSpqr.ok`, were compared. The `.ok` file appears to contain more advanced preliminary S/P transformations and pole-chain logic, but also has inconsistent declarations, unfinished branches and incomplete R-node handling. Neither was considered suitable for production use merely because compilation errors could be made to disappear.

## Exact patch effect

The **corrected** patch removes `FFSpqr.cc` and `FFSpqrNodeLabel.cc` from the CMake `FFlayout` source list. It intentionally **does not delete** those sources or `FFSpqr.ok`, alter SPQR headers, change Tcl/Tk behaviour, or add an optional target.

## Patch history warning

The first distributed file, `ffgraph-spqr-exclusion.patch`, was malformed (`git apply --check` reported `corrupt patch at line 15`). It is superseded by `ffgraph-spqr-exclusion-corrected.patch`; only the corrected one should be used. The user reported a successful build/test run afterwards.

## Deferred work

SPQR remains excluded and unfinished. If a future project needs SPQR, reconcile `FFSpqr.ok` and the header declarations, audit algorithm completeness separately, add fixtures for S/P/Q/R cases, and only then consider optional compilation. Do not silently re-add either source to the default library.

## Patch file inventory

`CMakeLists.txt`.

## Applying or validating a historical patch

These records are for provenance. If using an old patch, first confirm its baseline and whether equivalent changes are already present:

```sh
git status --short
git apply --check /path/to/ffgraph-spqr-exclusion-corrected.patch
# Only if the check succeeds and these changes are not already applied:
git apply /path/to/ffgraph-spqr-exclusion-corrected.patch
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
```

Do not replay historical patches blindly on a current Pass 8/9 checkout. Use `git log`, `git show`, and source comparison to determine what is already merged.
