# ffgraph — build, runtime and development dependencies

**Status:** Baseline dependency inventory based on `ffgraph-current.zip` and the subsequent patches discussed through Priority 2 Pass 7, **not a complete snapshot of the user's latest Git checkout**. Check current CMake declarations and the current Ubuntu machine before treating this as authoritative. Last reviewed: 1 November 2026.

## 1. Recommended source of truth

Use three complementary records instead of manually pinning every library in a README:

1. **CMake files (tracked):** `CMakeLists.txt` defines the dependencies and requirements the compiler/linker actually uses; `CMakePresets.json` defines supported configurations. Detect dependencies using `find_package()`/`pkg_check_modules()`/imported targets, fail with useful messages when required packages are missing, and avoid hard-coded `/usr/include/...` or `/usr/lib/...` paths.
2. **Human setup instructions (tracked):** this document or `docs/BUILDING.md` provides OS-specific package installation commands, compile/run/test/install workflows and runtime assets. Keep it synchronized with CMake and CI.
3. **Actual machine snapshot (generated):** capture package versions, compiler/CMake/Ninja versions, `pkg-config` results, CMake cache locations and `ldd` output from a *successful build*. Store in issue artifacts, release notes or CI logs. Treat it as evidence of a known-working environment, **not** as an exact-version lock file for everyone.

For stronger reproducibility, add an Ubuntu 24.04 CI job or container image referenced by digest, with a documented apt snapshot if exact bit-for-bit reproduction is required. Ubuntu package versions generally receive security/maintenance updates, so fixed version numbers in general build instructions become stale quickly.

## 2. Dependency inventory

| Component | Use | Ubuntu package(s) / discovery | Required? |
|---|---|---|---|
| GCC/g++ and standard libraries | Compile C/C++17, link application and static libraries | `build-essential` (CMake: C/CXX compilers) | Yes |
| CMake | Configure/build/install and CTest integration | `cmake` | Yes for current workflow |
| Ninja | Default generator in supplied presets | `ninja-build` | Yes for presets using Ninja; Unix Makefiles generator is an alternative |
| pkg-config | Discover Tk's compiler/linker settings | `pkg-config`; `pkg_check_modules(TK REQUIRED IMPORTED_TARGET tk)` | Yes with current CMake |
| Tcl development files + runtime | Tcl interpreter, script and object APIs | `tcl-dev` / underlying `tcl8.6-dev`; linked via Tk/pkg-config | Yes |
| Tk development files + runtime | Original graphical interface | `tk-dev` / underlying `tk8.6-dev`; `pkg-config tk` | Yes |
| X11 headers/libraries | Xlib/Tk GUI | `libx11-dev`; `find_package(X11 REQUIRED)` | Yes |
| Xpm | Pixmap handling and custom Tk image module | `libxpm-dev`; CMake `find_library(Xpm)` and `find_path(X11/xpm.h)` | Yes in current CMake |
| GNU libm / dynamic-loader support | Math, dynamic loading on applicable platforms | System libc development package; `find_library(m)` / `find_library(dl)` in CMake | Conditional/system |
| Git | Development/patches | `git` | Strongly recommended |
| Xvfb and xauth | Display-backed automated Tk tests without a desktop | `xvfb`, `xauth` | Optional for headless GUI tests |
| Valgrind | Additional leak/memory-error investigation | `valgrind` | Optional for development |
| Sanitizers | AddressSanitizer and UndefinedBehaviorSanitizer via GCC/Clang | Compiler runtimes (e.g. supplied with GCC) | Optional (`asan` preset) |
| Bison/Flex | Future grammar/scanner regeneration | `bison`, `flex` | **Not currently required** to build checked-in generated sources |
| WSL2/WSLg | Windows-hosted Linux GUI (user environment) | Windows WSL/WSLg installation, not an ffgraph source dependency | Only for WSL workflow |

**Important version caveat:** The original CMake file says `cmake_minimum_required(VERSION 3.21)`, but the later static link-order workaround uses `$<LINK_GROUP:RESCAN,...>` which requires **CMake 3.24 or newer**. Check that the current `CMakeLists.txt` declares a compatible minimum and verify the minimum on a clean Ubuntu setup. CMake 3.28 is available in Ubuntu 24.04's standard packaging at initial release; use the installed version as the actual reference rather than assuming it remains unchanged.

The source has locally implemented STL-era compatibility containers; **legacy `libg++` is not a current dependency**. The old `INSTALL` file's 1990s compiler, Tcl and Tk version requirements are historical notes, not instructions for this uplift.

### Ubuntu 24.04: baseline setup

```bash
sudo apt update
sudo apt install --no-install-recommends \
  build-essential cmake ninja-build pkg-config \
  tcl-dev tk-dev libx11-dev libxpm-dev git
```

Optional testing/development:

```bash
sudo apt install --no-install-recommends \
  xvfb xauth valgrind gdb
```

Optional future parser development:

```bash
sudo apt install bison flex
```

CMake and Tk should discover headers via imported targets. For example, Ubuntu Tcl/Tk headers commonly reside at `/usr/include/tcl8.6` or `/usr/include/tcl`; **do not copy them into the source tree**. In this project `PkgConfig::TK` needs to propagate include paths to every target that directly or transitively includes Tcl/Tk headers.

## 3. Supported build and test workflow

From a clean checkout:

