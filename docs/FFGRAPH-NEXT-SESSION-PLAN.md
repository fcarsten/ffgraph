# ffgraph — current modernisation plan

Updated: 1 November 2026. The current Git checkout and its diffs are authoritative.
This replaces the earlier roadmap that still recommended beginning Pass 8.
Historical checkpoint reports retain their original test counts and scope.

## Current position

FF/sgraph and GML use modern GNU Bison/Flex parsers by default in every CMake
build, including BUILD_TESTING=OFF. Both legacy implementations have been
retired after generated-parser, graph construction, failure and GUI testing.
There is no legacy FF/GML build selector or automatic fallback.

The last committed baseline when this checkpoint began was `f033d04` on
`pass12-modern-ff-parser`. The local annotated tag
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

## Next implementation checkpoint: isolated modern Rome pilot

Rome still uses checked-in Bison++/flex++ output in production. Its migration
has not begun. Start with `FFGRAPH-PASS12-ROME-BASELINE.md`, the actual
`fgraph/RomeParser.y`, `RomeScanner.l`, generated files, and
`FFGraphClass::load()`; preserve the scanner cleanup already covered by tests.

1. Extend the synthetic corpus with representative real Rome graphs when
   available. Freeze independent graph expectations before replacing any code.
   Cover duplicate IDs, ignored metadata, absent endpoints, forward references,
   loops, parallel edges, comments, CRLF, EOF, numeric limits and malformed input.
2. Build an isolated, opt-in GNU Bison/Flex parser/scanner in the build tree.
   Keep the current production parser selected. Use owned semantic values and
   explicit parse results; add syntax/range diagnostics and malformed-input tests.
3. Compare actual node/edge construction with the real legacy parser and
   independent expectations. Parsing success alone is insufficient. Exercise
   file dispatch and both direct legacy entry points; public string load is FF.
   Document any differences in partial-load behavior, diagnostics and numeric
   rejection before integrating. Do not preserve undefined numeric overflow.
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
