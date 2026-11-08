# File selector navigation selection

## Problem and root cause

After removing the initialization-time popup, the navigation options appeared
on request but could not reliably be selected on Linux. The custom opener
posted the menu on button press and unconditionally unposted it on release.
It also called `grab` and `focus` directly without registering the popup with
Tk's menu lifecycle. Its apparent delayed grab actually executed immediately:
the `[grab $menu]` substitution ran while constructing the `after` argument.
Tk consequently lacked the popup state and saved grab/focus information used
by normal selection and dismissal bindings.

The earlier regression intercepted the requested `post` and verified startup,
but did not complete the pointer interaction. The expanded regression
reproduced `Releasing the navigation button hid the menu` before the selection
fix (`build/debug/file-selector-selection-before.log`).

## Files and implementation changes

- `lib/ffgraph/tcl/fileselect.tcl`: open the menu on the opener's button release
  and let `tk_popup` own selection, grab/focus management and dismissal. Remove
  direct posting, manual grab/focus changes and unconditional release-time
  unposting. Resolve the current entry by label, with an initial-entry fallback.
  Let Tk center that entry instead of using an unmapped menu's width/positions.
  The startup-post removal remains in place. Preserve all navigation commands.
- `tests/test_file_selector.cpp`: retain startup detection and Windows binding
  coverage, and add Linux pointer-event sequences in live 2D and 3D selectors.
  Click the opener's frame, label and arrow and select Home, Root and Current
  through menu motion, press and release events. Check directory changes,
  popup visibility, the global popup grab, and restoration of the previous
  local grab and focus. Test Escape and outside-click dismissal too.
  Background Tcl errors become test failures. The test widens the legacy
  dialog to make the arrow subwidget visible before exercising all surfaces.

The new regression remains registered in both full and initial suites by the
preceding startup patch's CMake changes. No parser, graph/container ownership,
layout, experimental SPQR or parked WSLg inspector changes are involved.

## Tests actually executed and results

- Expanded regression against the startup-only fix: **failed as expected**
  because releasing the opener hid its menu.
- Focused Debug regression after the selection fix: **passed**; actual Tk
  event handlers invoke all three navigation commands in both projections.

- Full Linux Debug suite: **93/93 passed**.
- Full Linux Release suite: **95/95 passed**.
- Native Windows cross-build selector regression under Wine/Xvfb: **passed**.
  This covers startup/binding/cancel/accept, not native menu option selection.
- `git diff --check`: **passed**.

## Build and verification commands

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug -R '^test_file_selector$' --output-on-failure
ctest --preset debug --output-on-failure --parallel 2

cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release --output-on-failure --parallel 2

cmake -S . -B build/windows-cross
cmake --build build/windows-cross --target test_file_selector --parallel 2
xvfb-run -a ctest --test-dir build/windows-cross \
  -R '^test_file_selector$' --output-on-failure

git diff --check
```

The Windows directory retains its existing cross-toolchain and Wine emulator.
Logs use `file-selector-selection-*.log` in the relevant build directories.

## Unverified assumptions and remaining risks

- Linux verification uses Tk-generated pointer/key events and pointer warping
  under Xvfb; manual desktop input was not tested.
- Windows coverage checks startup, the opener binding, cancellation and file
  acceptance under native Windows Tk/Wine. It intercepts deliberate posting
  before the native modal loop, so native option selection and actual Windows
  11 execution remain unverified.
- Other legacy layout/navigation issues, including clipping of the arrow in
  a narrow dialog and Windows drive/root semantics, remain outside this fix.
- ASan/UBSan were not run for this Tcl-only production change. Restart the
  application to load the updated script; installed copies need their Tcl
  resources updated as well.

No system installation, commit or push was performed. The existing uncommitted
startup fix and its regression/CMake changes are retained.
