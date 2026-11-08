# File selector startup popup

For the later correction to when the completed selector is first shown, see
[FFGRAPH-FILE-SELECTOR-MAPPING.md](FFGRAPH-FILE-SELECTOR-MAPPING.md).

## Problem and root cause

Opening the file selector briefly displayed an extra window on Linux. On
Windows it displayed the Home/Root/Current navigation menu before the main
selector was ready and waited for the menu to be dismissed.

`FileSelectorBox_MakeItems` explicitly posted the navigation menu at `(0, 0)`
and immediately unposted it during construction. X11 returned from `post`,
allowing the following `unpost` to hide it. Native Windows Tk entered its
modal menu loop during `post`, delaying further dialog construction. These
calls also exist in the historical `.old` script. Their original purpose
is undocumented.

The preceding inspection reproduced the platform difference with a minimal
Tk script on Linux and native Windows Tk under Wine. This patch's regression
also failed before the fix with `Navigation menu posted during dialog
construction`, before allowing Tk to enter the native menu loop.

## Files and implementation changes

- `lib/ffgraph/tcl/fileselect.tcl`: remove the construction-time `post` and
  `unpost` calls and explain why the menu is posted only on user request.
  Preserve the menu entries, commands and selector workflow. A subsequent
  selection fix replaces the custom popup/grab bindings; see
  [FFGRAPH-FILE-SELECTOR-SELECTION.md](FFGRAPH-FILE-SELECTOR-SELECTION.md).
- `tests/test_file_selector.cpp`: create real 2D and 3D graph windows and run
  the real `FileSelectorBox` procedure. An execution trace rejects any startup
  menu posting, including a post immediately followed by unpost. The test
  checks complete dialog construction, hidden navigation menu, all three
  navigation entries, cancellation, reopening, file acceptance and withdrawal
  after completion. It probes the original navigation button binding and
  intercepts its deliberate `post` before entering a native modal menu on
  Windows. The subsequent selection regression exercises full pointer-event
  sequences on Linux, including all three commands and dismissal.
- `CMakeLists.txt`: register `test_file_selector` in both full and initial
  suites, using native Tk on Windows and Xvfb on Linux. Label it as a GUI test
  and retain a bounded timeout.

No graph ownership, parser, layout, experimental SPQR or parked WSLg
inspector changes are involved.

## Tests actually executed and results

- New Debug regression before the fix: **failed as expected**, with the
  explicit premature-post error (`build/debug/file-selector-before.log`).
- Focused Debug regression after the fix: **passed** in both 2D and 3D
  (`build/debug/file-selector-after.log`).

- Full Linux Debug suite: **93/93 passed**.
- Full Linux Release suite: **95/95 passed**.
- Native Windows cross-build selector regression under Wine/Xvfb: **passed**,
  including both projections. A real Windows 11 desktop was not tested.
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

The Windows build directory retains its existing MinGW/Tcl-Tk cross-toolchain
and Wine emulator configuration. Logs use `file-selector-*.log` under the
corresponding build directories. The ordinary Linux GUI tests use Xvfb.

## Unverified assumptions and remaining risks

- No physical Windows 11 desktop or manual pointer interaction was tested.
  The Windows navigation binding probe deliberately intercepts the requested
  popup; it does not automate native menu selection or establish positioning.
  Linux selection coverage was added in the subsequent report linked above.
- Other legacy selector behavior, including Windows drive/root navigation,
  is outside this fix. The existing navigation commands remain unchanged.
- ASan/UBSan were not run for this Tcl-only production change; graph/container
  ownership is unchanged.
- Restart a build-tree application to load the updated script. Installed
  applications need their installed Tcl resources updated as well.

No system installation, commit or push was performed.
