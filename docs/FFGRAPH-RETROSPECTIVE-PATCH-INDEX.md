# ffgraph — retrospective patch documentation index (Priority 1 to Pass 7)

**Created:** 1 November 2026
**Scope:** Retrospective descriptions of all distinct corrective/build/documentation/CI artifacts supplied **in this conversation** before the later Pass 8 and Pass 9 work. No application source, tests or build logic are changed by this documentation package.

## How to use these notes

This index accompanies existing newer `FFGRAPH-PASS8-*` and `FFGRAPH-PASS9-*` notes. The notes document historical intent, exact source/build scope where available, reported verification, known limitations and subsequent corrections. They are **not** an instruction to replay every historical patch on a current checkout.

The earlier Priority 1 release was distributed as a full source ZIP, not a proper Git patch. The initial SPQR patch was malformed and was superseded. The handover patch was reissued after a download problem. CMake troubleshooting included interactive CMake edits which did not all have separate downloadable patch files; those are recorded under the CMake migration note.

## Chronological patch/release inventory

| Sequence | Source artifact | Retrospective document | Notes |
|---|---|---|---|
| 1 | `ffgraph-priority1-fixed.zip` | [FFGRAPH-PRIORITY1-CORRECTNESS.md](FFGRAPH-PRIORITY1-CORRECTNESS.md) | Complete source archive, not a minimal Git patch |
| 2 | `ffgraph-cmake-migration.patch` | [FFGRAPH-CMAKE-MIGRATION-RETROSPECTIVE.md](FFGRAPH-CMAKE-MIGRATION-RETROSPECTIVE.md) | Initial CMake implementation; later interactive corrections |
| 3 | `ffgraph-tcl-test.patch` | [FFGRAPH-TCL-COMPATIBILITY-TEST.md](FFGRAPH-TCL-COMPATIBILITY-TEST.md) | Real-Tcl test replaces obsolete mock |
| 4 | `ffgraph-spqr-exclusion-corrected.patch` | [FFGRAPH-SPQR-BUILD-EXCLUSION.md](FFGRAPH-SPQR-BUILD-EXCLUSION.md) | Corrected patch; first SPQR patch corrupt |
| 5 | `ffgraph-priority2-pass1.patch` | [FFGRAPH-PRIORITY2-PASS1-PORTABILITY.md](FFGRAPH-PRIORITY2-PASS1-PORTABILITY.md) | Includes a later required correction |
| 6 | `ffgraph-priority2-pass1-fix.patch` | [FFGRAPH-PRIORITY2-PASS1-CORRECTION.md](FFGRAPH-PRIORITY2-PASS1-CORRECTION.md) | Compiler hotfix |
| 7 | `ffgraph-priority2-pass2-numerical.patch` | [FFGRAPH-PRIORITY2-PASS2-NUMERICAL.md](FFGRAPH-PRIORITY2-PASS2-NUMERICAL.md) | Numerical guards, SIGFPE |
| 8 | `ffgraph-priority2-pass3-ownership.patch` | [FFGRAPH-PRIORITY2-PASS3-OWNERSHIP.md](FFGRAPH-PRIORITY2-PASS3-OWNERSHIP.md) | Reference-count self assignment and leaks |
| 9 | `ffgraph-priority2-pass4-tcl-label-ownership.patch` | [FFGRAPH-PRIORITY2-PASS4-CALLBACK-LABELS.md](FFGRAPH-PRIORITY2-PASS4-CALLBACK-LABELS.md) | Tcl callback and label ownership |
| 10 | `ffgraph-priority2-pass5-graph-reference-cycles.patch` | [FFGRAPH-PRIORITY2-PASS5-REFERENCE-CYCLES.md](FFGRAPH-PRIORITY2-PASS5-REFERENCE-CYCLES.md) | Graph reference-cycle fix |
| 11 | `ffgraph-priority2-pass6-label-and-tcl-lifecycle.patch` | [FFGRAPH-PRIORITY2-PASS6-LABEL-TCL.md](FFGRAPH-PRIORITY2-PASS6-LABEL-TCL.md) | Replacement graph labels; early Tcl cleanup |
| 12 | `ffgraph-inspector-visibility-fix.patch` | [FFGRAPH-INSPECTOR-VISIBILITY.md](FFGRAPH-INSPECTOR-VISIBILITY.md) | Inspector visibility; broader WSLg issue parked |
| 13 | `ffgraph-priority2-pass7-tk-interpreter-teardown.patch` | [FFGRAPH-PRIORITY2-PASS7-INTERPRETER-TEARDOWN.md](FFGRAPH-PRIORITY2-PASS7-INTERPRETER-TEARDOWN.md) | Deferred Tcl interpreter teardown |
| 14 | `ffgraph-handover-docs-reissued.patch` | [FFGRAPH-HANDOVER-DEPENDENCY-DOCS.md](FFGRAPH-HANDOVER-DEPENDENCY-DOCS.md) | New handover/dependency docs and capture script |
| 15 | `ffgraph-github-actions-ci.patch` | [FFGRAPH-GITHUB-ACTIONS-CI.md](FFGRAPH-GITHUB-ACTIONS-CI.md) | Ubuntu 24.04 CI workflow |

## Evidence standard

- **Patch/archive inspection:** this retrospective was grounded in the original patch files currently available and a comparison of the Priority 1 source ZIPs.
- **Assistant-side tests:** only reported when performed in the earlier conversation; they are not repeated as part of this docs-only change.
- **User-side confirmation:** distinguished from assistant-side testing; generally indicates successful Ubuntu build, CTest and short interactive checks, not exhaustive proof.
- **Later work:** Pass 8/9 notes supersede some compatibility/audit assessments in these historical records.

## Main accepted sequencing

Priority 1 ZIP → initial CMake migration and interactive CMake fixes → Tcl test correction → corrected SPQR exclusion → Priority 2 Pass 1 and hotfix → Passes 2–6 → inspector patch (visibility issue later parked) → Pass 7 → handover/dependency docs → GitHub Actions CI. Legacy Makefile removal was performed manually by the user and **was not delivered as a dedicated patch** in this chat.

## Suggested verification for a modern checkout

```sh
cmake --preset debug
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
cmake --preset release
cmake --build --preset release --parallel
ctest --test-dir build/release --output-on-failure
cmake --preset asan
cmake --build --preset asan --parallel
ctest --preset asan --output-on-failure
```

Check registered Xvfb tests, actual GUI open/close behaviour, representative graph files, static link dependencies, and the documentation in the newer Pass 8/9 notes. **SPQR stays excluded; inspector/WSLg stays parked.**
