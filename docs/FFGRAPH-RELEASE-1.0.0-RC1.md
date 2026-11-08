# Release candidate preparation — 1.0.0-rc1

Prepared 8 November 2026 from merged `main`, baseline
`3a54100869527e09b945c632ae0257b9642810c5`. No commit, tag, push or publication
was performed. Local artifacts are explicitly uncommitted previews.

## Problem and root cause

The modernized application was ready for a preservation milestone, but release
identity and distribution instructions were inconsistent. `VERSION` still held
the original 0.91 alpha notes, CMake declared 1.0, and `INSTALL` recommended
obsolete Makefiles and claimed installation was unsupported.

The installed C++ library was incomplete: only `include/ffgraph` was copied,
omitting its root compatibility and container dependencies. An external
translation unit including installed `<ffgraph/FFGraph.h>` failed with
`fatal error: globdefs.h: No such file or directory`. A separate CMake consumer
also failed because no package definition was installed.

Binary bundles lacked project/container notices and depended on optional
toolchain locations for Tcl/Tk notices. There was no repeatable source/binary
release assembly with commit provenance and checksums.

## Files and implementation changes

- `VERSION`, `CMakeLists.txt`, `ffmain.cc`: one full release identity,
  `1.0.0-rc1`; derive numeric CMake compatibility version 1.0.0 and add
  `--version` before Tcl/GUI initialization. No editing/layout behavior changes.
- Install the compatibility/container headers and `globdefs.h`, excluding
  obsolete private Tk header copies. Add relocatable
  `cmake/ffgraphConfig.cmake.in` and its version file, providing
  `ffgraph::ffgraph`, C++17, platform definitions, grouped static archives,
  generated parser archives and Tcl/Tk/X11/optional-Xpm dependencies.
- `tests/installed_library/` and `tests/test_installed_library.cmake.in`:
  stage and relocate installation into a prefix with spaces, copy a separate
  consumer outside the source directory, configure/link it using only installed
  project headers/archives, then run graph construction, headless Sugiyama and
  GML import. Windows uses the configured native toolchain or Wine emulator.
  Sanitized consumers are instrumented when testing sanitized archives.
- `README`, `INSTALL`, build documents, `RELEASE-NOTES.md`, `RELEASING.md`:
  current support, compatibility, library usage, packaging and final Windows
  desktop checks. Preserve original `VERSION`/`INSTALL` verbatim under
  `docs/historical/`. Label the superseded dependency report as historical.
- `THIRD-PARTY-NOTICES.md`, `licenses/`, install rules and
  `cmake/WindowsRuntime.cmake`: include existing project/container/legacy Tk
  notices, verbatim upstream Tcl/Tk/GCC-exception/MinGW/winpthreads/zlib notice
  snapshots, available toolchain package notices and an optional extra notice
  directory for cross-builds. Provenance URLs are in the notice inventory.
- `scripts/package-release.py` and `cmake/ReleaseInfo.json.in`: rebuild/stage
  Release configurations, check notice presence, record build information and
  installed-file hashes, create a binary archive and matching source archive,
  and write SHA256 checksums. Clean source archives use HEAD's Git objects.
  Dirty-tree official packaging is refused; `--preview` explicitly marks local
  snapshots and includes unignored working-tree changes. Output inside the
  checkout must be ignored to avoid archiving generated release files.
- `tests/test_release_packaging.py`: exercise clean Git-object archives,
  dirty-tree refusal, explicit previews, changed/untracked source inclusion and
  exclusion of ignored build files and Git internals. Register it and the
  release-version check in both full and initial suites; Git is a declared
  test prerequisite.
- Linux and Windows CI workflows assemble and upload Release artifacts only
  after successful tests. No release publication permission or action is added.

No algorithms, graph/container ownership or experimental SPQR behavior change.
The parked WSLg issue was not revisited.

## Tests actually executed and results

- Pre-fix installed-header compilation failed on missing `globdefs.h`.
- Pre-fix external CMake consumer failed to find `ffgraphConfig.cmake`.
- Post-fix relocated installed-library consumer passed on Linux Debug/Release.
- Full Linux Debug suite passed **99/99**.
- Full Linux Release suite passed **101/101**, including the configured external
  Rome corpus tests and all three new CTest registrations.
- Windows Release cross-build portable suite passed **31/31** under Wine/Xvfb,
  including installed-runtime and relocated installed-library checks.
- Strict headless ASan/UBSan/LSan passed **77/77**; GUI ASan/UBSan runtime
  checks passed **20/20**, with leak detection OFF under the existing GUI preset.
  Strict GUI toolkit-leak diagnostics were not rerun for this packaging patch.