```bash
cmake --preset debug
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure

cmake --preset release
cmake --build --preset release --parallel

cmake --preset asan
cmake --build --preset asan --parallel
ctest --preset asan --output-on-failure
```

The `asan` configuration uses GCC/Clang AddressSanitizer and UndefinedBehaviorSanitizer; it is a test configuration, not a production release build. Some Tk tests may need `xvfb-run` and can be conditionally registered when Xvfb is installed. The compiler/linker static-library dependency graph should be managed through target dependencies or `LINK_GROUP:RESCAN` when circular dependencies make it necessary.

To test installation without modifying a shared system prefix:

```bash
cmake -S . -B build/install -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$HOME/.local/opt/ffgraph"
cmake --build build/install --parallel
cmake --install build/install
```

Uninstall a **dedicated** test prefix with `rm -rf "$HOME/.local/opt/ffgraph"` after verifying its path. For shared prefixes, consult `build/install/install_manifest.txt` and remove only the relevant installed paths; avoid deleting the shared prefix.

## 4. Runtime resources are dependencies too

The application isn't just a binary: it needs Tcl/Tk scripts and icon/image resources from `lib/ffgraph`, installed via CMake (historically `lib/ffgraph/tcl` and `lib/ffgraph/icons`). They are **repository assets**, not apt packages.

The source supplied earlier showed:

```cpp
#define FF_LIBDIR "./lib/ffgraph"
```

and honours `FF_LIBDIR` (Tcl scripts) and `FF_ICONDIR` (icons) environment overrides. If that default persists in the current source, running an installed executable from a different current working directory may fail to find its scripts/icons even though `cmake --install` succeeded. **Validate installation by launching from outside the repository**, with and without those environment overrides. A robust eventual solution is to configure runtime data paths from the installation prefix or locate resources relative to the installed executable while retaining overrides.

The `ffgraph3D` launcher may be installed as a symlink to `ffgraph`; validate it along with the primary executable. Ensure any other Tcl/Tk image handlers and fonts/images are installed.

## 5. Capture exact dependencies on a known-working developer machine

Run the companion script from the repository root:

```bash
bash scripts/capture-build-environment.sh
```

It writes `build/dependency-report/` containing versions and dependency-discovery data. Check those files before sharing, because machine paths, Git metadata and platform identifiers may be present. Do **not** put local cache files, source path names, system headers, copies of `/usr/lib`, or a raw `dpkg` database in the repository.

Useful manual checks:

```bash
cmake --version
ninja --version
g++ --version | head -1
pkg-config --modversion tcl tk x11
pkg-config --cflags --libs tk
ldd build/debug/ffgraph
# Get the exact installed Debian package versions:
dpkg-query -W -f='${Package} ${Version}\n' \
  build-essential cmake ninja-build pkg-config \
  tcl-dev tk-dev tcl8.6-dev tk8.6-dev libx11-dev libxpm-dev
```

When investigating system header paths, capture the **resolved flags** in `compile_commands.json` (configure with `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`) and CMake's dependency discovery, rather than checking paths into CMake.

### Stable repository files vs. ephemeral environment evidence

**Commit:** CMake files, presets, `docs/BUILDING.md`/dependency instructions, regression fixtures, CI workflow (and perhaps a maintained Ubuntu container recipe).

**Capture alongside a bug/CI run:** `git rev-parse HEAD`, `cmake --version`, `g++ --version`, `pkg-config --modversion`, output of `dpkg-query`, relevant CMake cache entries, `ldd`, `ctest` results, and WSL/WSLg versions when diagnosing a graphical issue. This allows others to match the environment without forcing everybody onto exactly the same patched distro libraries.

## 6. Optional stronger packaging choices

- **CI first (recommended):** an Ubuntu 24.04 job that installs declared packages, configures CMake, builds, tests and stages installation. The CI run and container image establish a documented known-working reference environment.
- **Debian source package (if shipping to Ubuntu users):** maintain `debian/control` with `Build-Depends` and package dependencies. This gives apt-native dependency resolution and integrates distro packaging; do not introduce solely to replace simple developer instructions.
- **CMake find modules / imported targets:** improve dependency detection; distinguish link dependencies visible in public headers from `PRIVATE` runtime internals. Current broad `PUBLIC PkgConfig::TK` is a pragmatic uplift and can be refined later.
- **A container image:** useful for repeatable command-line builds, but GUI/WSLg support, X sockets and host display integration require extra treatment. Do not pretend a headless container proves real desktop behaviour.
- **vcpkg/Conan:** not necessary just to handle Tcl/Tk/X11/Xpm on Ubuntu. Consider only if cross-platform source builds or wider dependency graphs justify them.

## 7. Maintenance checklist when dependencies change

1. Update and test `CMakeLists.txt` (correct minimum versions and imported targets).
2. Update the apt packages in this document and the CI job.
3. Validate a **clean** configure/build/test/install in the new environment.
4. Capture environment versions as a build artifact and record relevant compatibility changes in release notes.
5. Verify GUI functionality and runtime assets, not just static-library linking.
6. Retain license notices for bundled or third-party code; distinguish redistributable project files from system-provided headers and libraries.

**Known documentation debt:** The older `INSTALL` file states installation isn't supported and recommends `make`, contradicting the now-working CMake install path. Review and modernise it in Pass 12.
