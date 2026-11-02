# Pass 12 — verification hardening before Rome migration

Date: 1 November 2026. Branch: `pass12-modern-ff-parser`.
Starting commit: `f033d04712be2e1643980de5e5bd486c43ced6e7`; working tree clean.
No commits, tags or pushes were made. The existing pre-FF-retirement tag was
left unchanged.

## Problem and implementation

The existing Release suite passed 48/48, but twenty older test sources still
used C assert. NDEBUG removed their checks and sometimes the operations being
tested. In `test_tcl_compat.cpp`, Tcl evaluations and list extraction vanished,
so the test could pass without exercising its intended behavior.

All twenty affected test sources now include the existing `test_check.h` and
use FF_CHECK (178 converted checks). No production assertion policy or compiler
optimization flags changed. Existing test operations are now evaluated once in Release too.
`test_check_ndebug.cpp` is compiled explicitly with NDEBUG even in Debug; its
CMake driver checks a side effect and requires a child process with a false
check to abort and print the expected diagnostic.

A mutation experiment independently established the gap: a temporary copy of
the old Tcl test with `assert(count == 999)` compiled with `-O3 -DNDEBUG` exited
0. The corresponding `FF_CHECK(count == 999)` copy aborted with exit -6 and
`check failed: count == 999`. These temporary files were not added to the repo.

Root CMake and both parser test subdirectories now assign every registered
test exactly one label, headless or gui. The root GUI list includes the optional
large workflow test. Parser subdirectories also label standalone GML tests.
The current default partition is 43 headless and 7 GUI tests. A local CTest JSON
audit checked every label against its actual Xvfb command in all three builds.

`CMakePresets.json` adds a Release test preset and explicit sanitizer partitions:

- `asan`: full strict ASan/UBSan, including LeakSanitizer.
- `asan-headless`: strict headless subset.
- `asan-gui`: GUI runtime checks with leak detection disabled explicitly.
- `asan-gui-leaks`: strict GUI leak diagnostics.

CI now builds/tests Debug and Release. Headless leaks and GUI ASan/UBSan runtime
errors remain mandatory failures. Only the separately named GUI leak diagnostic
step permits failure; distinct output logs are uploaded as artifacts. The
manual GML workflow uses the same separation. No test was removed, disabled,
given WILL_FAIL, or covered by a leak suppression. A GUI runtime pass is not a
claim of leak freedom. The diagnostic job is deliberately not the sole GUI
sanitizer check.

The new Rome characterization also exposed two genuine scanner ownership bugs.
These were fixed with narrow scanner cleanup, documented separately in
`FFGRAPH-PASS12-ROME-BASELINE.md`. Rome grammar/selection and modern FF/GML
implementations remain unchanged. `BUILD-CMAKE.md` documents the commands;
`FFGRAPH-NEXT-SESSION-PLAN.md` replaces the stale Pass 8 starting recommendation
with the current status and staged Rome/installation/dependency work.

## Tests actually executed

Environment: Ubuntu 24.04, GCC 13.3.0, CMake 3.28.3, Ninja, GNU Bison 3.8.2,
Flex 2.6.4, system Tcl/Tk 8.6.14, Xft 2.3.6, fontconfig 2.15.0.
GUI and LeakSanitizer runs required execution outside sandbox display/tracing
restrictions. Tests used real Xvfb, not mocked Tcl/Tk.

| Check | Result |
|---|---|
| Unmodified Release baseline | 48/48 passed; assertion gap independently demonstrated |
| Debug configure/build and final full CTest | **50/50 passed** |
| Release configure/build and final full CTest | **50/50 passed**, checks active |
| Strict headless ASan/UBSan/leaks | **43/43 passed** after Rome cleanup |
| GUI ASan/UBSan runtime checks, system Tk, leaks off | **7/7 passed** |
| Strict GUI ASan/UBSan/leaks, system Tk | **3/7 passed**, four known toolkit leak failures |
| Final full strict ASan/UBSan/leaks | **46/50 passed**, same four toolkit failures only |
| Label partition and Xvfb membership audit | PASS for Debug/Release/ASan; 43 + 7 = 50 |
| Workflow YAML parse, preset parsing, git diff --check | PASS locally |
| Remote GitHub Actions | **Not executed**; workflow edits are locally validated only |

No runtime invalid-access ASan or UBSan error appeared in the final strict run.
The four system toolkit leak totals remain unchanged:

| Test | Leaked bytes / allocations |
|---|---|
| test_load_window_lifetime_pass11 | 9,367 / 304 |
| test_ff_window_oracle_pass12 | 53,396 / 1,970 |
| test_gui_file_workflow_pass11 | 59,697 / 2,208 |
| test_ff_gui_file_workflow_pass12 | 59,697 / 2,208 |

## Isolated newer Tk experiment

