# Windows installed-runtime test environment — 8 November 2026

## Problem and root cause

At commit `b903bd84d0edb63c1fc738dc59fca5276adeaef0`, both native Windows
Debug and Release GitHub jobs compiled successfully and passed 26 of 27 tests.
`test_installed_runtime` failed before launching the staged application with
`KeyError: 'SystemRoot'`. This also prevented subsequent artifact upload steps.

Python normalizes `os.environ` keys to uppercase on Windows. Its `copy()` returns
a regular dictionary, so the test's mixed-case `SystemRoot` lookup failed even
though the Windows system directory was available as `SYSTEMROOT`.

Evidence: [Windows workflow run](https://github.com/fcarsten/ffgraph/actions/runs/42320773988),
[Debug job](https://github.com/fcarsten/ffgraph/actions/runs/42320773988/job/128244517096),
[Release job](https://github.com/fcarsten/ffgraph/actions/runs/42320773988/job/128244516858).

## Files and implementation changes

- `tests/test_installed_runtime.py`: extract Windows child-environment isolation
  into a small helper and use the uppercase `SYSTEMROOT` key. Keep the staged
  `bin` and Windows `System32` directories on PATH and remove development
  Tcl/Tk overrides, preserving the dependency isolation check.
- `tests/test_installed_runtime_env.py`: exercise that helper with a plain
  dictionary containing uppercase Windows keys, a staging path with spaces,
  development library overrides and an unrelated variable. Check the complete
  resulting environment. This regression also runs on Linux, where the Wine
  integration test uses host Python and skips the native Windows branch.
- `CMakeLists.txt`: register the regression in both full and initial test
  suites as a headless Python test.

No application behavior, dependency audits, or existing tests are disabled.

## Tests actually executed and results

- Before changing the key, the new regression reproduced `KeyError: 'SystemRoot'`.
- After changing the key, the standalone regression passed.
- Debug and Windows cross-build CMake configuration and builds succeeded.
- Full Linux Debug CTest suite passed **96/96**, including the new regression
  and the installed 2D/3D application launch test.
- Windows cross-build focused CTest checks passed **2/2** under Wine/Xvfb:
  installed 2D/3D runtime launches and the new environment regression.
- `git diff --check` passed.

## Build and verification commands

```sh
python3 -B tests/test_installed_runtime_env.py
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure --parallel 2
cmake -S . -B build/windows-cross
cmake --build build/windows-cross --parallel 2
xvfb-run -a ctest --test-dir build/windows-cross \
  -R '^test_installed_runtime(_env)?$' --output-on-failure
git diff --check
```

The local cross-build cache and Wine wrapper are described in
[the native Windows port report](FFGRAPH-WINDOWS-NATIVE-PORT.md).

## Unverified assumptions and remaining risks

No native Windows host is available for local verification. The regression
directly tests the failing dictionary lookup, while Wine integration checks
the staged Windows application using host Python. Native MSYS2/UCRT64 Debug
and Release GitHub jobs must run again after these changes are committed and
pushed by the user. No commit, push, or remote workflow rerun was performed.
ASan/UBSan were not run; this patch changes only Python test setup and CTest
registration, with no C++ or memory ownership changes.
