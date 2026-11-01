# ffgraph — Development handover and dependency snapshot documentation

**Historical artifact:** `ffgraph-handover-docs-reissued.patch`
**Category:** Documentation-only patch
**Artifact availability:** retained in the prior conversation; not bundled with this docs-only patch.

> **Historical retrospective — prepared 1 November 2026.** This note documents a patch produced in the earlier ffgraph uplift conversation. It is not a new code change or evidence of fresh execution of that patch. File paths and historical verification are reconstructed from the retained patch/archive and conversation; the current checkout may contain subsequent fixes (including Passes 8 and 9).

## Purpose

Prepare a fresh chat/session handover for the remaining development passes and document third-party build and runtime dependencies without changing application code.

## Files added

- `docs/FFGRAPH-NEXT-SESSION-PLAN.md`: sequence and acceptance criteria for Pass 8 (container compatibility), Pass 9 (graph integrity), Pass 10 (ASan/UBSan and GUI lifecycle), Pass 11 (parser/scanner modernisation), Pass 12 (portability/performance/final documentation). Records parked WSLg and excluded SPQR.
- `docs/FFGRAPH-DEPENDENCIES.md`: Debian/Ubuntu build prerequisites, toolchain and runtime dependencies, distinction between CMake declarations and actual machine snapshots, CMake presets, installation, and current limitations.
- `docs/ffgraph-inspector-wslg-investigation-handover.md`: snapshot of the unresolved WSLg inspector investigation.
- `scripts/capture-build-environment.sh`: captures toolchain, installed apt package versions, CMake findings and linked-library dependencies from a working machine for reproducibility.

## Patch history

The same documentation payload was distributed as a ZIP, and the patch was later **reissued** after the user encountered a download error. `ffgraph-handover-docs-reissued.patch` is the available reissued patch. Do not apply both the original and reissued copies; they add the same paths. A ZIP wrapper was provided as an alternate download transport.

## Verification and boundaries

The assistant reported a shell syntax/execution check for the environment-capture script and patch applicability. The dependency inventory describes the then-known baseline; it does not guarantee the machine versions or current repository state. The user may have since updated these documents in a different chat, so avoid overwriting newer handover and dependency notes when applying archival patches.

## Suggested ongoing process

Keep CMake as the real dependency declaration, track tested Ubuntu package installation commands, run the capture script on successful reference builds, and use CI logs/release artifacts to preserve reproducible environment snapshots without pinning outdated package versions in ordinary documentation.

## Patch file inventory

`docs/FFGRAPH-DEPENDENCIES.md`, `docs/FFGRAPH-NEXT-SESSION-PLAN.md`, `...fgraph-inspector-wslg-investigation-handover.md`, `scripts/capture-build-environment.sh`.

## Applying or validating a historical patch

These records are for provenance. If using an old patch, first confirm its baseline and whether equivalent changes are already present:

```sh
git status --short
git apply --check /path/to/ffgraph-handover-docs-reissued.patch
# Only if the check succeeds and these changes are not already applied:
git apply /path/to/ffgraph-handover-docs-reissued.patch
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
```

Do not replay historical patches blindly on a current Pass 8/9 checkout. Use `git log`, `git show`, and source comparison to determine what is already merged.
