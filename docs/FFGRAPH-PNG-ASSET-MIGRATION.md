# PNG assets and optional Linux Xpm compatibility — 5 November 2026

## Problem and root cause

Bundled artwork consisted of 37 Xpm images (including a hidden `.xpm` file)
and 48 XBM images/masks. Linux enabled the old Tk pixmap extension by default,
and image selection/masking depended on that extension and legacy file paths.
Windows consequently could not display these assets. Normal builds should
instead use Tk's native PNG photo support, retaining opt-in Linux compatibility
for external Xpm files referenced by user graphs.

An existing rendering bug was reproduced before changing the node path:
`addNode` had its entire icon branch commented out. Asking Tk to add a node
with `redball.png` produced an `oval`, not an `image`. This also prevented
historical icon references from displaying correctly. The replacement restores
that branch and resolves normal/selected image pairs without the old duplicate
`createIcon` definitions and inconsistent selection cache.

## Assets and implementation changes

- Convert **all 85 bundled images** to RGBA PNG without resizing, quantization,
  or pixel changes. Color images live in `lib/ffgraph/icons/png/`; monochrome
  images/masks live in its `xbm/` subdirectory to avoid basename collisions.
  The hidden image becomes `unnamed-xpm.png`. Original Xpm/XBM files are removed
  from the resource tree; no original image is dropped.
- `scripts/convert-legacy-icons.py` is a maintenance-only, standard-library
  converter. It reads the historical C arrays, including the three-digit hex
  colors and transparent entries, and writes deterministic PNG files, a legacy
  alias table and source-raster manifest. Some original Xpm files have unrelated
  binary trailers; only their actual image arrays are converted. Empty source
  directories are rejected rather than overwriting the alias table with none.
- `tests/image_assets_manifest.tsv` records original-file SHA256 values,
  dimensions, PNG paths and decoded RGBA CRC32 values. The checksum covers
  every pixel and its alpha channel. XBM set bits become opaque black; unset
  bits become transparent, preserving Tk's original default bitmap appearance.
- `lib/ffgraph/icons/legacy-icons.tcl` and `tcl/icons.tcl` resolve old bundled
  names, extensionless normal/selected N/S pairs, and absolute old paths under
  the configured icon directory to PNG. Graph model strings and serialized
  FF/sgraph metadata remain unchanged. Known bundled names prefer their PNG
  aliases when those assets exist; custom legacy-only icon directories still
  fall back to external files.
- The image helper caches original images separately from masked variants.
  PNG alpha masks clip copies, preserving shared originals, selected images,
  mask removal and repeated-call caching. Historical extensionless/Xpm-style
  mask names prefer their one-bit XBM-derived PNG, rather than the similarly
  named opaque color image. Both masks are resolved before updating an item.
- External XBM byte arrays and X10 short arrays are converted to transparent
  Tk photos at load time, requiring no Xpm extension. With Linux compatibility
  enabled, external Xpm files still use the existing pixmap handler. For that
  handler only, PNG masks are serialized to in-memory XBM mask data. No legacy
  mask files need to be installed. The C pixmap implementation is unchanged.
- `stdGraph.tcl` uses the shared helper for image creation, node rendering,
  selection and masks. Its chooser displays the converted assets with correct
  monochrome paths and mask dimensions. It avoids duplicate legacy entries if
  an older installation still contains files that have PNG aliases. Unreachable
  pre-Tk-4 bitmap branches are removed; active script shebangs use ordinary
  `wish` instead of a custom `pixmap-wish` executable.
- `FFTclBasicWindow.cc` uses PNG for its fallback icon and object arguments
  for image/mask Tcl calls, preserving spaces and Tcl metacharacters in paths.
  The new icon is loaded before an existing canvas item is replaced.
- `CMakeLists.txt` requires Tk 8.6+, defaults `FFGRAPH_XPM` to **OFF**, and adds
  the image regression to both full and initial test suites. Xpm headers,
  library and the legacy C module are found/built only when explicitly enabled
  on Linux. Feature macros in the old config headers respect this choice.
  PNG files, aliases and helpers are installed on both platforms.
- CI's normal Linux jobs no longer install `libxpm-dev`. A separate compatibility
  job enables Xpm and checks the external-file path. Build instructions, the
  dependency inventory, README and asset README describe the new defaults.

Linux still uses X11 for Tk; removing that dependency was not part of this
patch. Windows retains its native Tk build. No graph/container ownership,
layout algorithms, parser formats, experimental SPQR or parked WSLg inspector
behavior was changed.

## Tests actually executed and results

| Verification | Result |
| --- | --- |
| Independent original raster decode using libXpm/Xlib | **85/85** match the source manifest |
| Regenerate PNGs, aliases and manifest from private original tree | Byte-for-byte identical |
| Full Linux Debug, `FFGRAPH_XPM=OFF` | **92/92 passed** |
| Full Linux Release, `FFGRAPH_XPM=OFF` | **94/94 passed**, including configured external Rome checks |
| Selected Linux compatibility suite, `FFGRAPH_XPM=ON` | **24/24 passed** |
| Focused PNG GUI regression under ASan/UBSan | Passed; GUI leak detection OFF per existing project policy |
| Native Windows cross-build and initial suite under Wine/Xvfb | **24/24 passed** |
| Fresh production Release configure/build, no Xpm option supplied | Passed; default is OFF; zero tests registered with `BUILD_TESTING=OFF` |
| `ldd` on default production binary | No `libXpm` dependency |
| `ldd` on explicit compatibility binary | `libXpm` present as intended |
| `git diff --check` | Passed |

