# Application default-layout ownership — 2026-11-03

## Finding and root cause

Manual `asan.log` reported **2,238 bytes in 16 allocations** after the user
suppressed fontconfig/freetype allocations. All remaining allocations belong
to six ffgraph startup layout prototypes, originally allocated in `ffmain.cc`:

| Allocation | Bytes / objects |
| --- | ---: |
| FFSugi, FFSpring, FFKaese, FFRandom, FFShow, FFOrtho | 1,712 / 6 |
| Their type strings | 366 / 6 |
| FFKaese's container bucket arrays | 160 / 4 |
| Total | **2,238 / 16** |

`addDefaultGraphLabel` stores borrowed prototype pointers. Graph construction
calls `proto()` to create independently owned graph labels. The registry
releases its pointer containers at shutdown, but never deletes the prototypes.
The application had no owner for the six startup allocations. The libstdc++
string/container frames are children of those leaked objects, not separate
standard-library defects.

The actual GUI Quit path calls `exit(0)` from the Tcl command handler. Merely
using automatic local owners in `main` would not fix that path: `std::exit`
does not unwind automatic local objects.

## Changes and scope

The fix was tested during this session and subsequently committed by the user
as `9b72553`; the agent did not commit or push.

- `misc/FFDefaultLayouts.cc/.h` holds the six registrations in an application
  helper, with one function-static `unique_ptr` owner per prototype. These
  owners are destroyed on both normal return and `std::exit`.
- `ffmain.cc` calls the helper. Enabled types and allocation/registration order
  remain the same. No SPQR layout was enabled.
- `FFGraphClass.h` documents the pre-existing borrowed-pointer contract.
  Ownership was not transferred to the registry; callers using their own
  static/stack prototypes remain valid.
- The graph instances still own their independent clones. No parser, layout
  algorithm, GUI command binding or shutdown flow was changed.
- CMake builds the same helper into the app and the new headless regression.

`test_application_default_layouts` checks all six registered types and their
independent clones, then returns normally. `test_application_default_layouts_exit`
uses the same binary and calls `std::exit(0)`. Both were run against the
original raw registrations before the ownership correction: both failed strict,
unsuppressed LeakSanitizer with the identical **2,238 bytes / 16 allocations**.
Both pass after the correction. Checks remain active in Release.

## Actual verification

Starting checkout was `6a3eced`, clean except the user's `asan.log` and
`lsan.supp`. Debug baseline **62/62** passed. Those user files were not edited.

| Check | Result |
| --- | --- |
| Actual pre-fix GUI Quit, Xvfb, user's existing dependency suppressions | Reproduced **2,238 / 16**, exit 1 |
| Pre-fix headless normal-return and exit regressions, no suppressions | Both failed with **2,238 / 16** |
| Fixed headless regressions, strict ASan/UBSan/LSan | **2/2 passed** |
| Final Debug CMake/build/CTest | **64/64 passed** |
| Final Release CMake/build/CTest | **66/66 passed**, including configured external Rome tests |
| Final strict headless ASan/UBSan/LSan, LSAN_OPTIONS cleared | **54/54 passed** |
| GUI sanitizer runtime with existing leak-disabled preset | **10/10 passed** |
| Fixed actual 2D GUI Quit, user's existing dependency suppressions | Exit **0**, no unsuppressed report |
| Fixed actual 3D GUI Quit, same suppressions | Exit **0**, no unsuppressed report |
| Fixed actual 2D GUI Quit, no suppressions | Exit **1**, fontconfig-only report described below |
| Git whitespace checks | Passed |

The unsuppressed fixed-GUI control still reported **53,207 bytes / 1,970
allocations**. All 652 allocation-stack groups have their first allocation
caller in libfontconfig. The same control with the user's file suppressed
exactly 53,207 bytes / 1,970 fontconfig allocations. The 3D run suppressed
59,477 bytes / 2,208 fontconfig allocations. These dependency totals depend on
which windows/fonts were used and differ from the user's longer manual session.
No fontconfig finalizer or suppression was added to production, CI or tests.

## Reproduction

```sh
cmake --preset asan
cmake --build --preset asan --parallel 2
LSAN_OPTIONS= ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
ctest --test-dir build/asan -R '^test_application_default_layouts' --output-on-failure
LSAN_OPTIONS= ctest --preset asan-headless
LSAN_OPTIONS= ctest --preset asan-gui

cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release
```

Actual-app automation used a copy of `lib/ffgraph` under
`/tmp/ff-default-layout-repro`. A timer appended only to the copied
`tcl/stdGraph.tcl` waited for `recall` to exist, printed a marker and invoked
`recall modPress q`. The repository's scripts were not changed. Xvfb launches
set FF_LIBDIR to that copy and FF_ICONDIR to the repository icons. The 3D test
used an ignored temporary `ffgraph3D` symlink to the same ASan executable.

Before/after GUI logs, strict regression logs and CTest output are retained as
`/tmp/ff-default-layout-*`. The user's root-level logs/suppression file remain
their own local artifacts.

## Limits

This fixes the specific startup ownership leaks in the supplied log. The
existing immediate Quit semantics, generic Tcl/global teardown, dependency
shutdown caches, allocation-failure paths and every possible interactive
editing sequence are not established leak-free. The strict GUI leak suite was
not rerun; its existing toolkit limitation remains. The parked WSLg inspector
issue and SPQR remain out of scope.
