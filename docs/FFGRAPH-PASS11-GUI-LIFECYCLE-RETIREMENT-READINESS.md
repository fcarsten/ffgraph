# Pass 11: GUI lifecycle regression suite and legacy GML retirement readiness

## Goal and baseline

Incremental follow-up to `FFGRAPH-PASS11-LOAD-WINDOW-LIFETIME.md`. This pass
**does not remove** the legacy `GTParser`/`GTScanner`, change GML syntax,
modify the production importer, or alter the Tcl/Tk GUI. It supplies an
end-to-end regression for the exact Tcl menu callbacks that previously caused
loaded windows to disappear. The modern parser remains the CMake default.

## What the new GUI test exercises

`tests/test_gui_file_workflow_pass11.cpp` creates real `FFTclWindow`-derived
windows under Xvfb. Its test subclass overrides only `proto()` to track the
additional windows created by **File > Load**. The test replaces the Tcl
`FileSelectorBox` procedure with a predetermined file path and invokes the
application's real `recall modPress l/m/w` commands. No C++ test helper invokes
`FFGraphClass::load()` in place of these menu callbacks.

* File-picker cancellation creates no graph window.
* Repeated File > Load creates a distinct graph/Tk root that survives return
  from the Tcl callback and deferred-interpreter cleanup.
* File > Merge adds nodes/edges to that graph without replacing or closing it.
* Repeated semantically invalid GML merges preserve node count, edge count,
  original node addresses, IDs, labels, and graph identity; both windows stay
  operational. This semantic-atomicity expectation applies to the modern
  backend only.
* File > Close deletes the child window without closing its parent, followed
  by closing the parent and checking that both C++ labels and Tk roots are
  reclaimed. Four load/merge/close/reopen cycles are performed.
* An idle callback executes File > Close inside nested Tcl evaluation, followed
  by safe deferred cleanup. This detects premature interpreter deletion.

The default test uses historical `nase.gml` and `nase3.gml` samples plus a
syntax-valid GML fixture with an unknown edge endpoint. A separate, opt-in
large-graph test loads historical `nase2.gml` (more than 1,000 node entries),
with a longer timeout. This keeps ordinary CI runs bounded.

### CMake/CTest

The normal Xvfb test is registered as `test_gui_file_workflow_pass11` when
`BUILD_TESTING=ON`, Xvfb is installed and `FFGRAPH_GML_BACKEND=modern`.
It is automatically included in the project's normal Debug/Release/ASan CTest
runs. It is also included in the manually triggered GML parity workflow in
both Debug and ASan/UBSan modes.

Large GUI graph testing is deliberately opt-in:

```sh
cmake -S . -B build/gml-gui-stress -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  -DFFGRAPH_GML_BACKEND=modern \
  -DFFGRAPH_GUI_STRESS_TESTS=ON
cmake --build build/gml-gui-stress --target test_gui_file_workflow_pass11 --parallel 2
ctest --test-dir build/gml-gui-stress \
  -R '^test_gui_(file|large_file)_workflow_pass11$' --output-on-failure
```

## Verification performed here

* Strict GCC C++17 syntax compilation of the new GUI driver, using the
  supplied Tcl/Tk 8.6 headers: **passed**.
* Python corpus regression suite: **9/9 tests passed**.
* Git patch whitespace and application checks: **passed**.
* GitHub Actions YAML parsed successfully.
* Confirmed the test's Tcl `recall modPress` command paths and original
  `FileSelectorBox` name against current sources.

**Not verified on this host:** execution of the new Xvfb/Tk test, compilation
or linking of the full modern parser, full CMake/CTest and full ASan/UBSan.
This environment lacks Bison, Flex, Tk development metadata and Xpm development
headers. Do not consider retirement approved until the Ubuntu runs pass.

## Ubuntu verification

The commands below assume the downloaded patch is saved in Windows Downloads
and the project is in WSL, as requested.

```sh
git apply --check /mnt/c/Users/User/Downloads/ffgraph-pass11-gui-lifecycle-readiness.patch
git apply /mnt/c/Users/User/Downloads/ffgraph-pass11-gui-lifecycle-readiness.patch

cmake -S . -B build/gml-modern-default -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  -DFFGRAPH_GML_BACKEND=modern
cmake --build build/gml-modern-default --target \
  ffgraph test_gui_file_workflow_pass11 --parallel 2
ctest --test-dir build/gml-modern-default \
  -R '^test_(load_window_lifetime|gui_file_workflow)_pass11$' \
  --output-on-failure -V
```

For a fresh sanitizer build, use the project's **actual** CMake option
`FFGRAPH_SANITIZERS=ON` (not `FFGRAPH_ENABLE_SANITIZERS`):

```sh
cmake -S . -B build/gml-gui-asan -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  -DFFGRAPH_GML_BACKEND=modern -DFFGRAPH_SANITIZERS=ON
cmake --build build/gml-gui-asan --target test_gui_file_workflow_pass11 --parallel 2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
ctest --test-dir build/gml-gui-asan \
  -R '^test_gui_file_workflow_pass11$' --output-on-failure -V
```

Then run `ctest --test-dir build/gml-modern-default --output-on-failure`
and test GUI **File > Load**, **Merge**, **Close**, and **Reload** manually with
`nase.gml`, `fuenfer.gml` and `nase2.gml`. `options.gml` is an AGD options
configuration and is **not** a valid graph input; it must not be used as a
successful GML graph fixture.

## Legacy GML retirement gate (next patch, not this patch)

The legacy `GTParser`/`GTScanner` source files, `GTControl`, the
`FFGRAPH_GML_BACKEND=legacy` selector and the legacy-versus-modern differential
test path still exist. Removing them must be a *separate* reviewed patch with
an explicit pre-removal Git tag and a committed golden corpus/oracles.

Required evidence before the removal:

1. Full Debug and Release application builds and CTest pass on Ubuntu 24.04.
2. Smoke, standard and stress GML differential suites pass against the actual
   generated Bison/Flex implementation (before the legacy test path is deleted).
3. New Xvfb GUI menu workflow and existing Tk lifecycle regressions pass.
4. ASan/UBSan, with leak detection, passes on both the parser and GUI paths.
5. Manual GUI opening/merging of historical files succeeds, including large
   inputs, and no unexpected window closures recur.
6. Git tag for the last checked-in legacy implementation is pushed and the
   golden fixtures are independent of the deleted legacy code.

Known remaining boundary: the semantic preflight guarantees unchanged graph
state for reported syntax and semantic failures, but allocation exceptions
or callback exceptions during the commit phase are **not** fully transactional.
Also, **File > Load** creates a new window before parsing; a rejected file may
leave a blank new window. The added test only asserts failure atomicity for
**File > Merge**, so it does not claim the entire GUI load operation is atomic.
Those should be considered separately from the parser-retirement decision.

The inspector/WSLg issue remains parked and SPQR remains excluded.
