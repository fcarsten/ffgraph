# ffgraph — current modernisation plan

Updated: 1 November 2026. The current Git checkout and its diffs are authoritative.
This replaces the earlier roadmap that still recommended beginning Pass 8.
Historical checkpoint reports retain their original test counts and scope.

## Current position

FF/sgraph and GML use modern GNU Bison/Flex parsers by default in every CMake
build, including BUILD_TESTING=OFF. Both legacy implementations have been
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

## Rome pilot completed; next checkpoint: opt-in integration

The isolated pilot is now implemented; see
`FFGRAPH-PASS12-ROME-PILOT-CHECKPOINT.md`. The opt-in
`FFGRAPH_ENABLE_MODERN_ROME_PILOT` option adds generated parser/adapter libraries
and four headless tests. Production still uses checked-in Bison++/flex++ Rome
output even when the pilot is ON. The parser also builds standalone without Tk.
No production backend selector or GUI modern Rome loading path exists yet.

The next checkpoint is **opt-in production integration**, following review of
the pilot's differences: failure atomicity, checked integer range, actual error
locations and long-line handling. Start with the pilot report, baseline report,
the actual grammar/scanner sources and `FFGraphClass::load()`. Keep the legacy
backend available; preserve the tested scanner cleanup and FF/GML behavior.

1. Extend the synthetic corpus with representative real Rome graphs when
   available. Freeze independent graph expectations before replacing any code.
   Cover duplicate IDs, ignored metadata, absent endpoints, forward references,
   loops, parallel edges, comments, CRLF, EOF, numeric limits and malformed input.
2. The pilot already uses owned typed records, checked integers and per-call
   reentrant scanner state. It passes generated-parser tests, live comparisons
   with the real legacy implementation and independent graph oracles. Retain
   these checks and extend them when integration reveals additional requirements.
3. Decide whether to accept unchanged-on-rejection imports for modern Rome.
   Legacy Rome changes IDs/directedness and keeps parsed prefixes on failure.
   The pilot deliberately does not replay failed input. Keep the difference
   explicit in integration tests and documentation; do not preserve undefined
   numeric overflow or silently truncate long input lines.
4. Integrate via an explicit backend choice, then verify Debug, meaningful
   Release, strict headless ASan/UBSan/leaks, GUI runtime and GUI leak diagnostics.
   Retain FF/GML regression and GUI workflows throughout.
5. Make a separate default/retirement checkpoint only after generated-parser
   compatibility and real application tests pass. Preserve a pre-removal Git
   reference when explicitly requested; do not commit or tag without permission.

The present baseline deliberately records partial graph mutation on Rome syntax
errors and silently skipped missing endpoints. Atomic failure would be a useful
improvement but needs an explicit compatibility decision, not an accidental
side effect of using a new parser.

## Dependency, install and portability checkpoint

After Rome integration, verify installed runtime behavior from outside the
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

Read `BUILD-CMAKE.md`, `FFGRAPH-PASS12-FF-LEGACY-RETIREMENT.md`, the verification
and Rome baseline reports before editing. Record uncommitted changes, actual
test counts/results and dependency versions. No commits or pushes without an
explicit user request; never silently disable a failing test.
