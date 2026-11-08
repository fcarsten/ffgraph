# Windows canvas editing with Num Lock — 8 November 2026

## Problem and root cause

The Windows application could create nodes, but ordinary dragging resized
selected nodes and Ctrl-click did not create edges. The canvas's legacy
`M1`/`Mod1` bindings assumed the usual X11 Alt mapping. Windows Tk instead maps
Num Lock to Mod1 and Alt to Mod2. When Num Lock is enabled, ordinary node
motion therefore matches `<M1-B1-Motion>` and invokes `changeSize`. Ctrl-click
also matches the later Mod1 node binding, which takes precedence over the
Ctrl-click edge action. The background Ctrl-click binding is installed later
than its Mod1 alternative, explaining why node creation can still work.

Confirmed against the bundled Tk 8.6.16 implementation:
[pointer modifier state](https://github.com/tcltk/tk/blob/core-8-6-16/win/tkWinPointer.c#L50-L87)
and [Windows modifier map](https://github.com/tcltk/tk/blob/core-8-6-16/win/tkWinKey.c).
The same raw modifier also affected edge-inspector clicks, background Zoom,
embedded graph-window resizing and keyboard command routing.

A standalone native Windows Tk dispatch probe selected `plain move` with
state 0, `mod1 resize` with state 8 (Num Lock), and `mod1 resize` with state 12
(Num Lock + Control). Linux and Windows both reproduced that binding selection
when those raw bits were supplied, but their physical keys map differently.
The user's lock-key state was not recorded; the matching reproduction
establishes this specific trigger rather than a Windows 11 desktop observation.

## Files and implementation

- `lib/ffgraph/tcl/stdGraph.tcl`: use semantic `Alt` in the active item,
  canvas, embedded-window and keyboard bindings. Tk resolves Alt for each
  platform. Preserve the script's Latin-1 encoding and the existing handlers,
  including the historically named `Mod1Button1` Zoom helper. Mouse-button
  notation in the usage dialog is separate and unchanged.
- `tests/test_canvas_editing.cpp`: create real 2D/3D graph windows and dispatch
  generated Tk mouse events through their registered bindings. Check Ctrl-click
  node creation, selection and plain dragging without resizing, Ctrl-click
  creation of an edge from the selected node to its target, and deliberate
  Alt-resizing without movement. Exercise Num Lock off/on, Caps Lock, and
  combined Num/Caps/Scroll Lock state. Also verify that ordinary keys route to
  `keyPress` and Alt keys route to `modPress` with those lock states present.
  Explicit event timestamps prevent accidental double-click inspectors.
- `CMakeLists.txt`: register `test_canvas_editing` in both full and initial
  desktop suites, with a timeout and GUI/sanitizer partition label.

No production C++, graph/container ownership, layout algorithms, parser,
experimental SPQR or parked WSLg inspector behavior is changed. Existing
selector and New Graph fixes are preserved.

## Tests actually executed and results

- Before the production change, the new regression passed on Linux and failed
  on native Windows Tk under Wine: `2D lock modifiers: Mod1-`, followed by the
  assertion that the dragged node's range must remain unchanged. Logs:
  `build/{debug,windows-rc1}/canvas-editing-before.log`.
- After the change, the focused regression passed on Linux and Windows/Wine,
  covering all four lock-state combinations in both graph projections and
  keyboard routing. The fixture supplies physical Alt as Mod2 on Windows and
  the default Xvfb Mod1 mapping on Linux.
- Full Linux Debug CTest: **101/101 passed**.
- Full Linux Release CTest: **103/103 passed**, including configured external
  Rome corpus checks.
- Focused ASan/UBSan GUI editing regression: **1/1 passed**. The GUI preset
  disables toolkit leak detection; no production ownership code changed.
- Full native Windows initial suite under Wine/Xvfb: **33/33 passed**,
  including editing, selector, New Graph, themed controls and installed-runtime
  checks. This is not a physical Windows 11 desktop run.

## Build and verification commands

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure --parallel 2
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release --output-on-failure --parallel 2
cmake --preset asan
cmake --build --preset asan --target test_canvas_editing --parallel 2
ctest --preset asan-gui -R '^test_canvas_editing$' --output-on-failure
cmake -S . -B build/windows-rc1
cmake --build build/windows-rc1 --parallel 2
xvfb-run -a ctest --test-dir build/windows-rc1 --output-on-failure
python3 scripts/package-release.py --build build/windows-rc1 --output build/rc1-artifacts/windows --preview
python3 scripts/package-release.py --build build/release --output build/rc1-artifacts/linux --preview
git diff --check
```

Build directories retain the previously configured native/cross toolchains.
Final logs use `build/{debug,release,asan,windows-rc1}/canvas-editing-final.log`.
Wine and Xvfb need access outside this environment's process sandbox.

## Unverified assumptions and remaining risks

Windows checks use native Windows Tk under Wine, rather than a Windows 11
desktop or physical keyboard. Synthetic events supply the state bits emitted
by Windows Tk; tests do not toggle the user's system lock keys. The Linux
fixture uses the default Xvfb modifier map, not every custom X11 key map.
Embedded graph-window resizing, Alt-click edge inspectors and desktop window
manager interception of Alt-drag are not separately automated here.

Installed applications must load the updated `lib/ffgraph/tcl/stdGraph.tcl`;
restart the application or extract the refreshed preview into a fresh directory.
No commit, push, tag or release publication was performed.
