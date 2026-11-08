# File selector diagnostics — 8 November 2026

## Problem and implementation

The Windows invisible-selector report and Linux initial blink need evidence
from the affected desktops. The ownership correction in
`FFGRAPH-FILE-SELECTOR-OWNERSHIP.md` fixes a demonstrated stacking problem,
but local Wine/Openbox tests do not establish the exact desktop behavior.

The diagnostic-enabled Windows/Wine test exposed an additional focus steal.
The log showed focus reaching `.fileselect.choose.current.e`, followed by a
`focus .frame0.canvas2` command while the selector was still mapped and waiting.
The canvas's unconditional `<Enter>` binding ran on a delayed pointer-crossing
event delivered after dialog/menu mapping. A new explicit Enter-event regression
failed on Linux before the binding correction too. Passive canvas hover now
leaves focus alone while the selector is viewable; normal hover after dismissal
and explicit canvas clicks retain their previous behavior.

Set **`FFGRAPH_FILE_SELECTOR_LOG`** to a writable log filename before launching
the application. Logging is disabled by default. Records are appended as UTF-8
and closed after each write, so they can be inspected while the GUI is waiting.
No console or debugger is required on Windows.

From PowerShell in the extracted package directory:

```powershell
$env:FFGRAPH_FILE_SELECTOR_LOG = "$env:TEMP\ffgraph-selector.log"
& ".\bin\ffgraph.exe"
```

Use `ffgraph3D.exe` for the 3D application. Try opening the selector through the
File menu and the keyboard shortcut, allow a few seconds for the delayed
snapshots, then close the application. Read or share the log at
`$env:TEMP\ffgraph-selector.log`. It includes directory and resource-script
paths. To disable diagnostics for subsequent launches in the same PowerShell:

```powershell
Remove-Item Env:FFGRAPH_FILE_SELECTOR_LOG
```

On Linux, from the checkout:

```sh
FFGRAPH_FILE_SELECTOR_LOG=/tmp/ffgraph-selector.log ./build/debug/ffgraph
```

Logs append across runs. Each record includes time in epoch milliseconds, PID,
interpreter session and opening number. The `enabled` record identifies the
loaded selector script and its modification time, OS, Tcl/Tk patch versions,
windowing system and ttk theme. If it is absent, check the environment setting
and whether the running application loaded the instrumented Tcl resources.

## What is recorded

- Relevant C++ `recall` entry commands for Load, Merge, Save, Save As and export.
- Entry/exit of selector construction, argument parsing, directory scans,
  rescanning and its confirmation message box; scan completion includes counts.
- Response-state writes (`wait`, `ok`, `dismiss`) and `tkwait` entry/exit.
- Window-management commands, focus/grab changes, and Map/Unmap, visibility,
  configuration, focus and destruction events for the owner and selector.
- Snapshots of existence, mapped/viewable state, geometry, owner, focus, grab
  and stacking order. Delayed snapshots run after 100/500/1500/3000 ms while an
  opening remains active and are cancelled when its call completes.
- Tcl error return codes/results and available error information, even when
  the C++ caller subsequently reports the error without a console.

A `call-enter` for a scan without a matching `call-leave` locates a stalled
directory operation. An active `wait` with a mapped selector below the graph
points to stacking/activation. Map/Unmap records and state writes distinguish
window-manager visibility changes from an actual response-loop exit.

## Files and safeguards

- `lib/ffgraph/tcl/fileselect-diagnostics.tcl`: interpreter-local optional
  execution/variable traces, appended event bindings and guarded file writer.
  Snapshot queries do not call `update`, set focus, raise windows or change
  grabs. Repeated enabling is idempotent. A failed log destination is reported
  once to stderr and ignored; it must not prevent selector operation.
- `lib/ffgraph/tcl/fileselect.tcl`: source the helper after defining the selector
  procedures. Existing resource installation includes the new file automatically.
- `lib/ffgraph/tcl/stdGraph.tcl`: guard the canvas's passive Enter-focus handler
  while the file selector is visible. Preserve the script's Latin-1 encoding.
- `tests/test_file_selector.cpp`: exercise both 2D/3D with logging disabled and
  enabled; retain menu/navigation, mapping, cancellation, acceptance and focus
  checks. Check required log records, original Tcl error results, idempotent
  enabling and successful operation with an unwritable log destination. Inject
  a delayed canvas Enter to check focus preservation and verify normal canvas
  hover works again after the dialog is dismissed.

Global menu-construction tracing was removed during verification: it changed
the result observed by an existing menu-creation execution trace. The final
helper records the actual `recall`/selector entry points without attaching a
menu-constructor trace. The existing menu regression then passed unchanged.

No graph/container ownership, parsing or layout changes are made. The earlier
uncommitted selector ownership/focus fix is retained separately.

## Tests actually executed and results

- Focused Linux Debug selector regression passed with all diagnostic modes.
- The explicit canvas Enter regression failed before the focus guard and
  passed afterward (`build/debug/selector-hover-before.log`).
- Full Linux Debug CTest passed **99/99**.
- Full Linux Release CTest passed **101/101**, including configured external
  Rome corpus tests.
- Windows Release cross-build focused selector regression passed under
  Wine/Xvfb with logging disabled/enabled, valid/unwritable log destinations,
  real C++ File-menu entry points and both 2D/3D windows.
- Automatic startup activation through the environment was verified in a
  managed Linux/Openbox probe. Sample records are in
  `build/debug/selector-startup-sample.log`.
- Automatic startup activation also passed with native Windows Tk under
  Wine/Openbox; sample records are in
  `build/windows-rc1/selector-native-startup.log`.
- Final focused Debug/Release and ASan/UBSan GUI runtime selector checks passed;
  the sanitizer preset keeps leak detection OFF. No memory ownership changes
  were made and strict toolkit-leak diagnostics were not rerun.
- Windows and Linux preview packages were refreshed. The packaged selector,
  diagnostics helper and canvas script matched the working files byte for byte;
  their artifact checksums passed verification.

The diagnostic-enabled Windows test initially failed its existing filename
focus assertion; its retained log in `/tmp/ffgraph-selector-test-failure.log`
identified the canvas Enter-focus command described above. This failure was
fixed in the production binding; the assertion was retained. Full CTest logs
are `build/{debug,release}/selector-diagnostics-ctest.log`.

## Build and verification commands

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure --parallel 2
cmake --preset release
cmake --build --preset release --target test_file_selector --parallel 2
ctest --preset release -R '^test_file_selector$' --output-on-failure
cmake -S . -B build/windows-rc1
cmake --build build/windows-rc1 --target test_file_selector --parallel 2
xvfb-run -a ctest --test-dir build/windows-rc1 -R '^test_file_selector$' --output-on-failure
python3 scripts/package-release.py --build build/windows-rc1 --output build/rc1-artifacts/windows --preview
git diff --check
```

## Remaining risks and verification limits

Logging performs synchronous file writes and extra queries/timer callbacks;
it can affect timing. Compare symptoms with diagnostics enabled and disabled.
It is intended for short diagnostic sessions; logs append without rotation.
Paths must be writable and parent directories must already exist.
The exact Windows 11/Linux desktop failures still need logs from those systems.
Native UCRT64 CI for uncommitted changes is unverified. No commit, tag, push or
publication was performed.
