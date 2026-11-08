# Initial native Windows port — 5 November 2026

## Problem and root cause

The Linux build unconditionally discovered and linked X11/Xpm. Startup and
graph services also used POSIX-only environment, password database, timing and
random-number APIs. Resource lookup assumed Linux executable discovery, and
installation used a Unix symlink for 3D startup. `include/String.h` collides
with the standard C `string.h` on case-insensitive Windows filesystems.

The agreed scope permits omitting deprecated Xpm support and initially porting
a selected test suite. The Windows application retains native Tcl/Tk and must
have no X11 runtime dependency.

## Files and implementation

- `CMakeLists.txt`: native MinGW-w64 Windows branch, conditional X11/Xpm
  dependencies and pixmap source, Windows GUI subsystem, copied 3D executable,
  explicit initial-suite selection and Xpm asset exclusion. Linux keeps the
  full suite and Xpm by default; `FFGRAPH_XPM=OFF` permits Linux verification
  of the same image-handler configuration.
- `cmake/WindowsRuntime.cmake`: resolve and stage native runtime DLLs and
  Tcl/Tk 8.6 script libraries. Windows system DLLs are excluded. Additional
  search directories can be supplied for cross-installation.
- Rename `include/String.h` to `include/FFString.h` and update its includes.
  The String implementation is unchanged. External source using the old
  compatibility header name must update that include too.
- `include/ffgraph/FFPlatform.h`, `globdefs.h`, `include/osfcn.h`, `ffmain.cc`,
  Random/Ortho and the optional profiler: native environment/sleep support and
  the POSIX 48-bit random sequence on Windows. Linux continues calling its
  existing POSIX generator. The frozen sequence has a regression check.
- `fgraph/FFGraphClass.cc`: Windows current-user path expansion and creator
  metadata without the Unix password database. The Linux behavior is retained.
- `misc/FFRuntimePaths.cc`: Windows executable discovery using
  `GetModuleFileNameW`, portable environment publication, and non-throwing
  build-directory comparison. Verification exposed that an installed machine
  may lack the configured build directory; that must not prevent startup.
- `fgraphic/FFWindow.cc`, Tk window headers/sources, and `ffConfig.h`: omit
  Unix display synchronization and the pixmap extension on Windows. The
  remaining `Xatom.h` constants come from native Tk's compatibility headers;
  they do not require an X server or X11 library. Graph copy/paste uses the
  Windows clipboard across independent interpreters.
- `lib/ffgraph/tcl/stdGraph.tcl`: guard unsupported Xpm images and pixmap masks,
  and skip Xpm chooser entries when the handler is absent.
  `fileselect.tcl` uses Tcl `file mkdir` instead of invoking a Unix shell.
- `tests/initial/CMakeLists.txt`, parser CMake files and portable test updates:
  retain an explicitly selected initial suite, port temporary-file/resource
  tests, add native 2D/3D clipboard smoke coverage, and inspect staged native
  dependencies. The installed-runtime driver can use an external emulator for
  cross-build verification. No failing existing test is silently disabled.
- `.github/workflows/windows.yml`, `BUILD-WINDOWS.md`, and the main build
  documentation describe the native build and its verification limits.

No graph/container ownership or algorithm changes were made. Experimental SPQR
remains excluded. The parked WSLg inspector issue was not investigated.

## Tests actually executed

- Native 64-bit Windows Debug application and initial test executables
  cross-compiled successfully with Ubuntu's MinGW-w64 GCC 13 toolchain and
  official MSYS2 MinGW64 Tcl/Tk 8.6.16 libraries.
- The initial CTest suite against that Windows build passed **23/23** under
  Wine 9.0/Xvfb: 20 headless tests and 3 GUI tests. Two tests use host Python;
  the C++ executables and installed applications are Windows PE binaries.
  Windows 2D/3D canvas creation, cross-window clipboard operations and actual
  Spring/Sugiyama inspector controls passed.
- The installed-runtime test staged the Windows app in a prefix containing
  spaces, inspected the executable and all bundled DLL imports for forbidden
  X11/Xpm/MSYS2/Cygwin dependencies, then passed four genuine application
  launches outside the checkout: automatic paths and environment overrides,
  for both 2D and 3D. Development Tcl/Tk overrides and Wine's development
  DLL search path were removed for these launches. Private copied scripts
  supplied idle shutdown hooks; production scripts contain no test hook.