The official [Tcl/Tk 8.6 release page](https://www.tcl-lang.org/software/tcltk/8.6.html)
identifies 8.6.18 as a supported patch release. The official GitHub Tk tag
`core-8-6-18` was downloaded and built entirely under `/tmp`, using system Tcl
8.6.14 and the existing Xft/fontconfig libraries. No system package, application
dependency selection or shutdown finalizer changed.

Downloaded archive SHA-256:
`8c1d775126c39124c89752737c32c23636b3cd308d0445977aec58cfbc42c46c`.
This records the archive actually tested; it is not an upstream signature check.
`ldd` verified that the reproducer loaded the isolated libtk and system libtcl.

The existing dependency-only `repro_tk_menu_font_leak.cpp` successfully destroyed
the menu/root and deleted its interpreter once with Tk 8.6.18. Strict LSan still
reported **320 bytes in 3 fontconfig allocations**, reduced from the previously
measured 9,359 bytes / 304 allocations with system Tk. Running all seven GUI
tests against isolated Tk 8.6.18 preserved their functional checks, with the
same four tests failing strict leaks, each now **320 bytes / 3 allocations**.
There were no ASan/UBSan runtime errors in this experiment.

A temporary dependency-only variant calling FcFini after Tcl_Finalize passed
strict sanitizers with no leak report. That attributes the residual cache in
this reproducer; it does not establish a safe application-wide finalization
policy. This variant is not part of production or committed tests. No broad
suppression, upgrade requirement, font workaround or global finalizer was added.

The first plain `make` attempt failed on a documentation macro missing from
the installed Tcl development package. Building `binaries libraries` and
installing only binaries, libraries and headers succeeded. The first reproducer
launch selected the old system tk.tcl and failed version matching; selecting
the matching TK_LIBRARY resolved it. Those setup failures were not successful
leak tests and are not counted above. Vendor Tk's own tests and a fully updated
Tcl/fontconfig stack were not tested.

## Reproduction commands

Executed from the repository root:

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug --parallel 2
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release --parallel 2
cmake --preset asan
cmake --build --preset asan --parallel 2
ctest --preset asan-headless --parallel 1
ctest --preset asan-gui --parallel 1
ctest --preset asan-gui-leaks --parallel 1  # Fails on the four toolkit leaks
ctest --preset asan --parallel 1            # Same four failures, full strict suite
git diff --check
```

Isolated dependency build and original reproducer, as executed:

```sh
curl --fail --location --max-time 40 \
  https://codeload.github.com/tcltk/tk/tar.gz/refs/tags/core-8-6-18 \
  --output /tmp/ff-tk-8.6.18.tar.gz
mkdir -p /tmp/ff-pass12-toolkit-8.6.18
tar -xzf /tmp/ff-tk-8.6.18.tar.gz -C /tmp/ff-pass12-toolkit-8.6.18
cd /tmp/ff-pass12-toolkit-8.6.18/tk-core-8-6-18/unix
./configure --prefix=/tmp/ff-pass12-toolkit-8.6.18/install \
  --with-tcl=/usr/lib/x86_64-linux-gnu --enable-symbols
make -j2 binaries libraries
make install-binaries install-libraries install-headers
cd /home/carsten/work/ffgraph
c++ -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  tests/repro_tk_menu_font_leak.cpp -I/usr/include/tcl8.6 \
  -L/tmp/ff-pass12-toolkit-8.6.18/install/lib \
  -Wl,-rpath,/tmp/ff-pass12-toolkit-8.6.18/install/lib \
  -ltk8.6 -ltcl8.6 -o /tmp/ff-repro-tk-8.6.18
TK_LIBRARY=/tmp/ff-pass12-toolkit-8.6.18/install/lib/tk8.6 \
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
xvfb-run -a /tmp/ff-repro-tk-8.6.18
TK_LIBRARY=/tmp/ff-pass12-toolkit-8.6.18/install/lib/tk8.6 \
LD_LIBRARY_PATH=/tmp/ff-pass12-toolkit-8.6.18/install/lib \
ctest --preset asan-gui-leaks --parallel 1
```

Ephemeral logs: `/tmp/ff-verification-*.log`, including separate Debug, Release,
headless, GUI runtime, strict GUI, full strict and isolated-toolkit results.

## Remaining risks

Full strict sanitizer testing remains red on the system GUI toolkit. Isolated
Tk 8.6.18 is evidence for a later dependency checkpoint, not a supported-stack
upgrade completed here. Ubuntu's CI runners may have different library patch
levels and leak totals. Remote artifact upload and workflow execution remain
unverified. Optional large GUI/layout stress profiles, interactive desktop
use, alternate compilers/platforms and installed runtime launch outside the
checkout were not exercised in this checkpoint. The new Rome fixtures are
synthetic and do not replace a representative external Rome graph collection.
