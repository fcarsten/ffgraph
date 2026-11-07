# Layout inspector selection on Windows

## Problem and root cause

The reported symptom is a layout inspector not appearing after selection on
Windows. The graph-label dropdown in the Graph Inspector routes selection
through Tcl's `addMenu` / `OptionButtonSet`, then C++ `recall setMenu`, which
finds the graph label and calls `showInspector()`.

`OptionButtonSet` inferred the chosen label from `$menu index active` when the
callback ran. That index describes the current highlight, not necessarily the
entry whose command is executing. With no highlight, the helper dispatched an
empty label and no layout inspector opened. A different highlight could select
the wrong label instead. This is particularly unsuitable for native menus,
which manage highlighting/dismissal outside the application's callback.

The regression reproduces the lost label using the actual inspector menu
commands in both Linux Tk and native Windows Tk under Wine. It failed before
the fix with `Layout selection lost its entry label: FFSugiyama`.

The same active-index inference is present in the initial repository commit
`cd7d7f6`, before the native Windows port (`17fdaca`) and ttk migration
(`b7a3751`). This dependency is longstanding, rather than introduced by ttk.
The preceding GUI coverage constructed and operated layout controls directly;
it did not select their labels through the Graph Inspector dropdown.

## Files and implementation changes

- `lib/ffgraph/tcl/finspector.tcl`: capture the menu entry's label in its
  command using a Tcl list. Pass that label explicitly to `OptionButtonSet`
  instead of querying the highlight at callback time. List construction also
  preserves labels as data when they contain spaces or Tcl metacharacters.
  Preserve the script's byte encoding.
- `tests/test_layout_controls_gui.cpp`: extend the existing full/initial-suite
  regression to create a real Graph Inspector and invoke its actual nested
  graph-label menu entries. Check Sugiyama, Spring and Ortho inspectors from
  both 2D and 3D graph windows, first with no active highlight and then a
  different highlight. Require the correct selected label, mapped normal
  inspector window, successful dismissal and reopening. Retain the existing
  algorithm-control checks, reject reported errors and check final Tk window
  cleanup.

No production C++ window handling, graph/container ownership, layout algorithm,
experimental SPQR or parked WSLg inspector behavior changes are involved.
The main window's Layout menu continues to execute algorithms directly;
this correction concerns the Graph Inspector's label-selection dropdown.

## Tests actually executed and results

- Extended Debug regression before the fix: **failed as expected** with the
  lost-label message (`build/debug/layout-inspector-before.log`).
- Extended native Windows regression before the fix: **failed as expected**
  with the same message (`build/windows-cross/layout-inspector-before.log`).
- Focused Debug and native Windows regressions after the fix: **passed**,
  including all three layout inspectors, repeated opening and both projections.
- Full Linux Debug suite: **94/94 passed**.
- Full Linux Release suite: **96/96 passed**.
- Focused ASan/UBSan GUI regression: **passed**. GUI leak detection was OFF
  under the existing sanitizer partition policy.

- Final focused native Windows/Wine compatibility checks: **3/3 passed**
  (layout inspector selection, installed runtime and ttk widgets).
- `git diff --check`: **passed**.
- Refreshed `build/windows-png-preview/` and
  `build/ffgraph-windows-native-debug.zip` with the corrected production script.

## Build and verification commands

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug -R '^test_layout_controls_gui$' --output-on-failure
ctest --preset debug --output-on-failure --parallel 2

cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release --output-on-failure --parallel 2

cmake --preset asan
cmake --build --preset asan --target test_layout_controls_gui --parallel 2
ctest --preset asan-gui -R '^test_layout_controls_gui$' --output-on-failure

cmake -S . -B build/windows-cross
cmake --build build/windows-cross --parallel 2
xvfb-run -a ctest --test-dir build/windows-cross \
  -R '^(test_layout_controls_gui|test_ttk_widgets|test_installed_runtime)$' \
  --output-on-failure --parallel 1

cmake --install build/windows-cross --prefix "$PWD/build/windows-png-preview"
python3 -c 'import shutil; shutil.make_archive("build/ffgraph-windows-native-debug", "zip", "build/windows-png-preview")'
git diff --check
```

The Windows build directory retains its existing MinGW/Tcl-Tk cross-toolchain
and Wine emulator. Logs use `layout-inspector-*.log` in the respective build
directories. The staged Windows script was checked for the captured-label
callback before refreshing the preview ZIP.

## Unverified assumptions and remaining risks

- A real Windows 11 desktop was not available. The deterministic regression
  invokes actual menu commands with controlled highlight states; it does not
  automate native pointer selection or verify taskbar/foreground behavior.
- An exploratory standalone native-popup/keyboard-injection probe timed out
  under Wine. It is not counted as a successful reproduction or validation.
- This establishes a concrete selection/dispatch bug. It does not establish
  that all possible causes of an invisible window on another desktop are fixed.
- The production fix is entirely Tcl. Restart the application and update its
  installed `lib/ffgraph/tcl/finspector.tcl`, or use the refreshed ZIP. Existing
  already-running interpreters retain their old procedures and callbacks.

No system installation, commit or push was performed.
