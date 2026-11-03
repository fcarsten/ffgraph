# Runtime resources and production-only build — 3 November 2026

## Problems and changes

The declared CMake 3.21 minimum did not cover the LINK_GROUP generator expression
already used by the project. Local CMake's primary generator-expression manual
identifies LINK_GROUP as added in 3.24. Root CMake and presets now require 3.24.
The standalone GML-only project has no LINK_GROUP dependency and retains its own
minimum. Verification used the installed toolchain; a CMake 3.24 binary was not
separately executed.

The app's default resource macro pointed at `./lib/ffgraph`, so launching from
outside the checkout/install working directory could fail. `ffmain.cc` now calls
the private `misc/FFRuntimePaths` helper before registering default layouts.
On Linux it locates the running binary via `/proc/self/exe`. Build-tree binaries
use the configured source resource directory; installed binaries use the configured
bin-to-library relative path. The latter supports relocation/install-time prefixes
without falling back to resources in a coincidentally available checkout.
FF_LIBDIR/FF_ICONDIR overrides are retained; an unset icon path defaults to the
chosen resource directory's icons. The generated config header is app-private;
library APIs, parser formats and command-line graph dispatch are unchanged.

The Sugiyama Tcl script now preserves its configured FF_LIBDIR instead of
overwriting it with a relative path. Its existing standalone fallback remains.
Actual inspector tests assert Tcl helpers/widgets load, not merely that the
C++ recall command exists. This is resource lookup work, not the parked WSLg
visibility investigation.

The ffgraph3D install symlink previously embedded the configure-time prefix and
DESTDIR too early. Its install code now evaluates install-time prefix/DESTDIR and
creates a relative link to ffgraph in the installed binary directory. Other
library/header/resource installation destinations remain as before.

## Tests actually executed

- `test_runtime_paths` launches from `/tmp`, verifies automatic build-tree
  resource lookup and independent environment overrides without creating Tk.
- `test_installed_runtime` stages installation in a private prefix containing
  spaces, verifies the relative ffgraph3D link, and launches both actual programs
  outside the checkout. Two automatic-path and two custom-override launches pass
  per build. Private staged Tcl scripts append an idle shutdown/result hook;
  production scripts contain no test hook. Logs reject Tcl resource errors and
  assert the resolved directories are the staged/custom resources.
- The full final suites passed: Debug **75/75**, Release **77/77**, strict
  headless sanitizers **63/63**, GUI runtime sanitizers **12/12**, including the
  staged runtime test in all three builds. GUI leak detection remains OFF in
  the runtime partition; this does not establish toolkit leak freedom.
- A **fresh Release BUILD_TESTING=OFF / profiler-OFF build** configured and
  built successfully in `build/production-portability-check`. It generated all
  modern parser/scanner libraries, registered **zero tests**, and passed the same
  four installed-runtime launches separately.
- Additional installation with DESTDIR under the build tree and install-time
  prefix `/opt/ffgraph stage` passed path/resource/symlink assertions. It did not
  write to the real `/opt` directory.

## Commands and files

```sh
cmake -S . -B build/production-portability-check -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DFFGRAPH_BUILD_PROFILER=OFF
cmake --build build/production-portability-check --parallel 2
ctest --test-dir build/production-portability-check -N
xvfb-run -a python3 tests/test_installed_runtime.py \
  --build "$PWD/build/production-portability-check" --libdir lib
env DESTDIR="$PWD/build/production-destdir-check" \
  cmake --install build/production-portability-check --prefix '/opt/ffgraph stage'
ctest --preset debug -R 'runtime_paths|installed_runtime|layout_controls_gui'
```

Implementation: `CMakeLists.txt`, `CMakePresets.json`, `ffmain.cc`,
`misc/FFRuntimePaths.{h,cc}`, its configured header template and the Sugiyama Tcl
resource guard. Tests: `tests/test_runtime_paths.cpp`,
`tests/test_installed_runtime.py`, plus the inspector-control test. Logs and
metadata are under `build/remaining-steps-checkpoint/`; staged per-launch logs
are in each build's `runtime-check-*` directories.

No system install, commit, push or dependency upgrade was performed. Other OSes,
custom runtime-output layouts, multi-config generators, arbitrary camera/font/icon
choices and interactive installed File/Load workflows were not separately tested.
The staged hooks establish actual startup/resource resolution; existing GUI
workflow tests cover loading separately. A newer complete Tcl/Tk/font stack and
strict GUI leak diagnostics remain follow-ups under the existing policy.
