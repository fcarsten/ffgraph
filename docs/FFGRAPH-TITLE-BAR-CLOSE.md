# Title-bar Close and last-graph shutdown — 8 November 2026

## Problem and root causes

The graph window's `WM_DELETE_WINDOW` protocol in `stdGraph.tcl` printed
`Use close` to stderr rather than closing anything. On native Windows GUI
builds without a Tcl stderr channel, it could also raise a channel error.
File > Close already provides the appropriate graph/inspector cleanup through
`recall modPress w` and deferred interpreter deletion.

Simply invoking that handler still did not terminate an application started
from `ffmain.cc`. The `initialGraphs` vector kept startup graphs alive until
after `FFWindow::mainLoop()` returned. Each graph owns layout inspectors with
separate, withdrawn Tk roots. Those roots kept `Tk_GetNumMainWindows()` positive,
so the event loop could not return to release the graphs. The same issue
affected closing the startup graph through the File menu.

The existing retained-owner registry already handles New/Load graphs by
releasing them once their graphic closes, outside active Tcl callbacks.
See `FFGRAPH-PASS11-LOAD-WINDOW-LIFETIME.md` and
`FFGRAPH-PRIORITY2-PASS7-INTERPRETER-TEARDOWN.md` for that mechanism.

## Files and implementation

- `lib/ffgraph/tcl/stdGraph.tcl`: route the graph root's title-bar protocol to
  the existing File > Close command. Preserve the script's Latin-1 encoding.
- `ffmain.cc`: keep startup graph owners during construction/loading, then
  transfer ownership to the existing `FFWindow::retainGraph` registry before
  entering the event loop. Closing a startup graphic can now release its graph
  and hidden inspectors at the normal safe cleanup point. No registry or
  destructor algorithm is changed.
- `tests/test_new_graph_gui.cpp`: invoke the actual title-bar protocol for
  New-created child windows and the original parent, in 2D/3D. Check that the
  parent survives a child's closure, graph/window destruction completes, and
  an open Graph Inspector and helper toplevel are cleaned up with their graph.
- `tests/test_installed_runtime.py`: replace successful forced `exit 0` with
  the title-bar protocol in the private staged resource hooks. Require normal
  process exit within the existing timeout for both 2D/3D executables, testing
  two command-line graphs and an empty startup graph, installed/default and
  explicit resource paths. The hook waits for window initialization before
  closing; it declares its environment-array scope explicitly.

Closing a graph uses existing File > Close semantics. Other graph windows
remain usable, and inspector-specific dismissal protocols are unchanged.
No parser, layout algorithm, experimental SPQR or parked WSLg inspector work
is included. No production test hooks or unconditional process exit are added.

## Tests actually executed and results

- Before the fix, both focused Debug regressions failed:
  `test_new_graph_gui` printed `Use close` and its child stayed alive;
  `test_installed_runtime` timed out waiting for process exit.
  Log: `build/debug/window-close-before.log`.
- With only the protocol fix and the corrected staged hook, the installed
  app loaded both startup graphs and executed its resource/shutdown hooks but
  still timed out. Log: `build/debug/window-close-protocol-only-corrected.log`.
- With the owner handoff too, both focused Debug checks passed. Log:
  `build/debug/window-close-after.log`.
- Full Linux Debug CTest: **101/101 passed**.
- Full Linux Release CTest: **103/103 passed**, including configured external
  Rome checks.
- ASan/UBSan GUI partition: **22/22 passed**, including actual installed-app
  shutdown and graph/inspector/window lifetime checks. Toolkit leak detection
  was OFF under the existing GUI preset.
- Full native Windows initial suite under Wine/Xvfb: **33/33 passed**,
  including normal exit of installed 2D/3D processes and graph/inspector/helper
  closure through the title-bar protocol.

An intermediate staged-hook attempt omitted `global env` after moving the
hook into a procedure. That fixture error was corrected before the isolated
protocol-only reproduction and final verification; it was not an additional
application defect. Full final logs use
`build/{debug,release,asan,windows-rc1}/window-close-final.log`.

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

Build directories retain their previously configured native/cross toolchains.
Xvfb and Wine require access outside the restricted process sandbox here.

## Remaining risks and verification limits

Windows testing uses native Tk under Wine, not a physical Windows 11 desktop.
Regressions invoke the installed `WM_DELETE_WINDOW` script rather than sending
an OS title-bar mouse event. The GUI sanitizer preset disables toolkit leak
detection; its ASan/UBSan results do not establish toolkit leak freedom.

Startup graphs now release their model and inspectors when their graphics
close, using the lifetime policy already applied to New/Load graphs. Explicit
File > Quit behavior is unchanged. Installed copies require the rebuilt binary
and updated Tcl resources. No commit, push, tag or publication was performed.
