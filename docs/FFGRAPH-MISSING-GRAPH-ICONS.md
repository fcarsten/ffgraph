# Missing artwork in legacy graph files

## Problem and root cause

Loading `../demographs/gd95/A95/A3.g` reported missing semantic icons such as
`INSTRUCTION_BUS_INTERFACE`, followed by `Icon and mask have different
dimensions`. The regression reproduced both messages before the fix; see
`build/debug/missing-icons-before.log`.

The graph contains 39 nodes and 57 edges and references seven distinct icon
names. No artwork matching those names was found in the current project or
the available demograph tree. They have no bundled PNG aliases. This patch
does not reconstruct that missing artwork.

The renderer already substituted the bundled 30 by 30 default image when an
icon change failed. It then applied the graph's original 14 by 14 mask to
that unrelated image, causing a second error for each node. Loading a graph
before attaching its window exposed another inconsistency: initial node
creation did not have the fallback used by icon changes.

## Files and implementation changes

- `lib/ffgraph/tcl/icons.tcl`: add a graph-rendering image-pair helper with
  explicit fallback permission. Recoverable load failures use the bundled
  default PNG and warn once per requested normal/selected pair per window
  interpreter. Remember whether each rendered item is a placeholder and skip
  the absent artwork's masks only for those items. Installing a real image on
  the item clears its placeholder state. Failure to load the default image
  remains an error.
- `lib/ffgraph/tcl/stdGraph.tcl`: append optional fallback arguments to
  `addNode` and `changeToIcon`, defaulting to strict behavior. Resolve the
  requested pair before replacing an existing canvas item. Both creation and
  icon changes record placeholder state consistently. Preserve the script's
  existing byte encoding.
- `fgraphic/FFTclBasicWindow.cc`: explicitly opt graph rendering into that
  recovery path for node creation and icon changes. Remove the duplicate C++
  fallback that could not identify placeholders to the mask code.
- `tests/test_image_assets.cpp`: extend the existing full/initial-suite
  regression with a self-contained FF_I graph using missing A95 icon names.
  Exercise live-window loading and load-before-attach in both 2D and 3D.
  Check valid image/label items, placeholder dimensions, absence of reported
  errors, warning-key deduplication, save/reload of icon/mask references and
  topology counts, repainting, and replacement with real masked artwork.
  An optional graph-file argument permits verification against the actual
  external A3 file without making the normal suite depend on that corpus.

Graph strings are retained in the model and serialized files. Ordinary image
and mask errors remain strict for direct Tcl calls; the existing regression
still rejects missing icons and incompatible mask dimensions. No parser,
ownership, layout algorithm or experimental SPQR changes are involved.

## Tests actually executed and results

- Before the fix, the extended Debug image regression failed and printed the
  reported missing-icon and mask-dimension errors.
- Updated Debug image regression: passed.
- Actual external `A3.g`: passed in all four projection/loading combinations,
  retaining its icon/mask metadata through save/reload.
- Full Linux Debug CTest suite: **92/92 passed**.
- Full Linux Release CTest suite: **94/94 passed**.
- Focused Linux test with optional Xpm compatibility enabled: passed.
- Focused ASan/UBSan GUI regression: passed. GUI leak detection was OFF under
  the project's existing sanitizer partition policy.
- Native Windows cross-build image regression under Wine/Xvfb: passed.
  A real Windows 11 desktop was not tested.
- `git diff --check`: passed.

## Build and verification commands

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure --parallel 2

FF_LIBDIR="$PWD/lib/ffgraph" FF_ICONDIR="$PWD/lib/ffgraph/icons" \
  xvfb-run -a build/debug/test_image_assets ../demographs/gd95/A95/A3.g

cmake --preset release
cmake --build --preset release --parallel 2
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

The compatibility and Windows directories retain their existing configured
options and toolchain. Logs use `missing-icons-*.log` in the relevant build
directories. Linux GUI checks use Xvfb.

## Unverified assumptions and remaining risks

- The original seven semantic images remain unavailable. Nodes use a default
  placeholder until their artwork is supplied; the warning states this
  explicitly. Graph references are preserved so subsequent loads can resolve
  installed artwork using the existing icon-directory search rules.
- This does not establish manual desktop interaction or real Windows 11
  behavior. The regression exercises the actual graph/window code under a
  virtual display.
- A missing or corrupt default image is an installation error and still
  prevents successful fallback. Masks belonging to successfully loaded images
  still require matching dimensions.
- Rebuild the C++ executable and restart the application to use both halves
  of the fix. Installed applications also need updated Tcl resources.

No system installation, commit or push was performed.
