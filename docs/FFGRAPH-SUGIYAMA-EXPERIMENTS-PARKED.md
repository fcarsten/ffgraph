# Sugiyama drawing-quality experiments undone and parked

**Historical rollback checkpoint:** subsequent authorized correctness,
behavior-preserving performance and runtime-resource fixes are recorded in
`FFGRAPH-LAYOUT-CONTROLS-CHECKPOINT.md`,
`FFGRAPH-EXACT-LAYOUT-OPTIMIZATIONS.md` and
`FFGRAPH-RUNTIME-PORTABILITY-CHECKPOINT.md`. The experimental aesthetic
algorithms remain parked. The byte-for-byte restoration below describes the
rollback point, before those later changes.

Date: 3 November 2026. Branch: `import_and_layout_profiling`, HEAD `c1fc4bc`.
The user requested undoing the experiments after manual testing found that
improvements in some aspects made other aspects worse. Scalar edge-length,
alignment and crossing metrics did not establish acceptable overall drawings.
Further aesthetic refinement is parked until explicitly requested again.

## Rollback scope

All production source, CMake, inspector resource, default registration, profiler
and existing test changes from the uncommitted experiments were restored byte
for byte from HEAD. Removed the experimental subclass, inspector, geometry helper,
associated new tests and diagnostic script from the active source tree. This
restores the six original layout prototypes and original profiling options.

The compact/BFS, whole-layer shear and horizontal-postpass checkpoint reports
were archived with their implementations. Their claims no longer describe the
current checkout. `docs/FFGRAPH-NEXT-SESSION-PLAN.md` now records the parked scope
and the outstanding tasks, rather than recommending further aesthetic work.

Earlier **committed** corrections remain: Spring numerical stability and
zero-force handling, the missing-graphics hang fix, crossing-count optimization,+barycenter ordering/stopping fixes, default-layout ownership, and modern parsers.
No existing committed tests were disabled. Tests specific to the withdrawn
experimental algorithms were withdrawn with those algorithms.

Related experimental scaffolding was also restored: graph-label subtype lookup,
exact menu selection/exception handling, the protected settings accessor, and
the Tcl resource-path fix. These were part of the uncommitted diff, not earlier
production commits. The independent resource-path defect remains a documented
follow-up: `sugi.tcl` overwrites configured FF_LIBDIR with a relative path.
It should be fixed and tested separately from layout aesthetics.

## Archive and recovery information

Before restoring anything, saved all **11 modified tracked files and 15
untracked experimental files**, with their complete contents, hashes, baseline
commit and tracked-file patch, under:

```
build/parked-sugiyama-experiments-2026-11-03/
  manifest.json
  tracked-changes.patch
  files/...
```

This archive is local, ignored by Git, and will not survive deleting the build
directory. Earlier diagnostic binaries, measurements and source variants remain
in the existing `build/sugiyama-*` and `build/sugi-*` artifact directories.
The repository's persistent parking record is this report and the roadmap.
No commit, push, tag, stash, index reset or history rewrite was performed.

## Verification

The source rollback was checked against HEAD before making the documentation-only
updates. Only this report and the roadmap differ from HEAD after verification.

| Check | Result |
| --- | --- |
| Debug CMake configure/build and full CTest | **67/67 passed** |
| Release CMake configure/build and full CTest | **69/69 passed**, including configured external Rome tests |
| ASan/UBSan build and strict headless/leak tests | **57/57 passed**, LSAN_OPTIONS cleared |
| GUI ASan/UBSan runtime tests, leak detection OFF | **10/10 passed** |
| Source restoration and `git diff --check` | Passed |

The lower test counts reflect removal of three experimental headless tests and
one experimental GUI test, together with their withdrawn implementation. All
committed baseline tests remain registered and passed. Logs are retained under
the archive's `rollback-verification/` directory; initial command logs are
`/tmp/ff-park-*`.

Commands:

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release
cmake --preset asan
cmake --build --preset asan --parallel 2
LSAN_OPTIONS= ctest --preset asan-headless
LSAN_OPTIONS= ctest --preset asan-gui
git diff --check
```

Strict GUI leak diagnostics, interactive visual assessment, installed runtime
and remote CI are separate checks and are not claimed by local CTest success.
The known toolkit leak policy, SPQR exclusion, parked WSLg issue and low priority
of optimizing Rome's deprecated format remain unchanged.