The asset regression decodes every PNG using Tk and compares every pixel with
the frozen original checksums. It checks relative/absolute aliases, implicit
N/S pairs, transparent masks and selected variants, removal and size rejection,
unchanged shared originals, bounded masked-image caching, external XBM formats,
all chooser entries, trailing directory separators and custom Xpm files with
spaces in their paths. A real FF graph with old icon/mask strings is loaded,
displayed and repainted in 2D and 3D; save/reload retains those original strings.

The installed-runtime test checks that only PNG assets are staged, then decodes
a legacy alias's PNG in four real application launches: 2D/3D, automatic paths
and environment overrides, from a prefix containing spaces. Its private idle
hooks do not modify production scripts. Windows PE dependency inspection still
rejects X11/Xpm/MSYS2/Cygwin imports. Windows verification used the existing
MinGW64/Tcl-Tk 8.6.16 cross-toolchain and Wine 9.0, not a Windows 11 desktop.

Initial Windows test failures identified harness assumptions: Tcl's Windows
temporary-file API discarded requested suffixes, and a startup probe called
`createIcon` during a constructor's nested idle update before `ICON_LIB_DIR`
was published. Tests now rename fixtures to their real format suffix and decode
the installed PNG using its loaded alias table and runtime environment. Timeouts
also retain captured output. These failures were fixed and rerun, not disabled.
Focused checks were rerun after the final legacy mask-name refinements.

## Build and verification commands

Normal Linux build, explicitly updating an existing cache that may retain ON:

```sh
cmake --preset debug -DFFGRAPH_XPM=OFF
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure --parallel 2
cmake --preset release -DFFGRAPH_XPM=OFF
cmake --build --preset release --parallel 2
ctest --preset release --output-on-failure --parallel 2
```

Optional external Xpm compatibility (`libxpm-dev` is needed for this build):

```sh
cmake -S . -B build/png-compat -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON -DFFGRAPH_INITIAL_TESTS_ONLY=ON -DFFGRAPH_XPM=ON
cmake --build build/png-compat --parallel 2
ctest --test-dir build/png-compat --output-on-failure --parallel 2
```

Production-only default, sanitizer and Windows checks actually used:

```sh
cmake -S . -B build/png-default-production -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build/png-default-production --parallel 2
ctest --test-dir build/png-default-production -N
ldd build/png-default-production/ffgraph

cmake --preset asan -DFFGRAPH_XPM=OFF
cmake --build --preset asan --target test_image_assets --parallel 2
ctest --preset asan-gui -R '^test_image_assets$' --output-on-failure

cmake -S . -B build/windows-cross
cmake --build build/windows-cross --parallel 2
xvfb-run -a ctest --test-dir build/windows-cross --output-on-failure --parallel 1
```

The Windows build directory retains the local toolchain/emulator configuration
documented in the Windows port report; these commands are not a standalone
cross-toolchain setup. Logs are under `build/{debug,release}/png-*.log`,
`build/png-compat-*.log`, `build/png-default-*.log`,
`build/asan/png-image-ctest.log` and `build/windows-cross/png-*.log`.
The Windows Debug preview was refreshed from `build/windows-png-preview/`
into `build/ffgraph-windows-native-debug.zip`, with PNG assets and production
scripts (no test shutdown hooks).

Conversion/reproduction used a private copy of the original resource tree,
made before deleting the legacy assets:

```sh
python3 scripts/convert-legacy-icons.py \
  --source build/png-migration-originals --output lib/ffgraph/icons \
  --manifest tests/image_assets_manifest.tsv
python3 scripts/convert-legacy-icons.py \
  --source build/png-migration-originals --output build/png-regeneration \
  --manifest build/png-regeneration-manifest.tsv
```

After migration, obtain the original tree from Git history when reproducing;
the current PNG-only tree is not converter input. The converter and normal
tests require no Pillow, ImageMagick or extra Tk image-format package.

## Remaining risks and unverified work

- Real Windows 11/UCRT64 CI, interactive display scaling and font fidelity
  remain unverified. The updated GitHub workflows were not executed here.
- The existing optional Linux pixmap implementation is retained, with native
  functional coverage for external files/masks. The focused sanitizer run used
  the default PNG path; it does not establish leak freedom for Tk or audit all
  behavior of the old optional handler.
- PNG mask behavior uses transparency to keep/clip pixels. Fully opaque
  black/white color pictures are images, not one-bit masks; converted masks
  are in `png/xbm/`. External GIF/PPM support is retained but not exhaustively
  retested with every possible file variant.
- CMake preserves explicit cache choices: changing the option's default does
  not silently turn OFF an existing ON cache. Existing installations may also
  retain old resource files; this patch does not delete user-installed icons.
- The initial Windows port's narrow-string/non-ASCII path limitations remain.

No system packages were removed, and no commit or push was performed.
