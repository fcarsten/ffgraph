# Info links through the WSL browser — 9 November 2026

## Problem and root cause

The Info license link produced:

```text
gio: https://www.gnu.org/licenses/gpl-3.0.html: Operation not supported
```

The working environment is Ubuntu 24.04 under WSL2. It provides `gio` but has
no default application for `x-scheme-handler/https` and no installed Linux
browser found by `sensible-browser`. The original Info launcher treated the
presence of `gio` as a usable Linux desktop browser setup. Because it starts
launchers asynchronously, a later `gio` failure could only appear on stderr.

WSL can instead hand the URL to the Windows default browser. This environment
has no `wslview`, but does provide Windows' `rundll32.exe` on its WSL PATH.
This is separate from the parked WSLg inspector issue.

## Files and implementation

- `lib/ffgraph/tcl/info.tcl`: factor Linux command selection into
  `linuxURLCommand`. If `WSL_INTEROP` or `WSL_DISTRO_NAME` is present, prefer
  `wslview`, then `rundll32.exe url.dll,FileProtocolHandler`. Otherwise/failing
  availability of those commands, keep the native Linux `xdg-open`/`gio open`
  selection. Preserve structured argument lists and asynchronous launch.
  The native Windows and macOS branches, formatted content and HTTP(S)-only
  validation are unchanged.
- `tests/test_ttk_widgets.cpp`: reproduce the exact tool availability that
  previously selected gio despite Windows interop. Check WSL's rundll32 route,
  preference for installed wslview, detection through the distro variable,
  native Linux's xdg-open preference and gio fallback, and missing-launcher
  errors. Save and restore WSL environment variables in `finally`; no browser
  is opened by the regression's command stubs.
- `BUILD-CMAKE.md`: document native Linux desktop handlers and the WSL browser
  route/interop PATH requirement.
- `CMakeLists.txt`: update the installed documentation entry from `README` to
  `README.md`, following the concurrent user rename. The stale reference broke
  installation checks and prevented refreshing preview packages.

The preceding uncommitted formatted-Info changes are preserved. No production
C++, graph ownership, layout, parser, SPQR or inspector code changed.

## Tests actually executed and results

- `gio mime x-scheme-handler/https` reported no default application; invoking
  `gio open` with the reported GNU URL reproduced `Operation not supported`
  and exit status 2.
- Before the fix, the extended Debug widget regression failed with
  `WSL sent the URL to gio instead of Windows`
  (`build/debug/info-wsl-before.log`). The focused check passed after the fix.
- Invoked the command selected by the production helper against that GNU URL
  through real WSL interop. It selected
  `/mnt/c/Windows/system32/rundll32.exe url.dll,FileProtocolHandler` and returned
  successfully (exit 0). The Windows browser UI is outside the captured Linux
  desktop; this confirms handler invocation rather than visual page rendering.
- Full Linux Debug/Release runs passed **100/102** and **102/104** initially;
  both failures in each were installation checks caused by the renamed README.
  After correcting the install entry, both failed checks passed **2/2** in
  each build. The browser/Info regression passed in both full runs.
- Native Windows Tk/Wine widget/Info regression: **1/1 passed**. Installed
  runtime recheck after correcting the README entry: **1/1 passed**.
- `git diff --check`: passed.

Final CTest logs are `build/{debug,release,windows-rc1}/info-wsl-final.log`.
The installation rechecks are in the corresponding
`info-wsl-install-recheck.log` files. No failing check was disabled.

## Build and verification commands

```sh
gio mime x-scheme-handler/https
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure --parallel 2
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release --output-on-failure --parallel 2
ctest --preset debug -R '^(test_installed_runtime|test_installed_library)$' --output-on-failure
ctest --preset release -R '^(test_installed_runtime|test_installed_library)$' --output-on-failure
cmake -S . -B build/windows-rc1
cmake --build build/windows-rc1 --target test_ttk_widgets --parallel 2
xvfb-run -a ctest --test-dir build/windows-rc1 -R '^test_ttk_widgets$' --output-on-failure
python3 scripts/package-release.py --build build/windows-rc1 --output build/rc1-artifacts/windows --preview
python3 scripts/package-release.py --build build/release --output build/rc1-artifacts/linux --preview
git diff --check
```

Build directories retain their existing toolchain configuration. Display-backed
tests and Windows interop need access outside the process sandbox here.

## Verification limits and remaining risks

The application cannot configure an absent/broken native Linux HTTP(S) handler.
Outside WSL, a desktop browser handler must still be installed/configured.
WSL routing needs one of the Windows interop launchers on PATH. Later failures
inside asynchronous launcher/browser processes are not monitored by this patch.
The native Windows regression uses Tk under Wine; real Windows page rendering
and physical link clicks still need desktop confirmation.

No sanitizer run was needed for this Tcl command-selection change. Restart the
application to load the updated helper (or update installed resources). No
commit, push, tag or publication was performed.
