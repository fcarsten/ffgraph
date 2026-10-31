# ffgraph inspector visibility / WSLg investigation — handover

**Status:** Parked at user request, 31 October 2026. **Not conclusively resolved.**

## Environment

- Ubuntu 24.04 under WSL2 on Windows; Linux graphical windows displayed through WSLg.
- `DISPLAY=:0`, `WAYLAND_DISPLAY=wayland-0`; `XDG_SESSION_TYPE` was not shown in the command output.
- `wsl --version` output supplied by user:
  - WSL 3.0.3.0; Linux kernel 6.18.54.1-1
  - WSLg 1.0.79; MSRDC 1.2.7391
  - Direct3D 1.611.1-81528511; DXCore 10.0.26100.1-240331-1435.ge-release
  - Windows 10.0.26300.9711
- ffgraph is a legacy C++/Tcl/Tk graphical editor, ported to modern Ubuntu; recent work introduced CMake, graph/label reference fixes, callback deregistration, and an inspector visibility patch.

## Symptoms (observed by user)

- The graph inspector sometimes disappears from the Windows desktop and fails to reappear when requested through **Options → Inspector**.
- Initially seemed connected to double-clicking graph canvas, but reproduced sporadically after opening/hiding via the Options menu; click position and node/edge/background do not seem relevant.
- Inspector sometimes remains in Windows taskbar's window list and can sometimes be restored there; after several attempts the Options command may start working again.
- The GUI sometimes becomes sluggish or temporarily unresponsive around these operations; application did not crash.
- Windows taskbar groups ffgraph's graph and inspector windows under one icon; standalone Tk test windows appeared as separate taskbar icons.

## Source-level findings

- `lib/ffgraph/tcl/stdGraph.tcl`: the canvas binds `<Double-Button-1>` to `recall modPress i`; this routes to the inspector command and explains why a double-click anywhere on the canvas can affect the inspector.
- `FFTclWindow::showInspector()` opens the inspector (or reuses it), then calls `updateInspector()`, which rebuilds inspector label menus and populates content each time.
- The older `FFTclInspector::show()` called `iconify(); deiconify();`. A patch changed this to direct `deiconify()` followed by `raise .`, with a check/recreation for a missing Tk root window. This is a useful improvement but **did not fully cure** the intermittent behaviour.
- ffgraph creates a **separate Tcl interpreter per window**, which differs from a conventional single-interpreter multi-`toplevel` Tk test.
- `FFWindow::update()` currently executes `catch update` in Tcl. Full `update` calls permit nested event processing and swallow errors, both possible sources of additional GUI responsiveness issues; neither is confirmed as the primary cause.

## Experiments and evidence

1. A simple `wish` script with a main window and an inspector `toplevel`, using `wm withdraw`, `wm deiconify` and `raise`, showed/hid reliably, but *visible disappearance* consistently lagged by roughly a second.
2. A variant briefly toggling `wm attributes -topmost` was also reliable but had similar hide latency; not adopted as a production workaround.
3. A second test using **two Tcl interpreters in one process**, `ffgraph-tk-two-interpreters.tcl`, measured Tcl-side return times for `wm withdraw`, `wm deiconify; raise` and `wm iconify; wm deiconify; raise`.
4. User's measurements:

   ```text
   21:59:37 wm withdraw .: 1ms state=withdrawn viewable=0
   21:59:37 wm deiconify .; raise .: 1ms state=normal viewable=1
   21:59:48 wm withdraw .: 0ms state=withdrawn viewable=0
   22:00:17 wm iconify .; wm deiconify .; raise .: 2000ms state=normal viewable=1
   22:00:19 wm iconify .; wm deiconify .; raise .: 2002ms state=normal viewable=1
   22:00:31 wm iconify .; wm deiconify .; raise .: 2002ms state=normal viewable=1
   22:00:47 wm deiconify .; raise .: 0ms state=normal viewable=1
   22:00:56 wm iconify .; wm deiconify .; raise .: 2000ms state=normal viewable=1
   ```

   The user specifically observed that `wm withdraw` logged its near-zero return **well before the window visually disappeared on the Windows desktop**. Tcl's internal `wm state` and the Windows desktop's rendering/stacking state are therefore not synchronised for these operations. Repeated ~2000 ms timings on the `iconify` path are consistent with a timeout/wait in the window-management stack, but not a proven diagnosis.

5. A previous investigation found a somewhat similar public WSLg report involving 1–2-second X11 window delays: <https://github.com/microsoft/wslg/issues/1443>. The reported environment/version differs and no exact match has been established.

## Current assessment

- **Likely WSLg/Tk interaction:** slow visual window transitions and some ~2-second `iconify` operations, reproduced without ffgraph.
- **Not yet explained:** why Options → Inspector intermittently fails to restore its window and why ffgraph sometimes becomes unresponsive. This might involve application-level event-loop reentrancy, inspector work, or a WSLg window-state race.
- Neither the double-click handler nor the recent ownership patches have been proven to be the root cause.
- Avoid treating `wm state=normal` or a fast return from `wm withdraw` as proof that Windows has completed the visible operation.

## Recommended next steps when resuming

1. Keep the direct `deiconify(); raise` improvement; **do not** reintroduce `iconify()` as part of the ordinary show path.
2. Add *opt-in* timestamped diagnostics around `FFTclWindow::showInspector()`, `FFTclInspector::show()`, `updateInspector()`, `buildMenus()`, `inspect()`, `FFWindow::update()`, and relevant Tk `<Map>`, `<Unmap>`, `<Visibility>`, `<FocusIn>`/`<FocusOut>` events. Capture interpreter identity, `wm state`, `winfo exists`, `winfo viewable`, and elapsed time.
3. Distinguish Tk callback duration from delayed desktop-visible transitions. Record Tcl errors instead of silently discarding them.
4. Test repeated show/hide and window-manager minimise/restore, with and without WSLg (e.g. conventional Xorg/Xvfb for event correctness and an alternative Windows X server for user-visible stacking). Note that Xvfb cannot validate WSLg/Windows taskbar focus behavior.
5. Investigate whether `FFWindow::update()` can be changed from reentrant `update` to `update idletasks` at specific call sites; do not replace it globally without examining callers.
6. Consider caching inspector menus or rebuilding them only when their contents change, after measuring the cost.
7. Only introduce `-topmost`, focus-forcing, or platform-specific workarounds if measurements justify them; they may cause focus-stealing.
8. Keep Tcl interpreter lifecycle/destruction fixes in **separate patches** from inspector/WSLg diagnostics so regressions can be bisected.

## Relevant files/artifacts

- `fgraphic/FFTclInspector.cc`, `fgraphic/FFTclWindow.cc`, `fgraphic/FFTclGraphInsp.cc`, `fgraphic/FFWindow.cc`
- `lib/ffgraph/tcl/stdGraph.tcl`, `tests/test_inspector_visibility.cpp`
- Earlier patch: `ffgraph-inspector-visibility-fix.patch`
- Standalone diagnostic: `ffgraph-tk-two-interpreters.tcl`

This document deliberately captures a **working hypothesis and reproducible evidence**, not a verified root cause or a claimed fix.
