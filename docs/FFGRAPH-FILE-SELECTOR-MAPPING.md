# File selector presentation timing

## Problem and root cause

The selector was reported to appear, disappear briefly and then reappear.
Its creation/reuse path exposed the window before initializing its contents.
For a new selector, the first `update` mapped the newly created toplevel;
for a reused selector, `wm deiconify` showed it immediately. Further `update`
calls during directory scans processed events while the title, listboxes and
entry were still being prepared.

An execution trace reproduced the early presentation on Linux: the selector
was mapped on entry to `FileSelectorBox_GetDirStruct`. The new regression
failed with `File selector became visible before directory initialization
finished` (`build/debug/file-selector-mapping-before.log`). Xvfb has no desktop
window manager, so this establishes the premature mapping, not the user's
exact disappear/reappear animation or its window-manager mechanism.

## Files and implementation changes

- `lib/ffgraph/tcl/fileselect.tcl`: withdraw a new selector immediately after
  creating it, and keep a reused selector withdrawn while updating it. Finish
  pending layout work with `update idletasks`, set the response state to
  `wait`, then deiconify and raise the completed selector once. Keep the
  existing withdrawal after acceptance/cancellation and retain navigation fixes.
- `tests/test_file_selector.cpp`: trace directory initialization and reject
  visible scans during opening. Record actual selector `<Map>` events and
  require one per opening, each occurring in the ready `wait` state. Cover
  both first creation and reopening in 2D/3D while preserving the existing
  navigation, dismissal, cancellation and file-acceptance regression coverage.

No graph/container ownership, parser, layout algorithm, experimental SPQR or
parked WSLg inspector behavior changes are involved.

## Tests actually executed and results

- New presentation regression before the fix: **failed as expected**, exposing
  the visible selector during directory initialization.
- Focused Debug regression after the fix: **passed** in 2D/3D, with one ready
  mapping per opening and the previous interaction checks still passing.

- Full Linux Debug suite: **93/93 passed**.
- Full Linux Release suite: **95/95 passed**.
- Native Windows cross-build selector regression under Wine/Xvfb: **passed**,
  including the mapping checks in both projections.
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

Logs use `file-selector-mapping-*.log` in the relevant build directories. The
Windows directory retains its existing cross-toolchain/Wine configuration.

## Unverified assumptions and remaining risks

- Manual desktop/Wayland/WSLg presentation and the reported exact animation
  were not observed here. The automated checks establish preparation while
  hidden and a single ready mapping under the available Tk runtimes.
- Real Windows 11 and native Windows menu option selection remain unverified;
  the existing Windows binding probe intercepts native popup posting.
- ASan/UBSan were not run for this Tcl-only production change. Existing
  uncommitted selector changes are retained. No system installation, commit
  or push was performed.
- Restart a build-tree application to load the script. Installed applications
  need updated installed Tcl resources.
