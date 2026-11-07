# Colour selector List-mode selection

## Problem and root cause

Selecting a named colour in List mode raised `can't read "colorBoxValue": no
such variable` in `ColorBoxSelectColor` while assigning the red slider.
The expanded GUI regression reproduced the exact message and callback stack
before the fix (`build/debug/color-list-before.log`).

The callback obtained the final `#rrggbb` field with `lrange ... end end`, which
returns a list, rather than its element. For example, the string representation
of that one-element list is `{#f8f8ff}`. The subsequent character slice starts
with `#` instead of the first hexadecimal digit, so `scan %x` reads nothing and
does not create `colorBoxValue`. Both chooser implementations inherited this
parsing pattern; it exists in the initial repository commit `cd7d7f6`.

Once parsing was corrected, tests also exposed two whitespace-related issues:
the ttk swatch's photo data treated `ghost white` as multiple colour tokens,
and the older shared chooser assembled configuration commands as strings,
splitting colour names containing spaces. The swatch issue dates from the ttk
migration. The shared chooser also used obsolete list-selection subcommands.

## Files and implementation changes

- `lib/ffgraph/tcl/colorBox.tcl`: extract the hex field with `lindex ... end`.
  Clear and set the single selected row explicitly before updating RGB values,
  the current name, preview and palette sample. Preserve the existing return
  and acceptance behavior.
- `lib/ffgraph/tcl/xfmisc.tcl`: make the same hex-field correction in the
  shared chooser callback. Replace obsolete `select from/to` calls with the
  supported `selection clear/set` API. Use braced `catch` scripts for preview,
  target-widget and palette configuration so colour names remain single
  arguments rather than being split when interpolated scripts are evaluated.
- `lib/ffgraph/tcl/ttk.tcl`: construct photo data as one row containing one
  colour element, preserving names containing spaces when filling a swatch.
- `tests/test_ttk_widgets.cpp`: extend the existing full/initial-suite GUI
  regression to open both chooser variants in each 2D/3D graph window. Select
  `ghost white`, `black` and `red` through their actual list click bindings;
  check a single selected row, intact colour name, RGB channels, preview and
  palette pixels. Cover double-click acceptance in the inspector chooser and
  OK acceptance in the shared chooser. Check the returned colour and cleanup.

No graph/container ownership, layout algorithm, parser, experimental SPQR or
parked WSLg inspector changes are involved. No asset conversion is needed.

## Tests actually executed and results

- Before the fix: the expanded Debug regression **failed with the reported
  `colorBoxValue` error** through the real list click binding.
- After the parsing fix, the same test exposed the multi-word swatch/legacy
  configuration problems; those were corrected rather than bypassed.
- Focused Debug chooser regression: **passed** for both implementations,
  both projections, zero/nonzero RGB channels and multi-word colour names.
- Full Linux Debug suite: **95/95 passed**.
- Full Linux Release suite: **97/97 passed**.
- Native Windows cross-build chooser regression under Wine/Xvfb: **passed**.

- Focused ASan/UBSan chooser regression: **passed**. GUI leak detection was
  OFF under the existing sanitizer partition policy.
- Refreshed `build/windows-png-preview/` and
  `build/ffgraph-windows-native-debug.zip`; checked the staged hex-token and
  swatch-quoting fixes before packaging.
- `git diff --check`: **passed**.

## Build and verification commands

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug -R '^test_ttk_widgets$' --output-on-failure
ctest --preset debug --output-on-failure --parallel 2

cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release --output-on-failure --parallel 2

cmake --preset asan
cmake --build --preset asan --target test_ttk_widgets --parallel 2
ctest --preset asan-gui -R '^test_ttk_widgets$' --output-on-failure

cmake -S . -B build/windows-cross
cmake --build build/windows-cross --parallel 2
xvfb-run -a ctest --test-dir build/windows-cross \
  -R '^test_ttk_widgets$' --output-on-failure

cmake --install build/windows-cross --prefix "$PWD/build/windows-png-preview"
python3 -c 'import shutil; shutil.make_archive("build/ffgraph-windows-native-debug", "zip", "build/windows-png-preview")'
git diff --check
```

Windows retains its existing cross-toolchain/Wine configuration. Logs use
`color-list-*.log` in the corresponding build directories.

## Unverified assumptions and remaining risks

- Windows verification uses native Tcl/Tk under Wine/Xvfb, not a real Windows
  11 desktop. Tests execute the actual binding scripts directly so callback
  failures propagate synchronously rather than opening background-error dialogs.
- Tests provide deterministic list rows and do not require an X11 `rgb.txt`
  file on Windows. This fix does not add a Windows colour-name catalogue or
  change how the existing chooser discovers its colour file.
- The tests cover valid RGB rows and names, not arbitrary malformed external
  colour databases or every historical shared-chooser feature.
- Restart to load the updated Tcl scripts. Installed applications need the
  updated `colorBox.tcl`, `xfmisc.tcl` and `ttk.tcl`, or the refreshed ZIP.

No system installation, commit or push was performed.
