# ttk widget port

## Problem and implementation constraints

The application used classic Tk controls and extensive fixed gray/black colors,
ridge/raised borders and custom mouse press effects. Replacing constructor
names alone would break unsupported options, integer sliders, scrollbar unit
commands, inspector class checks and classic checkbutton state subcommands.
The goal is themed controls while preserving graph editing and layout behavior.

## Files and implementation changes

- `lib/ffgraph/tcl/ttk.tcl`: shared, interpreter-local initialization uses Tk's
  bundled ttk. Windows selects the first available native theme from `vista`,
  `xpnative`, `winnative`; X11 uses `clam`. Global Tk constructors and Tk's own
  class bindings are not overridden. Theme colors are applied to retained
  text/list/message displays without recoloring graph canvases.
- Port controls in `stdGraph.tcl`, `fgraph.tcl`, `plainGraph.tcl`,
  `finspector.tcl`, `fileselect.tcl`, `spring.tcl`, `sugi.tcl`, `ortho.tcl`,
  `naiv.tcl`, `povexp.tcl`, `colorBox.tcl`, and active `xfmisc.tcl` helpers.
  Buttons, frames, labels, entries, check/radio buttons, menubuttons and
  scrollbars use ttk. Remove incompatible appearance options and old recoloring
  routines rather than overriding native theme rendering. The repeated-action
  toolbar buttons retain their callbacks and use ttk pressed states.
- Composite sliders contain actual ttk scale/label/frame widgets and retain
  their existing public paths. Preserve integer quantization, range clamping,
  captions/value displays, disabled behavior and coalesced idle callbacks.
  Programmatic updates while unmapped do not invoke commands, matching the
  classic Tk behavior verified in a separate runtime probe. Pending callbacks
  and public aliases are removed on destruction.
- Color palette buttons and previews use image-backed ttk widgets: Windows
  native button themes can ignore custom background colors. Keep the existing
  palette/RGB/HSV workflows and color-query interface. Destroy owned sample
  images and aliases with their widgets.
- The file selector uses a real ttk menubutton and its standard mouse/keyboard
  bindings. Keep the previous prepare-hidden/show-once fix. Initialize the
  checkbox variable explicitly and translate the directory scrollbar's old
  four-unit protocol into ttk fractions and `moveto`/`scroll` operations.
  Allow natural initial sizing and derive minimum width from its controls;
  screenshot inspection found clipping at the old 300-pixel width.
- Inspector helpers recognize `TLabel` and `TEntry` classes. In
  `flayout/FFOrthoInsp.cc`, synchronize explicit checkbox variables from the
  model rather than invoking removed `select`/`deselect` widget subcommands.
  The Ortho layout algorithm is unchanged.
- `tests/test_ttk_widgets.cpp`: verify themed widget classes in both graph
  projections, integer slider callbacks and disabled state, callback scripts,
  swatch pixels/commands/destruction, POV export controls, input/alert dialogs,
  inspector fields and palette selection. Also construct and operate the
  Ortho and Naive inspectors. Reject reported C++ and background Tcl errors.
- `tests/test_file_selector.cpp`: test the native menubutton at its left,
  center and indicator regions, retaining actual Linux selection/dismissal
  and ready-mapping coverage. The Windows probe exercises the standard ttk
  keyboard binding while intercepting the native modal popup.
- `CMakeLists.txt`: add the ttk regression to both full and initial suites and
  mark it as GUI coverage. Installation already copies the shared Tcl resources.
  `BUILD-CMAKE.md` and `BUILD-WINDOWS.md` describe the theme dependencies.

Tk provides no direct ttk replacement for canvas, text, listbox or menu.
Those remain, with theme colors where appropriate. Wrapped message displays
and the legacy graph-embedded label/frame drawing paths also retain Tk;
their graph colors remain data rather than application theme decoration.
Historical `.old`, `.orig`, `.new`, `.mit` and similar archival files are not
active application entry points and are not modernized.

