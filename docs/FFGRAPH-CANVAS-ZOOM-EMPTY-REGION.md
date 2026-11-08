# Canvas Zoom with missing or degenerate bounds

## Problem and root cause

The reported `can't read "sx1": no such variable` stack trace was reproduced
exactly by drawing a Zoom rectangle on a fresh empty graph. Its canvas has an
empty `-scrollregion`: there are no graph items from which `adjWindow` can
derive bounds. The temporary drag rectangle does not carry graph-content tags.

After scaling, `button1Release` called `centerCanvas`. That helper scanned the
empty region as four integers without checking whether any values were read.
Consequently `sx1` was unset when the centering expression used it. The error
also interrupted the normal drag cleanup. The same unchecked scan exists in
the initial repository commit, `cd7d7f6`; this is independent of the ttk port.

The adjacent zoom math also divided by rectangle width and height without
handling zero extents. Tcl floating division returns `Inf` for a positive
value divided by zero. A click with neither extent could therefore feed an
infinite scale factor to the camera. For a single-axis drag the old minimum
calculation selected the finite factor, so that useful behavior is preserved.

## Files and implementation changes

- `lib/ffgraph/tcl/stdGraph.tcl`: `centerCanvas` returns normally when no
  four-coordinate region exists. Read the region as a Tcl list and convert its
  valid Tk screen distances with `winfo fpixels`, supporting fractional and
  unit-suffixed coordinates instead of scanning integers. Center each axis only
  when its span is positive. Remove unused viewport-fraction calculations.
- In the same file, ignore a Zoom click with no extent. For horizontal or
  vertical drags, compute the existing finite factor directly from the nonzero
  dimension; otherwise retain the usual aspect-preserving minimum. Keep common
  drag cleanup and the return to Create mode. Remove obsolete commented debug
  calculations in the affected branch. Preserve the file's byte encoding.
- `tests/test_canvas_zoom.cpp`: create actual 2D/3D windows and drive the real
  background press/motion/release callbacks. Cover the fresh empty-canvas
  reproduction, no-op clicks, finite single-axis zooming, drag-state cleanup,
  and normal zooming of populated graphs without changing model coordinates.
  Check fractional, screen-unit, zero-width and zero-height scroll regions.
  The numerical centering oracle removes decorative canvas insets and allows
  normal Tk pixel rounding; production inset behavior is unchanged.
- `CMakeLists.txt`: register the new regression in both full and initial test
  suites and label it as a GUI test. Linux uses Xvfb; Windows uses native Tk.

No production C++, graph/container ownership, layout algorithm, parser,
experimental SPQR or parked WSLg inspector behavior is changed.

## Tests actually executed and results

- New Debug regression before the fix: **failed with the exact reported
  stack trace**, including `centerCanvas` and the Zoom arm of `button1Release`
  (`build/debug/canvas-zoom-before.log`).
- Focused corrected Zoom regression: **passed** in both projections.
- Full Linux Debug suite: **95/95 passed**.
- Full Linux Release suite: **97/97 passed**.
- Native Windows cross-build Zoom regression under Wine/Xvfb: **passed**,
  including all cases in both projections.

- Focused ASan/UBSan Zoom and renderer-coordinate checks: **2/2 passed**.
  GUI leak detection was OFF under the existing sanitizer partition policy.
- Refreshed `build/windows-png-preview/` and
  `build/ffgraph-windows-native-debug.zip`; checked the staged script includes
  the empty-region guard before packaging.
- `git diff --check`: **passed**.

## Build and verification commands

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug -R '^test_canvas_zoom$' --output-on-failure
ctest --preset debug --output-on-failure --parallel 2

cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release --output-on-failure --parallel 2

cmake --preset asan
cmake --build --preset asan --target \
  test_canvas_zoom test_renderer_coordinates_gui --parallel 2
ctest --preset asan-gui \
  -R '^(test_canvas_zoom|test_renderer_coordinates_gui)$' --output-on-failure

cmake -S . -B build/windows-cross
cmake --build build/windows-cross --parallel 2
xvfb-run -a ctest --test-dir build/windows-cross \
  -R '^test_canvas_zoom$' --output-on-failure

cmake --install build/windows-cross --prefix "$PWD/build/windows-png-preview"
python3 -c 'import shutil; shutil.make_archive("build/ffgraph-windows-native-debug", "zip", "build/windows-png-preview")'
git diff --check
```

The Windows directory retains its existing cross-toolchain and Wine emulator.
Logs use `canvas-zoom-*.log` in the respective build directories.

## Unverified assumptions and remaining risks

- The reproduction establishes one concrete trigger for the reported stack;
  the user's original graph/state was not available. Other callers can also
  reach `centerCanvas` with an empty region and receive the same safe return.
- Windows verification uses native Tk under Wine, not a real Windows 11
  desktop. GUI tests call the registered handler procedures directly rather
  than automating physical pointer input.
- Existing valid zoom factors and graph/camera policies are preserved. This
  does not redesign centering, empty-graph zoom policy or extreme zoom limits.
- Restart the application to load the updated script. Installed copies need
  an updated `lib/ffgraph/tcl/stdGraph.tcl`, or the refreshed Windows ZIP.

No system installation, commit or push was performed.
