# File selector ownership and activation — 8 November 2026

## Problem and root cause

The release-candidate desktop tests reported an invisible selector on Windows
and an initial disappear/reappear on Linux. The Windows symptom affects both
File-menu actions and keyboard shortcuts. Investigation started from clean
`main` at `1c178e4` (1.0.0-rc1 preparation).

The selector was an independent top-level window. It had no `wm transient`
relationship to its graph window and did not explicitly focus its filename
entry. Raising it once did not keep it above subsequent owner activation,
including activation/focus changes around native menu dismissal. Establishing
ownership while hidden also avoids changing window-manager ownership after
presentation, which can require remapping on some desktops.

The earlier mapping regression opened the Tcl procedure directly under bare
Xvfb. That passed before this patch, but only proved that the selector was
mapped, not that it remained above its graph. Both a Linux/Openbox probe and
native Windows Tk under Wine/Openbox reproduced the stacking defect: after
raising the graph, `wm stackorder .fileselect isabove .` returned **0**, while
the selector still existed and was mapped. With the fix it returns **1** on
both platforms. The expanded permanent regression failed before the fix with
`File selector has no graph-window owner`.

The reported exact Linux desktop animation was not reproduced under Openbox;
the concrete stacking defect is demonstrated. Final Windows 11 and the user's
Linux desktop confirmation remain separate from this evidence. The release
packaging commit did not modify the selector's Tcl implementation.

## Files and implementation changes

- `lib/ffgraph/tcl/fileselect.tcl`: set `wm transient .fileselect .` while the
  selector is withdrawn, before scanning/preparing its contents. Keep the
  existing prepare-hidden/deiconify-once flow, focus the filename entry, and
  restore the previous widget focus on completion. Handle `WM_DELETE_WINDOW`
  by setting the existing dismiss state, so closing the title bar cancels the
  response loop and keeps the selector reusable instead of destroying widgets
  while `tkwait variable` continues waiting.
- `tests/test_file_selector.cpp`: check ownership, filename focus and focus
  restoration, while preserving ready-map and navigation/menu checks. Exercise
  real Load and Save As menu callbacks through the C++ `fsBox()` bridge in both
  2D/3D, including title-bar cancellation and reopening. Simulate a WM protocol
  in global Tcl scope, as Tk does. Apply the existing five-second watchdog to
  each opening; the test now opens four dialogs per projection instead of two.
  The overall CTest timeout is unchanged.

No parser, graph/container ownership, layout, experimental SPQR or parked WSLg
inspector changes were made. Restart applications or refresh installed Tcl
resources to load the fix.

## Tests actually executed and results

- Original selector test before changes: passed, demonstrating the coverage gap.
- Expanded ownership regression against old Tcl: failed as expected; output is
  in `build/debug/selector-owner-before.log`.
- Managed Linux/Openbox and Windows/Wine/Openbox stacking probes: selector was
  below the raised graph before the fix and above it afterward.
- Full Linux Debug CTest: **99/99 passed**.
- Full Linux Release CTest: **101/101 passed**, including configured external
  Rome corpus tests.
- Final focused Debug/Release selector checks: passed after applying the
  per-opening watchdog to the expanded test.
- Windows Release cross-build focused selector check under Wine/Xvfb: passed
  in both projections, including the C++ File-menu callbacks and cancellation.
- Focused ASan/UBSan selector regression: passed under the existing GUI runtime
  preset, with leak detection OFF. No ownership changes require new memory
  sanitizer guarantees; strict toolkit-leak diagnostics were not rerun.

An early close-event test incorrectly evaluated the WM protocol in a local Tcl
procedure scope. That wrote a local array instead of the global response state;
it was corrected to use `uplevel #0`. The expanded Windows test also exceeded
the old shared five-second deadline across four openings; it now retains that
deadline independently for each opening. No failing production test was disabled.

## Build and verification commands

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure --parallel 2
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release --output-on-failure --parallel 2
cmake --preset asan
cmake --build --preset asan --target test_file_selector --parallel 2
ctest --preset asan-gui -R '^test_file_selector$' --output-on-failure
cmake -S . -B build/windows-rc1
cmake --build build/windows-rc1 --target test_file_selector ffgraph --parallel 2
xvfb-run -a ctest --test-dir build/windows-rc1 -R '^test_file_selector$' --output-on-failure
python3 scripts/package-release.py --build build/windows-rc1 --output build/rc1-artifacts/windows --preview
git diff --check
```

Full-suite logs are `build/{debug,release}/selector-owned-ctest.log`.
The private probes are `/tmp/ffgraph-selector-probe.cpp`,
`/tmp/ffgraph-selector-owner.tcl`, `/tmp/ffgraph-selector-wm-run.py` and
`/tmp/ffgraph-selector-pointer.py`. Openbox and its libraries were extracted
under ignored `build/selector-wm`; no system package was installed. These are
local diagnostic tools, not application or test-suite dependencies.

## Unverified assumptions and remaining risks

- The exact Windows 11 invisible-dialog report and Linux desktop animation need
  confirmation on those desktops. Wine/Openbox proves the ownership/stacking
  correction and existing Xvfb tests prove mapping/interaction properties.
- A native-menu probe used real pointer/keyboard events on Linux. Windows
  reproduction additionally used native `SendInput` key events under Wine;
  this established entry into the actual C++ selector path, not Windows 11
  equivalence. Permanent Windows navigation tests still intercept native menu
  posting as documented in the previous selector reports.
- Managed Wine may focus the selector's top-level during initial activation;
  the entry focus request is verified by the permanent tests under bare Xvfb.
  Native desktop focus policy is not fully established by those tests.
- The fix retains existing directory scanning and modeless input behavior;
  it does not redesign filesystem navigation, drive handling or reentrancy.
- No commit, push, tag or publication was performed. Local ZIPs are previews
  using the existing MinGW64/MSVCRT cross-toolchain, not native UCRT64 CI builds.