No graph/container ownership, parser, layout algorithm, PNG/Xpm compatibility,
experimental SPQR or parked WSLg inspector behavior is changed.

## Tests actually executed and results

- Full Linux Debug suite: **94/94 passed**.
- Full Linux Release suite: **96/96 passed**.
- Final focused Release selector/widget/installed-runtime checks: **3/3 passed**.
- Focused ASan/UBSan GUI regression: **3/3 passed**. GUI leak detection was
  OFF under the project's existing sanitizer partition policy.
- Focused optional-Xpm compatibility regression: **3/3 passed**.
- New Linux GUI regressions cover the application callbacks and both
  projections. A deliberate pre-adaptation Ortho run caught the obsolete
  `select`/`deselect` calls, which were corrected and rerun.
- Linux application and selector screenshots were captured and inspected:
  `build/ttk-main.png` and `build/ttk-selector.png`. The latter was recaptured
  after fixing the observed label clipping. A private resource copy supplied
  capture/exit hooks; production resources contain no screenshot hooks.
- The first Windows initial-suite run passed 25 tests and timed out in the new
  input-dialog test. Its synthetic Return arrived without reliable Windows
  focus. The test now waits for visibility, requests focus and queues Return;
  a diagnostic watchdog bounds any further dialog wait. The focused Windows
  ttk regression then passed. No test was disabled.

- Final native Windows initial suite under Wine/Xvfb: **26/26 passed**,
  including installed-runtime resource/dependency checks and all six GUI tests.
- `git diff --check`: **passed**.
- Refreshed `build/windows-png-preview/` and
  `build/ffgraph-windows-native-debug.zip` from the production cross-build.
  Checked the staged scripts contain no private screenshot/exit hooks.

## Build and verification commands

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure --parallel 2

cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release --output-on-failure --parallel 2
ctest --preset release \
  -R '^(test_ttk_widgets|test_file_selector|test_installed_runtime)$' --output-on-failure

cmake --preset asan
cmake --build --preset asan --target \
  test_ttk_widgets test_file_selector test_image_assets --parallel 2
ctest --preset asan-gui \
  -R '^(test_ttk_widgets|test_file_selector|test_image_assets)$' --output-on-failure

cmake -S . -B build/png-compat
cmake --build build/png-compat --target \
  test_ttk_widgets test_file_selector test_image_assets --parallel 2
ctest --test-dir build/png-compat \
  -R '^(test_ttk_widgets|test_file_selector|test_image_assets)$' --output-on-failure

cmake -S . -B build/windows-cross
cmake --build build/windows-cross --parallel 2
xvfb-run -a ctest --test-dir build/windows-cross --output-on-failure --parallel 1
git diff --check

cmake --install build/windows-cross --prefix "$PWD/build/windows-png-preview"
python3 -c 'import shutil; shutil.make_archive("build/ffgraph-windows-native-debug", "zip", "build/windows-png-preview")'
```

Build directories for compatibility and Windows retain their previous options
and toolchain/emulator setup. Logs use `ttk-*.log` in the corresponding build
directories; final Windows results use `ttk-final-ctest.log`.

## Remaining risks and use

- Windows verification uses native MinGW/Tcl-Tk binaries under Wine/Xvfb,
  not a real Windows 11 desktop. Native menu option interaction and per-monitor
  scaling remain unverified. Linux Xvfb screenshots do not include desktop
  window-manager decorations or establish Wayland/WSLg appearance.
- `clam` is a bundled ttk theme, not a GTK integration package. Existing
  historical XF/X resource color overrides are superseded for themed controls;
  graph colors and palette choices remain functional.
- Tests establish lifecycle cleanup for the new slider aliases and swatch
  images, not leak freedom for the toolkit as a whole.
- Rebuild and restart the application. Installed copies need the new
  `ttk.tcl` helper and updated Tcl resources as well as the rebuilt binary.

No system installation, commit or push was performed.
