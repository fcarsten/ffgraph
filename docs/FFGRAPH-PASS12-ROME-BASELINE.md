# Pass 12 — Rome semantic baseline and scanner cleanup

Follow-up: the [isolated modern pilot](FFGRAPH-PASS12-ROME-PILOT-CHECKPOINT.md)
has since been implemented. The results below describe the earlier baseline
and scanner-cleanup checkpoint.

Date: 1 November 2026. Baseline commit: `f033d04`, branch
`pass12-modern-ff-parser`. This is preparation for an isolated modern Rome pilot.
The production Rome parser and grammar remain historical; no modern Rome
parser/backend option has been implemented or selected. No commits or pushes
were made. Modern FF/GML behavior, SPQR and the parked inspector issue are
unaffected.

## Analysis and compatibility corpus

Read `RomeParser.y`, `RomeScanner.l`, checked-in generated sources/headers and
`FFGraphClass::load()` before adding tests. Rome rows are either two integers
(node ID, ignored metadata) or four integers (edge ID, ignored metadata,
source ID, target ID). Imports force undirectedness. The parser creates nodes
as rows arrive and looks up endpoints in the current graph; it does not resolve
forward references later. Missing endpoints print a warning and skip the edge.
Duplicate IDs create distinct nodes, and lookup among duplicates depends on
legacy container iteration. No node ID map is populated by node actions.

`test_rome_baseline_pass12.cpp` checks real graph construction against
hand-written ID/edge multisets, graph name, directedness, endpoint membership,
load events and merge identity. Eight fixtures in `tests/rome_fixtures` cover
topology, metadata, duplicate/negative IDs, missing/forward endpoints, unknown
punctuation, comments, empty input and three kinds of syntax error. Each runs
through public file loading and three direct Parse variants: stream, string
with CRLF, and string without final LF. Two merge cases bring the total to
**34 import scenarios**. Checks remain active in Release.

Observed semantics to preserve or explicitly reconsider during migration:

- Graph names remain unchanged; parallel edges and loops remain distinct.
- Public filename loading sends START_LOAD/END_LOAD and resets old IDs to -1.
  Direct RomeControl Parse retains old IDs and sends neither event.
- Public string `load(input, NO)` is FF, not Rome. Filename dispatch checks
  `.gml`, then `.g` substrings, then falls back to Rome; `.rome` fixtures use
  that actual path. Extension/substring dispatch was not changed.
- Column-one comments and `#` separator rows are accepted; unknown punctuation
  is silently skipped. CRLF and missing final LF work because the input feeder
  ignores CR and synthesizes LF.
- Syntax errors keep previously constructed nodes/edges. Missing endpoints
  produce stderr warnings without invoking the user error handler.
- Syntax diagnostics report line 1 even when the error occurs later; an early
  newline token rule makes the later line-counter rule unreachable.

These fixtures are synthetic, not a downloaded Rome benchmark collection.
They characterize the current loader, not a claim of conformance to every
Rome format variant. Duplicate-ID pointer choice is deliberately unspecified;
node count, duplicate values and endpoint membership are checked instead.

## Demonstrated defects and targeted fix

The new baseline passed graph expectations but initially failed strict LSan:
**410,690 bytes / 51 allocations** over the initial 26 scenarios. Allocation
stacks led to RomeScanner::yy_create_buffer. The generated scanner has no
destructor; RomeControl's empty destructor never freed its active buffer.

A separate two-input probe using `1 0\nbogus\n` and `1 0\n"quoted"\n` also
confirmed KEY/STRING strdup leaks: **16,480 bytes / 5 reported allocations**,
including 6- and 8-byte text allocations. Some buffer reachability in this small
probe differs from the larger test, so those totals are not an allocation count
per import. The parser rejects both token kinds and never frees their text.

`RomeScanner.l` and checked-in `RomeScanner.cc` now:

- Delete the active scanner buffer once in RomeControl's destructor, when one
  exists. There is no base scanner destructor that would free it again.
- Keep rejected KEY/STRING text in the existing scanner-owned String member
  instead of allocating unmanaged strdup copies. The parser always rejects
  these tokens immediately and never stores their text in graph objects or
  successful grammar reductions. Borrowed text therefore stays alive throughout
  error reporting and is freed with the scanner.

No grammar rule, accepted pattern, numeric conversion, parser API/header,
diagnostic, partial-load policy or graph/container ownership code changed.
Historical Bison++/flex++ regeneration was not attempted; matching source and
checked-in scanner output were edited together and compiled. The modern Rome
pilot will remove that generation limitation later, behind an opt-in path.

## Tests actually run

| Check | Result |
|---|---|
| Initial Rome graph expectations, Debug | PASS before cleanup |
| Initial strict Rome baseline | FAIL; 410,690 bytes / 51 allocations |
| Malformed KEY/STRING probe before cleanup | FAIL; leaked text and scanner buffers |
| Final Rome baseline in Debug and Release | PASS; 34 import scenarios |
| Final Rome baseline strict ASan/UBSan/leaks | PASS; no reported leaks or runtime errors |
| Original malformed-token probe after relinking | PASS; exit 0 with strict sanitizers |
| Final full Debug and Release | 50/50 each |
| Final strict headless suite | 43/43 |
| Final full strict suite | 46/50; four previously isolated GUI toolkit leaks |

Exact overall results, toolchain versions and toolkit investigation are in
`FFGRAPH-PASS12-VERIFICATION-HARDENING.md`. Commands executed:

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug -R '^test_rome_baseline_pass12$'
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release
cmake --preset asan
cmake --build --preset asan --parallel 2
ctest --preset asan-headless --parallel 1
ctest --preset asan --parallel 1  # Four known system GUI toolkit leak failures
```

The small malformed-token probe was compiled separately under `/tmp` against
the actual instrumented FFGraph/scanner archives, using the same graph/header
includes as the permanent baseline test. Its logic was:

```cpp
for (const char* input : {"1 0\nbogus\n", "1 0\n\"quoted\"\n"}) {
  FFGraph graph(YES);
  RomeControl parser;
  parser.Parse(String(input), graph.operator->());
}
```

A returning user error handler was installed. Run environment:
`ASAN_OPTIONS=detect_leaks=1:halt_on_error=1`
and `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`.
Logs: `/tmp/ff-rome-token-leak-before.log`, `ff-rome-token-leak-after.log`, and
`/tmp/ff-verification-test-*.log`. These are ephemeral local evidence, not
committed artifacts.

## Remaining risks and next gate

Numeric overflow still uses legacy atoi/atof; it was not exercised or fixed.
Long-line truncation, embedded NULs, reusing one controller across several
Parse calls, callback/allocation exceptions and GUI Rome File/Merge behavior
need additional coverage. The ownership patch does not claim general parser
reentrancy or exception safety. No independent generated Rome parser exists
yet, so these are baseline/semantic tests, not differential-parser tests.

Next, implement the isolated opt-in modern parser while retaining this baseline
and the production legacy parser. Compare graph construction and document
numeric, diagnostic and failure-atomicity differences before integration.
