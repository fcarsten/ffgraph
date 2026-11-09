# Windows CI Git availability and source archive newlines — 9 November 2026

## Failure and root cause

At commit `dd23f7fac92cbb31625d3708a48d7cbd73e820e0`, both native Windows
Debug and Release jobs built successfully and passed 33 of 34 tests. Their
single failing test, `test_release_packaging`, stopped at its first
`subprocess.check_output(["git", ...])` with `FileNotFoundError: [WinError 2]`.
The subsequent staging/package upload steps were skipped. All four Linux jobs
on the same commit succeeded.

Evidence: [Windows run](https://github.com/fcarsten/ffgraph/actions/runs/42553962074),
[Debug job](https://github.com/fcarsten/ffgraph/actions/runs/42553962074/job/128955444208),
[Release job](https://github.com/fcarsten/ffgraph/actions/runs/42553962074/job/128955444548),
and [Linux run](https://github.com/fcarsten/ffgraph/actions/runs/42553962038).

CMake found Git at `C:/Program Files/Git/cmd/git.exe` and native Python 3.15.2
through its Windows discovery mechanisms. This did not make the bare `git`
command visible to Python children: MSYS2 setup defaults to a minimal Windows
PATH. The existing runner Git installation was excluded from that environment.
See the action's [PATH options](https://github.com/msys2/setup-msys2#path-type).

Native Windows reproduction also exposed newline assumptions hidden by the
missing-command failure. Python text writes can produce CRLF where the test
expects LF. In addition, `git archive` can apply `core.autocrlf` conversion,
changing exported source text even when the Git object contains LF. The latter
also reproduced on Linux by enabling Windows-style Git settings in the fixture.

## Files and implementation

- `.github/workflows/windows.yml`: preserve the runner PATH with
  `path-type: inherit` and run a native Python-to-Git subprocess check before
  configuration. The existing CTest and package-generation steps remain enabled.
- `BUILD-WINDOWS.md`: include Git among development prerequisites and explain
  that Python children need it on PATH. Installed applications do not need Git.
- `tests/test_release_packaging.py`: write the byte-asserted VERSION fixtures
  explicitly with LF, enable `core.autocrlf=true` in the temporary repository,
  and check that binary data survives clean source export unchanged. Retain
  clean/dirty provenance, preview-content and ignored-file checks.
- `scripts/package-release.py`: run clean `git archive` with
  `core.autocrlf=false` and `core.eol=lf`, independent of host checkout settings.
  Explicit repository attributes still take precedence; preview snapshots
  continue to preserve their working-tree bytes.

No application, C++ ownership, GUI, parser, layout, SPQR or parked WSLg inspector
behavior changed. No failing test was removed, skipped or relaxed.

## Verification actually performed

- Native Windows Python 3.12, with Git removed from PATH: reproduced the CI
  `WinError 2` at `git init`.
- Native Windows with Git available: reproduced the CRLF/LF fixture failure.
- Extended Linux regression with `core.autocrlf=true`, before the archive fix:
  failed with `b'1.0.0-rc1\r\n' != b'1.0.0-rc1\n'`.
- After both newline fixes, the packaging regression passed with native Windows
  Python/Git under both process-local autocrlf settings (false/true).
- Linux standalone packaging regression: **1/1 passed**.
- CMake Debug/Release configure and builds: passed. Focused CTest packaging and
  release-version checks: **2/2 passed in each build**.
- Windows workflow parsed successfully as YAML; verified the new preflight is
  after MSYS2 setup and before CMake configuration.
- `git diff --check`: passed.

Native Windows checks used WSL interoperability and temporary fixture files
under `/tmp`. Temporary process-local Git configuration allowed access to the
fixture's UNC path; no persistent user/project Git configuration was changed.
CTest logs are `build/{debug,release}/windows-ci-git-checks.log`.

## Build and verification commands

```sh
python3 -B tests/test_release_packaging.py
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug -R '^(test_release_packaging|test_release_version)$' --output-on-failure
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release -R '^(test_release_packaging|test_release_version)$' --output-on-failure
git diff --check
```

In the native Windows build environment, the workflow preflight is:

```sh
python -c "import subprocess, sys; print(sys.executable); subprocess.run(['git', '--version'], check=True)"
python -B tests/test_release_packaging.py
```

## Remaining verification limits

The revised MSYS2/UCRT64 workflow still needs a GitHub run on the new commit.
Local native Windows Python 3.12 is not the CI runner's Python 3.15.2, and the
PATH change itself cannot be fully validated outside the hosted MSYS2 action.
Full application/GUI/sanitizer suites were not rerun for this build-tool and
fixture change. No project commit, push, remote workflow rerun or publication
was performed.
