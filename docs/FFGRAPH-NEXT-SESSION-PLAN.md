# ffgraph — current modernisation plan

Updated: 1 November 2026. The current Git checkout and its diffs are authoritative.
This replaces the earlier roadmap that still recommended beginning Pass 8.
Historical checkpoint reports retain their original test counts and scope.

## Current position

FF/sgraph, GML and Rome use modern GNU Bison/Flex parsers in every CMake
build, including BUILD_TESTING=OFF. All legacy implementations have been
retired after generated-parser, graph construction, failure and GUI testing.
There is no legacy FF/GML build selector or automatic fallback.

The Rome pilot began at committed baseline `11a9304` on
`pass12-modern-rome-parser`, with the verification checkpoint already committed.
The local annotated tag
`ffgraph-pre-legacy-ff-removal` points to
`65e5498c40116c0ec85bccb3bbab5d060f098f57`, preserving the old FF parser/scanner.
Do not move that tag to a newer commit. Nothing in this checkpoint was committed
or pushed by the agent.

Container, graph ownership and GUI lifecycle work from Passes 8–10 already has
regression coverage. Avoid restarting those reviews from this document's old
candidate lists. Reproduce any remaining suspected issue before changing code.
The parked WSLg inspector investigation stays parked; SPQR remains excluded.

## Verification checkpoint

See `FFGRAPH-PASS12-VERIFICATION-HARDENING.md` for actual measurements and commands.

- All test checks execute in Release; twenty older tests now use FF_CHECK.
  A regression verifies side effects and intentional failure under NDEBUG.
- Debug and Release CI exercise the complete suite, including Xvfb tests.
- Every CTest has exactly one label: headless or gui. Strict ASan/UBSan/leak
  checking is mandatory for headless tests. GUI runtime ASan/UBSan is mandatory.
  Strict GUI leak checks remain a separate visible diagnostic step with retained
  logs. The full `asan` preset remains strict and can still fail on toolkit leaks.
- System Ubuntu Tk 8.6.14/Xft/fontconfig shutdown leaks reproduce independently
  of ffgraph. Isolated Tk 8.6.18 substantially reduces them but leaves 320 bytes
  per affected process. No system package or application shutdown policy changed.
- The new Rome baseline exposed scanner ownership leaks. Targeted cleanup fixes
  buffer and rejected-token lifetime; the grammar/loading contract is unchanged.

Use the exact environment and reports to judge these results. Remote GitHub
Actions and interactive WSLg use are separate verification tasks.

## Rome completed: modern-only production

The user explicitly authorized skipping the opt-in integration checkpoint and
making modern Rome default while removing legacy. See
`FFGRAPH-PASS12-ROME-LEGACY-RETIREMENT.md`. All three graph importers now use
build-tree GNU Bison/Flex output; no pilot flag or runtime backend selection
is needed. The local annotated tag `ffgraph-pre-legacy-rome-removal` preserves
commit `5db5a38e9c5c4c5bb176ec08c445ee2ea9b7d248` with the complete old parser
and live differential tests. It was not pushed by the agent.

Permanent Rome tests combine frozen pre-removal snapshots, independent topology
and source-graph oracles, actual production loading, 2D/3D window state and real
Tcl File/Load/Merge/Close flows. The external collection remains outside Git;
FFGRAPH_ROME_CORPUS_DIR registers optional local verification.

Rome rejection now preserves destination IDs/directedness/content; complete
long lines, checked integers and actual locations replace old limitations.
Successful merge ID resets, warning/edge skipping and filename dispatch remain
unchanged. Native string imports remain FF. GUI File > Load still creates a
window before parsing. Allocation/callback exceptions during replay remain a
separate unverified transactional concern.

The next recommended checkpoint is installed runtime and portability, below.
Keep all graph/corpus/GUI tests active; do not reintroduce legacy generation or
restart the already completed parser migration. Earlier reports retain their
historical baseline counts and policies.

## Dependency, install and portability checkpoint

Verify installed runtime behavior from outside the
checkout, including the ffgraph3D symlink and FF_LIBDIR/FF_ICONDIR resource lookup.
A staged install passing does not prove that launch works away from the source
working directory. Address the stated CMake minimum versus LINK_GROUP support
and test a clean BUILD_TESTING=OFF build.

Test a supported newer Tcl/Tk build through the existing GUI suite before
proposing dependency changes. Tk 8.6.18 testing so far used system Tcl 8.6.14
and fontconfig/Xft, not a fully upgraded stack. The dependency-only explicit
FcFini experiment does not justify adding process-global finalizers to ffgraph.
Keep leak policy and CI limitations visible; do not add broad suppressions.

Profile representative graph imports/layouts before changing containers for
performance. Preserve API/format/GUI behavior, and make focused patches with
regression tests and corresponding docs/FFGRAPH-*.md reports.

The user chose profiling as the immediate next checkpoint. See
`FFGRAPH-IMPORT-LAYOUT-PROFILING.md` for measurements, the corrected GML sibling
stack growth, and the still-open Spring coincident-star numerical failure.
Investigate that failure before optimizing layouts; repeated FF/Rome ID scans
and Sugiyama crossing counts are the measured performance candidates. The
installation/portability work above remains a separate follow-up.

## Starting commands

```sh
git status --short --branch
git log -5 --oneline
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release
cmake --preset asan
cmake --build --preset asan --parallel 2
ctest --preset asan-headless
ctest --preset asan-gui
ctest --preset asan-gui-leaks  # Known system toolkit leak failures remain visible
```

Read `BUILD-CMAKE.md`, the FF/Rome legacy-retirement reports and the verification
report before editing. Record uncommitted changes, actual
test counts/results and dependency versions. No commits or pushes without an
explicit user request; never silently disable a failing test.
