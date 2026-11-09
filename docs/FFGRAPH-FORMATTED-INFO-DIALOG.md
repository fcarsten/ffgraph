# Formatted Info dialog — 9 November 2026

## Problem and design

The 2D Info dialog displayed contributor acknowledgements, license, project
URL and usage instructions in a single Tk `message` widget. Markdown-like
headings appeared literally and links were not actionable. The 3D application
used the shared fallback containing only `No Help Today ...`.

Both applications now use a shared read-only Tk `text` view. Text tags provide
a larger title, bold section headings and role/action labels, and underlined
hyperlinks. Text remains selectable for copying. Word wrapping, a themed
vertical scrollbar and a resizable window keep longer credits/instructions
readable at different window sizes. There is no ttk text widget; theme colors
and ttk scrollbar/OK button keep the display consistent with the existing UI.

## Files and implementation

- `lib/ffgraph/tcl/info.tcl`: shared widget construction and content. Preserve
  contributor names/roles and the historic usage section, clarify its Zoom
  modifier as Alt, and correct the background spelling. The license label links
  to the GNU GPL v3 page and the project URL links to GitHub. Fonts derive from
  `TkDefaultFont`; no additional named-font owners are created.
- Link tags support mouse activation, pointer cursor feedback and Tab/Shift-Tab
  navigation with visible focus and Return/Space activation. Tab past the last
  link moves on to the next control. Selecting a URL for copying does not
  launch it. OK and Escape close the dialog, which can be reopened.
- Browser launch uses argument lists and runs asynchronously: Windows uses
  `rundll32.exe url.dll,FileProtocolHandler`; Linux uses `xdg-open` with a
  `gio open` fallback. Only HTTP(S) targets are accepted. Missing/spawn-failing
  launchers produce a parented error with the URL available for copying.
- `lib/ffgraph/tcl/stdGraph.tcl`: source the new content explicitly as UTF-8
  and delegate the existing `addUserUsage` entry point to the shared builder.
  Preserve this legacy script's original byte encoding.
- `lib/ffgraph/tcl/plainGraph.tcl`: remove the duplicate plain-message Info
  implementation so both 2D and 3D receive the same formatted content.
- `tests/test_ttk_widgets.cpp`: exercise the real Help > Usage menu in both
  projections. Check content including accented names, section/font tags,
  disabled editing, selection, scrolling, hyperlink commands and keyboard
  focus/activation, launcher argument boundaries, failure reporting, rejected
  URI schemes, OK/Escape closing, reopening and named-font cleanup.

Installation already copies the Tcl resource directory, including the new
helper. Archived `.old`/`.mit`/`.ohne` and similar scripts are unchanged. No
production C++, graph ownership, layout, parser, SPQR or parked WSLg inspector
changes are included.

## Tests actually executed and results

- Standalone Tk construction probe: passed, reporting a disabled `Text` view
  with ordinary body text separate from its formatting tags.
- Focused Linux Debug ttk regression: **1/1 passed**.
- Full Linux Debug CTest: **102/102 passed**.
- Full Linux Release CTest: **104/104 passed**, including configured external
  Rome checks.
- Focused native Windows Tk under Wine/Xvfb: **2/2 passed**, covering the
  extended ttk/Info regression and installed 2D/3D runtime resource lookup.
- Captured and visually inspected the actual Tk window at **846 × 571**:
  `build/info-dialog.png`. Headings, accented contributor names, license and
  repository links, wrapping and scrollbar layout were readable. The smaller
  450 × 300 case is also exercised by the regression.
- `git diff --check`: passed.

The link regression replaces `exec`, `auto_execok` and the error message box
inside its test interpreter and restores them in `finally`. It checks the
bound link actions without opening a real browser or requiring Internet access.
The full Windows suite and sanitizer suites were not rerun for these Tcl-only
presentation changes. Test logs are `build/debug/info-focused.log` and
`build/{debug,release,windows-rc1}/info-final.log`.

The screenshot environment has neither Python Tkinter nor Pillow. The final
capture used installed `wish` plus X11 pixel capture through Python's standard
library/ctypes; no new runtime or development dependency was installed.

## Build and verification commands

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure --parallel 2
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release --output-on-failure --parallel 2
cmake -S . -B build/windows-rc1
cmake --build build/windows-rc1 --target ffgraph test_ttk_widgets --parallel 2
xvfb-run -a ctest --test-dir build/windows-rc1 -R '^(test_ttk_widgets|test_installed_runtime)$' --output-on-failure
python3 scripts/package-release.py --build build/windows-rc1 --output build/rc1-artifacts/windows --preview
python3 scripts/package-release.py --build build/release --output build/rc1-artifacts/linux --preview
git diff --check
```

Build directories retain their existing toolchain/emulator configuration.
Xvfb/Wine execution requires access outside the restricted process sandbox.

## Remaining verification limits

Windows checks use native Windows Tk under Wine, rather than a physical Windows
11 desktop. Actual browser handoff and physical keyboard navigation still need
desktop confirmation. The Linux desktop must provide `xdg-open` or `gio` for
browser launching. Asynchronous browser processes can fail after a successful
spawn; the dialog catches missing launchers and synchronous launch errors.
Custom high-contrast/dark themes and per-monitor DPI behavior were not tested.

Restart the application with updated installed Tcl resources. No commit, push,
tag or release publication was performed.
