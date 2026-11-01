# ffgraph — Inspector visibility fix — distinct from the parked WSLg issue

**Historical artifact:** `ffgraph-inspector-visibility-fix.patch`
**Category:** GUI follow-up between Passes 6 and 7
**Artifact availability:** retained in the prior conversation; not bundled with this docs-only patch.

> **Historical retrospective — prepared 1 November 2026.** This note documents a patch produced in the earlier ffgraph uplift conversation. It is not a new code change or evidence of fresh execution of that patch. File paths and historical verification are reconstructed from the retained patch/archive and conversation; the current checkout may contain subsequent fixes (including Passes 8 and 9).

## Trigger and initial hypothesis

The user reported that an inspector window could disappear and resist reopening through the Options menu even though ffgraph did not crash. Initial review found `FFTclInspector::show()` used `iconify()` followed by `deiconify()`, and reopening rebuilt menus. A withdrawn Tk window can also survive as a Tcl interpreter after its root is destroyed.

## Original patch changes

- `FFTclInspector::show()`: check that the Tcl interpreter still owns a Tk root, then call `deiconify` and `raise` instead of an `iconify/deiconify` round-trip.
- `FFTclWindow::showInspector()`: discard/recreate an inspector whose Tk root no longer exists; otherwise reuse it and update menus.
- Added `FFTclInspector::hasLiveWindow()` declaration/definition.
- Added `tests/test_inspector_visibility.cpp` and conditional `xvfb-run` CTest registration.

## Evidence and why the incident is *not* considered fixed

The assistant reported compiling the modified units and running the new test under Xvfb. The user later reported that opening/closing through either double-click or the menu could intermittently leave the inspector hidden or sluggish; restoring from the Windows taskbar sometimes worked. The environment is Windows/WSL2 using WSLg 1.0.79 and WSL 3.0.3.0. Separate Tcl/Tk tests showed that `wm withdraw` could return in ~0–2 ms while the window disappeared perceptibly later; `wm iconify; wm deiconify; raise` occasionally blocked for ~2,000 ms. These observations support WSLg/Tk synchronisation delays but do not prove the ffgraph failure's root cause.

## Decision and follow-up

**Inspector/WSLg issue explicitly parked.** The separate handover document `docs/ffgraph-inspector-wslg-investigation-handover.md` (if present) preserves test output and recommended diagnostics: instrument show/withdraw timing, Tk state/event transitions, nested Tcl `update`, and menu-building overhead; compare native Linux/Xvfb versus WSLg. Do not conflate this with interpreter-ownership work.

The later Pass 7 patch modifies the inspector test to honour deferred interpreter destruction.

## Patch file inventory

`CMakeLists.txt`, `fgraphic/FFTclInspector.cc`, `fgraphic/FFTclWindow.cc`, `include/ffgraph/FFTclInspector.h`, `tests/test_inspector_visibility.cpp`.

## Applying or validating a historical patch

These records are for provenance. If using an old patch, first confirm its baseline and whether equivalent changes are already present:

```sh
git status --short
git apply --check /path/to/ffgraph-inspector-visibility-fix.patch
# Only if the check succeeds and these changes are not already applied:
git apply /path/to/ffgraph-inspector-visibility-fix.patch
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
```

Do not replay historical patches blindly on a current Pass 8/9 checkout. Use `git log`, `git show`, and source comparison to determine what is already merged.
