# ffgraph Pass 10 — file-format and Tcl/Tk lifetime regression checkpoint

**Date:** 1 November 2026
**Patch baseline:** latest user archive `ffgraph-current(1).zip`, then Pass 8, Pass 8 follow-up, and Pass 9 patches in that order.
**Scope:** A focused, independently applicable Pass 10 checkpoint, **not** an exhaustive application memory-safety certification.

## Changes

1. **Fix real GML scanner leaks (`fgraph/GTScanner.l`, `GTScanner.cc`, `GTScanner.h`).** The legacy GML scanner allocates `KEY` and `STRING` semantic values using `strdup`, but the legacy parser does not release either type. The `GTControl` wrapper now retains pointers to all returned key/string tokens and frees them on destruction. The checked-in legacy flex++ scanner additionally allocated a buffer that had no generated destructor; `GTControl` now explicitly disposes of its final scanner buffer. Both source grammar/scanner (`.l`) and checked-in build outputs (`.cc`/`.h`) were updated consistently. **No parser grammar, token interpretations, public graph API or file formats were changed.** Future parser regeneration (Pass 11) should preserve this token ownership rule.
2. **Add repeatable end-to-end file-format test (`tests/test_graph_io_pass10.cpp`).** Exercises repeated sgraph string and file serialization/reimport, with 3 nodes, 3 edges including a parallel pair and a self-loop, start/end designations and geometry labels; graph mutation and cleanup after import; repeated GML file roundtrips; and load/save/reload of the repository's real `contrib/out.g` sample (499 nodes, 499 edges on the tested baseline). It verifies edge endpoint membership, counts and named topology. Temporary files use `mkstemp` and are removed by scoped cleanup.
3. **Add Tk/Xvfb lifecycle regression (`tests/test_tk_lifecycle_pass10.cpp`).** Exercises 12 iterations of paired real Tk roots, subwindow borrowing and command deregistration, 8 callbacks that destroy their owning C++ window during nested Tcl evaluation, and continued operation of another root while deferred interpreters are collected. Uses public Tcl/Tk calls and does not depend on the parked WSLg inspector issue.
4. **CTest and CI wiring.** The file-format test is registered whenever `BUILD_TESTING` is enabled; the Tk lifecycle test is only built and registered if `xvfb-run` is available, following existing project conventions. GitHub Actions has a separate Ubuntu 24.04 sanitizer job with ASan/UBSan and leak detection enabled; the existing Debug job is retained. The new sanitizer job has been authored but **has not been executed on GitHub Actions** from this environment.

## Failure reproduced before the fix

The new `test_graph_io_pass10` was compiled with the original Pass 9 GML scanner and run with `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1`. It failed with:

- 73 bytes in 13 allocations from `GTScanner.l:133` (`KEY`/`strdup`);
- 45 bytes in 5 allocations from `GTScanner.l:120` (`STRING`/`strdup`);
- 40 bytes in 1 allocation from legacy flex++ `yy_create_buffer`;
- **158 bytes in 19 allocations in total**, after a *single* GML import.

After recompiling **both** `GTScanner.cc` and its consumer `FFGraphClass.cc` against the updated `GTScanner.h`, the same test passed with leak detection enabled. It then passed eight repeated GML imports and the bundled large sgraph roundtrip under ASan/UBSan. A mixed-header trial (new scanner with old `FFGraphClass.o`) produced an expected C++ object-layout incompatibility; it is **not** a defect in a clean rebuild. Do not mix objects compiled against old/new generated scanner headers.

## Actual verification

Host: Debian 13, GCC 14.2, C++17, CMake 3.31, Ninja, supplied Tcl/Tk 8.6 headers, installed Tcl/Tk 8.6 runtime and Xvfb. Tk and Xpm **development** dependencies were not installed.

| Build | New file roundtrip | New Tk/Xvfb lifecycle | Previous Pass 9 headless suite |
|---|---|---|---|
| Debug (`-O0 -g`) | PASS | PASS | 7/7 PASS |
| Optimised (`-O1 -DNDEBUG`) | PASS | PASS | 7/7 PASS |
| ASan/UBSan (`-O0 -g1`, leak detection on) | PASS; no sanitizer findings | PASS; no sanitizer findings | 7/7 PASS |

**Verification boundary:** New tests linked **actual ffgraph C++ graph/GUI code** and system Tcl/Tk libraries. The temporary test driver used a previously built, source-identical Pass 9 core archive **plus recompiled updated `GTScanner.cc` and its header consumer `FFGraphClass.cc`**. The earlier seven standalone CTest executables were rerun unchanged and thus independently verified only the unchanged Pass 9 implementation, not the new GML fix. The new test executables were run directly, not through the full project's CTest, because this environment lacks `tk.pc` (and the Xpm dev headers/unversioned library link file). The `cmake --preset debug -DBUILD_TESTING=ON` command fails at `pkg_check_modules(TK REQUIRED IMPORTED_TARGET tk)`; no full-application executable, install check or full-project CTest execution is claimed. Valgrind was not available.

## Ubuntu 24.04 verification before merging

```bash
# From a checkout containing the prior Pass 8 / Pass 9 patches:
git apply --check pass10-memory-regression.patch
git apply pass10-memory-regression.patch
sudo apt-get update
sudo apt-get install --yes build-essential cmake ninja-build pkg-config \
    tcl-dev tk-dev libx11-dev libxpm-dev xvfb xauth

cmake --preset debug -DBUILD_TESTING=ON
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure

cmake --preset release -DBUILD_TESTING=ON
cmake --build --preset release --parallel 2
ctest --test-dir build/release --output-on-failure

cmake --preset asan -DBUILD_TESTING=ON
cmake --build --preset asan --parallel 2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
    ctest --preset asan --output-on-failure --parallel 1
```

The new `test_tk_lifecycle_pass10` uses `xvfb-run` if present; CMake omits it if Xvfb is not installed. Inspect leak stacks if other Tcl/Tk-specific tests fail under leak detection rather than disabling leak checking globally.

## Remaining risks and proposed continuation

- Full Ubuntu CMake/CTest/ASan and GitHub Actions results still need confirmation, including all existing GUI tests under Xvfb.
- The old Valgrind `MFPlex<FFPoint>::length()` scenario still lacks its original user graph; the existing spring, geometry and large sample regressions do not prove every route or memory path safe.
- Some legacy parser paths and error-recovery code have not been fuzzed. An actual malformed/truncated graph corpus and original triggering input would be useful for Pass 11.
- Tcl/Tk global caches can complicate leak attribution; none were suppressed in these Pass 10 tests. Other full application ownership paths, especially interactive graph windows, remain unverified here.
- The separately parked inspector/WSLg issue and excluded SPQR algorithms were not changed.

**Next:** Confirm complete project CI on Ubuntu; then Pass 11 parser/scanner regeneration and malformed-file corpus coverage, with file-format compatibility gates.
