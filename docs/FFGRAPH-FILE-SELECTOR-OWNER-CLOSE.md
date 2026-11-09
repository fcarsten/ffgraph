# Closing a graph while its file selector is active — 9 November 2026

## Problem and root cause

Closing a graph with a Load/Save As selector open could report:

```text
invalid command name "recall"
    while executing
"recall modPress w"
    (command for "WM_DELETE_WINDOW" window manager protocol)
```

The title-bar handler immediately removed the graph's graphic and its C++
window. That destructor removed `recall`, while the Tcl interpreter and root
still existed pending deferred cleanup. Another Close request could therefore
call a missing command. More seriously, the file selector was still inside
`tkwait`, called through the deleted window's C++ `fsBox()`/Load/Save callback.
The outer callback must finish before its window can be deleted; deferring
interpreter deletion alone does not preserve that C++ object.

The earlier title-bar and last-graph shutdown changes remain applicable; see
`FFGRAPH-TITLE-BAR-CLOSE.md`. This follow-up orders selector cancellation and
window closure safely rather than changing graph reference ownership.

## Files and implementation

- `lib/ffgraph/tcl/stdGraph.tcl`: install `closeGraphWindow` as the title-bar
  protocol. While a selector is active, request cancellation and remember the
  pending Close. Cancel a nested directory-confirmation response too. Once the
  selector unwinds, the existing Close command performs graph cleanup. Ignore
  repeated requests after `recall` has already been removed. Preserve Latin-1.
- `lib/ffgraph/tcl/fileselect.tcl`: wrap the existing selector body to track
  active calls with guaranteed `finally` cleanup. Schedule pending closure at
  idle only after the selector finishes; its C++ caller then returns with an
  empty selection. Check cancellation around preparation event processing so
  it cannot be overwritten by a fresh `wait`. Validate/create directories only
  for an accepted selection, avoiding a new prompt after cancellation.
- `fgraphic/FFTclBasicWindow.cc`: if `modPress w` arrives through File > Close
  or the keyboard while a selector is active, use the same cancellation path.
- `tests/test_file_selector_owner_close.cpp`: dispatch real Load and Save As
  menu commands in 2D/3D, closing during preparation, response waiting and the
  Save As directory confirmation. Send repeated title-bar requests and the
  shared C++ Close command. Check window/command liveness inside the nested
  callback, including nested idle processing, then verify closure after the
  file operation returns. Also check repeated post-close requests, no unwanted
  directory creation, surviving other graph windows and complete Tk teardown.
- `CMakeLists.txt`: register the regression in full/initial desktop suites and
  the GUI/sanitizer partition, with a bounded timeout.

Concurrent user changes to window-title text are preserved and are separate
from this fix. No layout, parser, container ownership, experimental SPQR or
parked WSLg inspector changes are included.

## Tests actually executed and results

- Before the production fix, the new Debug regression reproduced the exact
  `invalid command name "recall"` error and observed the owner already deleted
  inside the still-active file-dialog callback. It stops at that checkpoint
  instead of intentionally returning through the deleted C++ object. Log:
  `build/debug/selector-owner-before.log`.
- After the fix, focused Debug selector, owner-close, New Graph and installed
  runtime checks passed **4/4** (`selector-owner-after.log`).
- Full Linux Debug CTest: **102/102 passed**.
- Full Linux Release CTest: **104/104 passed**, including configured external
  Rome checks.
- ASan/UBSan GUI partition: **23/23 passed**, including the new nested-dialog
  regression. Toolkit leak detection was OFF under the existing GUI preset.
- Windows initial suite under Wine/Xvfb: **33/34 passed** on the full run,
  including the new owner-close regression (all ten cases), selector, New Graph
  and installed shutdown checks. The existing ttk regression timed out in its
  input-dialog stage. An isolated retry passed **1/1** with the same executable
  and resources, without code changes. No test was disabled. Logs:
  `build/windows-rc1/selector-owner-final.log` and
  `build/windows-rc1/selector-owner-ttk-recheck.log`.
- Whitespace validation of the fix files passed. Concurrent user credit edits
  outside those files were left unchanged.

Final logs are `build/{debug,release,asan,windows-rc1}/selector-owner-final.log`.

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
git diff --check -- CMakeLists.txt fgraphic/FFTclBasicWindow.cc lib/ffgraph/tcl/fileselect.tcl lib/ffgraph/tcl/stdGraph.tcl
```

Build directories retain their existing native/cross toolchains. Xvfb and Wine
need access outside this environment's restricted process sandbox.

## Verification limits and remaining risks

Windows checks use native Tk under Wine rather than a physical Windows 11
desktop. Tests invoke the registered WM protocol rather than a physical title-bar
click. The sanitizer GUI preset disables toolkit leak detection; ASan/UBSan
runtime success does not establish toolkit leak freedom.

The Windows ttk timeout above was not reproduced on retry; its cause remains
unverified. A successful retry does not establish that its focus/timing behavior
has been fixed by this patch.

The ordering applies to this application's selector and its confirmation loop;
it does not redesign all legacy nested dialog/event-loop behavior. Existing
File > Close semantics apply after cancellation. Restart installed applications
with both the rebuilt executable and updated Tcl resources. No commit, push,
tag or publication was performed.
