# ffgraph — Ubuntu 24.04 GitHub Actions CI bootstrap

**Historical artifact:** `ffgraph-github-actions-ci.patch`
**Category:** Continuous integration
**Artifact availability:** retained in the prior conversation; not bundled with this docs-only patch.

> **Historical retrospective — prepared 1 November 2026.** This note documents a patch produced in the earlier ffgraph uplift conversation. It is not a new code change or evidence of fresh execution of that patch. File paths and historical verification are reconstructed from the retained patch/archive and conversation; the current checkout may contain subsequent fixes (including Passes 8 and 9).

## Workflow added

`.github/workflows/ci.yml` defines a single `ubuntu-24.04` job named `Build and test`. Triggers are push, pull request and manual `workflow_dispatch`; token permission is `contents: read`.

The job:

1. Checks out the repository via `actions/checkout@v7` with `persist-credentials: false`.
2. Installs `build-essential`, `cmake`, `ninja-build`, `pkg-config`, Tcl/Tk development packages, X11/Xpm development packages, `xvfb` and `xauth`.
3. Configures CMake using `debug` and `-DBUILD_TESTING=ON`.
4. Builds with bounded parallelism (`--parallel 2`).
5. Executes `ctest --preset debug --output-on-failure --parallel 2`, including any Xvfb tests registered by CMake.

## Verification and rollout

The assistant reported YAML syntax/structure and patch-application checks against an uploaded source archive. **A real GitHub-hosted workflow execution was not verified in this chat.** The user was advised to push the patch and review GitHub Actions results.

## Constraints and future improvements

This is deliberately a single **Debug** job, not a Release/ASan matrix. It assumes repository CMake presets, CI-installed Tk/Xpm dependencies and conditional Xvfb test registration are current. If a newer CMake or dependency requirement is introduced, update the runner/job as needed. An ASan/UBSan job, test artifacts and compiler-version matrix are possible follow-ups, but not part of this patch.

## Patch file inventory

`.github/workflows/ci.yml`.

## Applying or validating a historical patch

These records are for provenance. If using an old patch, first confirm its baseline and whether equivalent changes are already present:

```sh
git status --short
git apply --check /path/to/ffgraph-github-actions-ci.patch
# Only if the check succeeds and these changes are not already applied:
git apply /path/to/ffgraph-github-actions-ci.patch
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
```

Do not replay historical patches blindly on a current Pass 8/9 checkout. Use `git log`, `git show`, and source comparison to determine what is already merged.
