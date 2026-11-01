# ffgraph — Priority 2 Pass 1 correction — remove invalid vector assignment

**Historical artifact:** `ffgraph-priority2-pass1-fix.patch`
**Category:** Priority 2 / Pass 1 hotfix
**Artifact availability:** retained in the prior conversation; not bundled with this docs-only patch.

> **Historical retrospective — prepared 1 November 2026.** This note documents a patch produced in the earlier ffgraph uplift conversation. It is not a new code change or evidence of fresh execution of that patch. File paths and historical verification are reconstructed from the retained patch/archive and conversation; the current checkout may contain subsequent fixes (including Passes 8 and 9).

## Why the hotfix exists

After replacing a raw `FFNode` pointer array with `std::vector<FFNode>`, the initial Pass 1 patch left `knoten = 0;` at the end of `FFOrtho::swapBadies()`. GCC correctly rejected `operator=(int)` for `std::vector<FFNodeClass*>`.

## Exact change

The one-file patch changes `flayout/FFOrtho.cc`, removing the invalid assignment and leaving a comment that the vector's lifetime ends automatically on function return. No runtime semantics or ownership policy are otherwise altered.

## Evidence

The user applied the correction and reported that the CMake build passed. This patch is **dependent on** `ffgraph-priority2-pass1.patch`; do not apply it to a source tree that still uses the raw pointer array.

## Follow-up

The correction should be kept together with the initial Pass 1 change in a squashed migration if rebuilding the history, but remains documented separately for traceability.

## Patch file inventory

`flayout/FFOrtho.cc`.

## Applying or validating a historical patch

These records are for provenance. If using an old patch, first confirm its baseline and whether equivalent changes are already present:

```sh
git status --short
git apply --check /path/to/ffgraph-priority2-pass1-fix.patch
# Only if the check succeeds and these changes are not already applied:
git apply /path/to/ffgraph-priority2-pass1-fix.patch
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
```

Do not replay historical patches blindly on a current Pass 8/9 checkout. Use `git log`, `git show`, and source comparison to determine what is already merged.
