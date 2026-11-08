# Interactive icon-node creation — 5 November 2026

## Problem and root cause

Selecting Node Shape > Icon, choosing an image and creating a node produced
`tagOrId "" doesn't match any items` while constructing its label.

The menu sets Tcl's `DefShape` and the C++ creation mode to Icon. The
`controlButton1` callback calls C++ `recall newNode`, which first calls
`graph->addNode()`. That initializes a plain graphic node and creates its
canvas item and label. Only after this initialization returns does C++ apply
the selected icon and masks.

During that first drawing step the node's own icon is still `__NoIcon__`,
so Tcl calls `addDefFigure`. That routine handled geometric shapes but had no
Icon case. It returned an empty canvas ID, causing the label error and also
preventing the later icon assignment from finding a valid node item.

The regression reproduced the exact message twice before the fix, using the
actual menu, chooser bindings and creation callback. Historical pre-PNG code
also had its intended Icon-mode temporary-circle fallback commented out. This
is a drawing/initialization mismatch, not a PNG decoding failure. Previous
asset coverage tested pre-labelled graphs and explicit image creation but
missed the interactive creation order.

## Files and implementation changes

- `lib/ffgraph/tcl/stdGraph.tcl`: `addDefFigure` uses a local Circle fallback
  when the selected mode is Icon. It creates a valid temporary item and label;
  the existing C++ code subsequently replaces that item with the chosen image
  and applies the masks. The user's `DefShape` stays Icon for subsequent nodes.
- `tests/test_icon_node_creation.tcl`: invokes the real Icon menu and chooser
  bindings, selects normal/selected PNG images and masks, calls the canvas
  creation callback twice, checks that Icon mode remains selected, then switches
  to Square and creates an ordinary node. Deterministic current-item tags and
  binding substitutions avoid dependencies on mouse position or chooser scroll.
- `tests/test_image_assets.cpp`: runs that workflow in both 2D and 3D windows.
  It rejects reported errors, checks node counts, nonempty live image/label IDs,
  shape and icon/mask metadata, and correct image versus rectangle item types.
  This extends the existing image regression in both full and initial suites.

No graph/container ownership, node publication order, parser formats, image
conversion, experimental SPQR or parked WSLg inspector behavior was changed.
The earlier PNG migration changes in the working tree are retained.

## Tests actually executed and results

- Before the fix: the extended Debug `test_image_assets` failed with the exact
  empty-ID error, recorded in `build/debug/icon-creation-before.log`.
- After the fix: the extended Debug image regression passed, including both
  projections, repeated creation and switching back to a geometric shape.
- Full Linux Debug suite: **92/92 passed**.
- Full Linux Release suite: **94/94 passed**.
- Focused optional-Xpm compatibility and ASan/UBSan GUI runtime checks passed.
  GUI leak detection is OFF under the existing sanitizer partition policy;
  this does not establish toolkit leak freedom.
- Native Windows cross-build/Wine image regression: passed, including the
  interactive creation workflow in both projections. No real Windows 11
  desktop or manual pointer test was run.

## Build and verification commands

```sh
cmake --preset debug
cmake --build --preset debug --target test_image_assets --parallel 2
ctest --preset debug -R '^test_image_assets$' --output-on-failure
ctest --preset debug --output-on-failure --parallel 2

cmake --preset release
cmake --build --preset release --target test_image_assets --parallel 2
ctest --preset release --output-on-failure --parallel 2

cmake -S . -B build/png-compat
cmake --build build/png-compat --target test_image_assets --parallel 2
ctest --test-dir build/png-compat -R '^test_image_assets$' --output-on-failure

cmake --preset asan
cmake --build --preset asan --target test_image_assets --parallel 2
ctest --preset asan-gui -R '^test_image_assets$' --output-on-failure

cmake -S . -B build/windows-cross
cmake --build build/windows-cross --target test_image_assets --parallel 2
xvfb-run -a ctest --test-dir build/windows-cross \
  -R '^test_image_assets$' --output-on-failure
```

The compatibility and Windows build directories retain their previously
configured options/toolchains. `icon-creation-*.log` files in each of those
build directories contain configuration, build and test results. Linux GUI
tests ran under Xvfb, with access to its local display socket.

## Remaining risks and use

This fix covers the reported creation failure. Manual desktop input and real
Windows 11 execution remain unverified; tests execute the registered callbacks
directly. Other independent icon issues are not established as fixed by this
regression.

Restart a build-tree application to load the updated Tcl script. An installed
application needs its installed Tcl resources updated through installation.
No system installation, commit or push was performed.
The staged Windows preview and `build/ffgraph-windows-native-debug.zip` were
refreshed with the corrected production Tcl script.
