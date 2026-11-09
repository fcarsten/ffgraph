# Public rc1 release notes — 9 November 2026

## Problem and scope

`RELEASE-NOTES.md` lacked the recent desktop fixes and Info/WSL features, and
its artifact section described maintainer publication work rather than helping
public testers choose a download, start the application and report problems.

## Files and changes

- `RELEASE-NOTES.md`: add the intended Windows, Ubuntu and source asset names,
  extraction/launch instructions, bundled-runtime versus system dependencies,
  unsigned Windows executable status, checksums and manifest explanation.
- Summarize the selector/owner-close, New Graph, last-window shutdown, Num Lock,
  formatted Info and WSL browser changes using their existing implementation
  reports. Retain the historical algorithms/compatibility limitations.
- Add concrete testing suggestions, a GitHub Issues link, useful reproduction
  details and the optional selector diagnostic-log setting. Link build and
  packaging instructions to the planned `v1.0.0-rc1` tag so they resolve from
  a GitHub release description.

This is a documentation-only change. `VERSION`, application behavior, release
tags and published assets are unchanged.

## Verification actually performed

- Compared the descriptions with `RELEASING.md`, `VERSION`, package naming and
  the recent per-change reports under `docs/`.
- CMake Release configure and build: passed.
- Existing release-version and packaging CTest checks: **2/2 passed**. Log:
  `build/release/release-notes-checks.log`.
- `git diff --check`: passed.

No new tests were added for this text change. Full application, GUI, Windows
and sanitizer suites were not rerun; the notes do not assert fresh execution
of those suites or approval of final release binaries.

## Commands and remaining limits

```sh
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release -R '^(test_release_version|test_release_packaging)$' --output-on-failure
git diff --check
```

The tag-specific documentation links and named downloads describe the intended
public release. They become available when the verified tag and matching assets
are published; live availability was not verified or claimed here. Final CI,
desktop verification and clean-commit packaging remain part of `RELEASING.md`.
No commit, push, tag, package refresh or publication was performed.