- Linux Debug with `FFGRAPH_XPM=OFF` and the initial suite passed **23/23**.
- The final full Linux Debug suite passed **91/91**; the full Release suite
  passed **93/93**, including the two configured external Rome corpus tests.
  Linux's ordinary Xpm extension was enabled for both full suites.
- `git diff --check` and Python compilation of the installed-runtime driver
  passed. ASan/UBSan were not run for this patch; no ownership changes were
  made. No physical/virtual Windows 11 run or GitHub workflow run occurred.

Logs are in `build/windows-cross/{configure,build,ctest-wine,install}.log`,
`build/windows-initial-linux/{configure,build,ctest}.log`, and
`build/{debug,release}/windows-port-*.log`. A fresh staged Windows Debug
preview with production scripts is at `build/windows-native-preview/`;
`build/ffgraph-windows-native-debug.zip` packages that directory.

## Build and verification commands

Native Windows prerequisites and commands are in
[BUILD-WINDOWS.md](../BUILD-WINDOWS.md). The intended native CI toolchain is
MSYS2 UCRT64 with Tcl/Tk 8.6, Debug and Release.

Linux commands:

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure --parallel 2
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release --output-on-failure --parallel 2

cmake -S . -B build/windows-initial-linux -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  -DFFGRAPH_XPM=OFF -DFFGRAPH_INITIAL_TESTS_ONLY=ON
cmake --build build/windows-initial-linux --parallel 2
ctest --test-dir build/windows-initial-linux --output-on-failure --parallel 2
```

Local Windows verification uses downloaded Ubuntu MinGW-w64 cross-tools,
official MSYS2 MinGW64 Tcl/Tk 8.6.16 headers/import libraries, and private Wine
9.0 under Xvfb. These are test-environment files under ignored `build/`, not
project dependencies or system installations. The local cross-toolchain uses
MSVCRT rather than the CI workflow's UCRT. Configuration/build commands:

```sh
cmake -S . -B build/windows-cross -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/build/windows-cross-toolchain.cmake" \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  -DCMAKE_CROSSCOMPILING_EMULATOR="$PWD/build/windows-wine-run.sh"
cmake --build build/windows-cross --parallel 2
xvfb-run -a ctest --test-dir build/windows-cross \
  --output-on-failure --parallel 1
```

The local toolchain points `pkg-config` at the Windows libraries. Cross-install
dependency inspection additionally searches the native DLL directory and the
private Wine prefix's Windows system directory through
`FFGRAPH_WINDOWS_RUNTIME_SEARCH_DIRS`. The emulator wrapper supplies Wine's
private loader, builtin-DLL directory, prefix and development Tcl/Tk libraries.
It removes development-library overrides for installed-app checks. Wine's
private registry maps TEMP/TMP to `Z:\tmp` so Linux-hosted build paths remain
valid after the Windows resource test changes directory. Initial Wine setup
and harness failures were diagnosed rather than treated as application passes.

## Unverified assumptions and remaining risks

- No physical or virtual Windows 11 desktop was available. Wine execution and
  MinGW cross-compilation do not establish Windows 11 GUI behavior, UCRT64 CI
  success, display scaling, font fidelity or Explorer/file-association behavior.
- The new Windows GitHub workflow has not been executed or published. Nothing
  was committed or pushed.
- Windows support initially targets 64-bit MinGW-w64 and Tcl/Tk 8.6, using
  Ninja. MSVC, ARM64, Tcl/Tk 9 and multi-config layouts are unverified.
- Non-ASCII Windows filenames, non-ASCII environment overrides, very long paths,
  and optional exports/legacy auxiliary tools need further Windows validation.
  Executable discovery uses the wide Windows API, but existing graph I/O and
  environment string boundaries still use narrow strings.
- The Xpm and pixmap-mask omission is intentional. Other image types and all
  interactive application features have not received exhaustive Windows testing.
- Windows ACLs, antivirus behavior, installer/signing integration and a
  distribution license inventory are outside this initial build/runtime check.