- Optional Linux Xpm installed-runtime, installed-library and asset checks passed
  **3/3**.
- Standalone release packaging provenance regression passed.
- Official packaging from the dirty working tree was refused as intended.
- Linux and Windows preview archives, source archives, manifests and checksum
  files were generated successfully. Archive checks verified every installed
  file against its manifest, all artifact checksums, source VERSION, absence of
  Git/build internals, ZIP integrity and Linux 3D symlink preservation.
- Applications extracted from the actual archives passed **8/8** startup/icon/
  resource-path checks: Linux/Windows, 2D/3D, automatic/custom resources. Tests
  ran in a temporary directory with spaces. Private extracted script copies
  supplied idle shutdown hooks; the shipped archives contain no hooks.
- The extracted source preview configured and built Release successfully with
  `BUILD_TESTING=OFF` and profiling OFF, then printed `ffgraph 1.0.0-rc1`.
  Existing optional-export `system()` return-value warnings were visible;
  compilation/linking succeeded.
- Python syntax, the provenance regression and `git diff --check` passed.

Local artifacts are in `build/rc1-artifacts/{linux,windows}/`. Test logs include
`build/{debug,release}/rc1-ctest.log`, `build/asan/rc1-{headless,gui-runtime}.log`,
`build/windows-rc1/rc1-ctest.log` and `build/png-compat/rc1-compat.log`. Package
logs are `build/{release,windows-rc1}/rc1-package.log`. The temporary archive
smoke driver is `/tmp/ffgraph-rc1-package-smoke.py`; it exercises the same private
shutdown hook as the permanent installed-runtime test. The extracted source
build is `build/rc1-source-build`; its source path is recorded in
`build/rc1-source-verification-path.txt`.

## Build and verification commands

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure --parallel 2
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release --output-on-failure --parallel 2
cmake --preset asan
cmake --build --preset asan --parallel 2
ctest --preset asan-headless --parallel 1
ctest --preset asan-gui --parallel 1
cmake -S . -B build/png-compat
cmake --build build/png-compat --parallel 2
ctest --test-dir build/png-compat -R 'test_(installed_library|image_assets|installed_runtime)$' --output-on-failure
python3 -B tests/test_release_packaging.py
python3 scripts/package-release.py --build build/release --output build/rc1-artifacts/linux --preview
python3 scripts/package-release.py --build build/windows-rc1 --output build/rc1-artifacts/windows --preview
git diff --check
```

The local Windows Release cross-build uses the existing ignored MinGW64
toolchain and Wine wrapper described in `FFGRAPH-WINDOWS-NATIVE-PORT.md`:

```sh
cmake -S . -B build/windows-rc1 -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/build/windows-cross-toolchain.cmake" \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
  -DCMAKE_CROSSCOMPILING_EMULATOR="$PWD/build/windows-wine-run.sh" \
  "-DFFGRAPH_WINDOWS_RUNTIME_SEARCH_DIRS=$PWD/build/windows-cross;$PWD/build/windows-wine-prefix-verified/drive_c/windows/system32"
cmake --build build/windows-rc1 --parallel 2
xvfb-run -a ctest --test-dir build/windows-rc1 --output-on-failure --parallel 1
```

Local package builds also supply `FFGRAPH_WINDOWS_EXTRA_LICENSE_DIR` with the
actual Debian cross-toolchain GCC, MinGW and zlib package copyright inventories.
These are ignored build-environment files, not new source dependencies.

## Unverified assumptions and remaining risks

- Final native MSYS2/UCRT64 CI for these uncommitted changes has not run.
  The existing merged baseline has independent CI history; that does not prove
  new workflow packaging steps pass. Local Windows binaries use the existing
  MinGW64/MSVCRT cross-toolchain, not UCRT64.
- No interactive Windows 11 desktop was available. The exact final ZIP needs
  the documented manual workflow/path/display-scaling checks. Non-ASCII path
  behavior is not certified by ASCII path-with-spaces automation.
- Notice snapshots identify their upstream provenance, not a dependency lock.
  Actual compiler/runtime dependencies can change; inspect CI package inventories
  and notices before publishing. Additional DLLs need their applicable notices.
- Installed archives are static and require compatible compiler/runtime and
  development dependencies. This does not establish an ABI guarantee across
  compiler families. MSVC, ARM64, Tcl/Tk 9 and multi-config generators remain
  unsupported/unverified.
- Linux binaries depend on system libraries and target Ubuntu 24.04; no universal
  Linux binary, installer, signing or automatic GitHub publication is provided.
- Final artifacts must be regenerated from the reviewed, clean, verified commit.
  Preview artifacts identify the baseline commit plus an uncommitted source state.
