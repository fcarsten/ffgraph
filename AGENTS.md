# ffgraph — Agent instructions

## Project

Legacy C++ graph processing and visualization application,
originally developed in the 1990s and modernized for Ubuntu 24.04.

C++17, CMake, Tcl/Tk and X11.

## Development principles

- Preserve existing program behavior unless fixing a demonstrated bug.
- Prefer targeted changes over broad architectural rewrites.
- Keep the existing Tcl/Tk GUI working.
- Treat graph and container ownership changes as high risk.
- Do not enable the experimental SPQR implementation.
- Do not revisit the parked WSLg inspector issue without a request.

## Workflow

- Read relevant documentation in docs/ before starting.
- Reproduce bugs before fixing them whenever practical.
- Add regression tests for correctness changes.
- Run CMake and CTest after modifications.
- Use ASan/UBSan for memory ownership changes.
- Do not silently disable failing tests.
- Never commit or push unless explicitly requested.

## Documentation

For every substantive patch, create a corresponding
docs/FFGRAPH-*.md report covering:

- Problem and root cause
- Files and implementation changes
- Tests actually executed and results
- Unverified assumptions and remaining risks
- Build and verification commands

Distinguish successful tests from tests that could not run.