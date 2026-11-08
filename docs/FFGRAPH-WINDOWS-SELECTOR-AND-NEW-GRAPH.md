# Console-free file selector and New Graph lifetime — 8 November 2026

## Problem and root causes

The Windows 11 diagnostic log supplied by the user identifies why the file
selector never appeared. `FileSelectorBox` successfully scanned its fallback
directory, but aborted while still withdrawn with:

```text
can not find channel named "stderr"
puts stderr "FSBox: No valid Dir-Name ->$start_dir<-"
```

The installed bundle does not contain the default relative `./demographs`
directory. Falling back to the current directory is supported behavior, but
the associated warning assumed a Tcl standard error channel. A native Windows
GUI executable can have no such channel. The exception occurred before the
response wait and `wm deiconify`, independently of menu versus keyboard entry.
Earlier ownership and focus corrections remain relevant to stacking and focus,
but did not address this exception. See `FFGRAPH-FILE-SELECTOR-OWNERSHIP.md`
and `FFGRAPH-FILE-SELECTOR-DIAGNOSTICS.md` for those preceding changes.

New Graph had a separate lifetime defect. The `recall modPress n` handler
created a window through a temporary `FFGraph()` handle. Windows observe their
graphs rather than owning them, so destruction of that temporary immediately
destroyed the graph and its window. File > Load already uses a retained owner
for the same reason; see `FFGRAPH-PASS11-LOAD-WINDOW-LIFETIME.md`.

## Files and implementation

- `lib/ffgraph/tcl/fileselect.tcl`: route both informational warning sites
  through `FileSelectorBox_Warn`. It attempts the usual stderr output safely
  and records a warning through the optional selector logger. An unavailable
  channel or logger cannot abort dialog construction or completion.
- `fgraphic/FFTclBasicWindow.cc`: hold a local graph owner while creating the
  new window, then register it with the existing `FFWindow::retainGraph`
  mechanism. Existing event-loop cleanup releases it after its graphic closes.
- `tests/test_file_selector.cpp`: remove the Tcl stderr alias temporarily,
  without closing its underlying channel, and restore it after the exercise.
  Start from a guaranteed nonexistent directory to exercise the warning and
  fallback. Run the existing 2D/3D navigation, menu, focus, acceptance and
  dismissal checks with diagnostics both disabled and enabled.
- `tests/test_new_graph_gui.cpp`: invoke the actual New menu and `recall`
  command in 2D/3D. Track window destruction externally, check that a distinct
  usable graph survives the callback and cleanup, then close it and check that
  both the window and graph are released. Its lifetime marker has a unique
  graph-label type so that the graph actually adopts it.
- `CMakeLists.txt`: register the new test in the common desktop GUI test set,
  including the initial Windows suite and sanitizer GUI partition.
- `.gitignore`: exclude the user-supplied root `ffgraph-selector.log` from
  accidental commits and preview source archives. The supplied log is retained
  without modification.

The preceding uncommitted selector ownership/focus and diagnostic changes are
preserved. This follow-up does not redesign the ownership registry or change
graph parsing, container ownership, layout selection or SPQR.

## Tests actually executed

- Before the production fixes, both targeted Debug regressions failed: the
  selector reproduced the missing-stderr exception, and New Graph destroyed
  its child window before returning from the command.
- Linux Debug full CTest: **100/100 passed**.
- Linux Release full CTest: **102/102 passed**, including the configured
  external Rome corpus tests.
- ASan/UBSan GUI partition: **21/21 passed**, including both regressions and
  the existing load/teardown lifetime checks. The GUI preset disables leak
  detection for toolkit allocations; this is not a strict toolkit-leak check.
- Windows Release cross-build and full initial Wine/Xvfb suite: **32/32
  passed**, including console-free selector and New Graph checks using native
  Windows Tcl/Tk. Both 2D and 3D command paths are covered.

The pre-fix failures are in `build/debug/windows-desktop-before.log`.
Full test logs are `build/{debug,release,windows-rc1}/windows-desktop-fixes.log`
and `build/asan/windows-desktop-gui.log`. The first Debug lifetime-marker
fixture reused a default graph-label type and was rejected by the graph; its
destruction assertion failed until the fixture used a unique type. The final
full Debug, Release and sanitizer runs use the corrected fixture.

## Build and verification commands

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure --parallel 2
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release --output-on-failure --parallel 2
cmake --preset asan
cmake --build --preset asan --parallel 2
ctest --preset asan-gui --output-on-failure --parallel 2
cmake -S . -B build/windows-rc1
cmake --build build/windows-rc1 --parallel 2
xvfb-run -a ctest --test-dir build/windows-rc1 --output-on-failure
python3 scripts/package-release.py --build build/windows-rc1 --output build/rc1-artifacts/windows --preview
python3 scripts/package-release.py --build build/release --output build/rc1-artifacts/linux --preview
git diff --check
```

The Windows build directory already uses the local MinGW/native Tk toolchain
and Wine emulator. Wine and Xvfb execution require access outside the restricted
process sandbox in this development environment.

## Unverified assumptions and remaining risks

The user's Windows 11 failure is evidenced by their log; final desktop
confirmation of the patched application still needs a fresh extracted preview.
Wine/native Tk checks do not reproduce every Windows window-manager behavior.
Linux's reported brief disappearance has not been verified on the user's
desktop; the earlier change establishes transient ownership while withdrawn,
and the existing selector regressions check that scanning completes before it
becomes visible.

The retained-owner registry is unchanged. Direct Tcl destruction of a root
without removing its C++ graphic can retain that graph until application exit,
as documented for File > Load. Native Windows CI for these uncommitted changes
and a strict ASan GUI leak run were not performed. No commit, push, tag or
release publication was made.
