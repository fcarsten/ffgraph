# Pass 12 — isolated GNU Bison/Flex FF pilot

Current status: modern FF is now mandatory; see the
[legacy retirement report](FFGRAPH-PASS12-FF-LEGACY-RETIREMENT.md).
The latest [verification checkpoint](FFGRAPH-PASS12-VERIFICATION-HARDENING.md)
and [Rome baseline](FFGRAPH-PASS12-ROME-BASELINE.md) prepare the next stage.

Follow-up: [baseline failure fixes](FFGRAPH-PASS12-FF-BASELINE-FIXES.md)
records subsequent fixes and the toolkit leaks exposed after GUI tests could
finish. The results below describe the original pilot checkpoint.

The [integration checkpoint](FFGRAPH-PASS12-FF-INTEGRATION-CHECKPOINT.md)
subsequently adds explicit backend selection and live FF GUI verification.

Date: 1 November 2026. Branch: `pass12-modern-ff-parser`.
Base: `bc2e51a3c7001a78106148fa0af82d01735bad43` (legacy GML retirement).
No commit, push, tag, production-parser switch or legacy removal was performed.

## Starting checkout and baseline

Read `FFGRAPH-PASS12-FF-PARSER-HANDOVER.md` first, then the current CMake,
presets, FF grammar/scanner, graph loader/writer, existing I/O and graph
regressions, GML implementation and relevant Pass 10/11 ownership, semantic,
production-switch and GUI reports. The handover was already untracked; it was
preserved. The starting branch was `main`, otherwise clean. The
`ffgraph-pre-legacy-gml-removal` tag exists; GT parser sources are absent.
The feature branch was created before source changes.

Host: Ubuntu 24.04, GCC 13.3.0, CMake 3.28.3, GNU Bison 3.8.2, Flex 2.6.4,
Ninja 1.11.1, Tcl/Tk 8.6.14. Presets use Ninja, C++17 and smoke GML corpus;
layout-corpus and large GUI stress options were OFF. Xvfb was available.

Before implementation, both preset configurations and full application builds
succeeded. Debug CTest passed **42/42**. ASan/UBSan CTest with leak detection
passed **40/42**. Both failures already existed:

- `test_load_window_lifetime_pass11`
- `test_gui_file_workflow_pass11`

Both report `stack-use-after-scope` in
`FFTclBasicWindow::changeLabel(FFGraphicEdge*, const String&)`, currently
`fgraphic/FFTclBasicWindow.cc:2046–2050`. The local `const char* lid` borrows
`e->getLabelId()`'s temporary `String`, which expires before `FF_TclEvalObjv`.
No suppression, test disabling or unrelated GUI fix was applied.

`ctest --preset debug -R '^test_graph_io_pass10$'` also passed separately.
That existing test includes FF **and GML** round trips; its FF checks include
file/string paths, loops, parallel edges, start/end nodes and `contrib/out.g`
(**499 nodes, 499 edges**). It is not an exclusively FF executable.

Sandbox-only attempts initially failed: Debug **37/42** (five Xvfb tests could
not connect to displays); LeakSanitizer could not work under sandbox tracing.
Full suites were rerun with approval outside the sandbox. The results above
are those real reruns, not claims that the sandbox failures were code fixes.

## Contract and implementation

The production dispatch remains exactly as inspected:

- `FFGraphClass::load(buffer, NO)` uses `FFControl::Parse(String, graph)`.
- File loading chooses GML first for `.gml`, FF for `.g`, Rome otherwise.
- FF resets existing node values to `-1` before parsing and brackets import
  with load events/window update management. The pilot adapter does not wrap
  these GUI lifecycle operations and is not a replacement for `load()`.
- The writer renumbers node values and emits nested per-source edge lists.

The original grammar directly mutates the graph. Targets create nodes before
their declarations; repeated IDs reuse those nodes, including `-1` and existing
merge destinations. Graph names are applied at the completed graph reduction
and only when the destination name is empty. Node geometry operations can
adjust existing routes. Edge point lists reduce right-to-left, prepend points,
then remove the two default endpoints at the edge-name action. Preserve this
ordering rather than collecting unordered attributes.

New files:

- `fgraph/modern/modern_ff.y`: GNU Bison C++ `lalr1.cc`, variant semantic
  values with RAII `std::string`, invocation-specific context, ordered semantic
  operations. The legacy's **23 shift/reduce conflicts** are pinned with
  `%expect 23`: greedy unknown-internal values plus redundant first node/edge
  argument alternatives. A changed count fails generation. Rule order and
  shift resolution are retained; this checkpoint does not simplify the grammar.
- `fgraph/modern/modern_ff.l`: reentrant Flex scanner, separate `ffff_`
  symbols, legacy keyword/unknown-word vocabulary and escape interpretation.
  String mode remains **inclusive**. The comment rule excludes its final
  brace from matching and consumes it separately, retaining the legacy's
  longest-match behavior even inside strings.
- `ff_events.{h,cc}`: per-call parse result and scanner guard. Flex owns the
  active scan-bytes buffer; only scanner destruction releases it. Tokens and
  discarded Bison symbols use C++ lifetime management. Failed parses discard
  partial operations. No parser-global unmatched-label set is introduced.
- `ff_graph_adapter.{h,cc}`: replay into real `FFGraphClass`, using public
  graph/graphics/label operations. Preserves ordered forward references,
  names, directedness, start/end nodes, hundredths weights, 2D/3D node and route
  geometry, colors, icon/mask variants, label callbacks and window operations.
  It optionally resets IDs on successful replay for comparison with `load()`.
- `tests/modern_ff/`: real generated-parser tests and native fixtures.
- Root `CMakeLists.txt`: six added lines for
  **`FFGRAPH_ENABLE_MODERN_FF_PILOT=OFF`** by default. When ON, sources and the
  Bison state report are generated only under the build tree. The parser
  archive and test adapter are isolated from the application link graph.

The production FF grammar/scanner and checked-in generated files, loader,
Rome parser, GML implementation, GUI and containers have no diff. Ninja's
`ffgraph` link dependencies were inspected with the pilot ON: the modern FF
archive is absent. SPQR remains excluded. Existing inspector CTest coverage
was run as part of the full suites; the parked WSLg issue was not investigated.

## Compatibility corpus and graph oracles

Eight hand-written files under `tests/modern_ff/fixtures/`:

| Fixture | Coverage |
| --- | --- |
| `empty.g` | Empty graph, absent directedness defaults to directed |
| `structure.g` | Undirected, forward/undeclared targets, duplicate declaration, disconnected negative ID, parallel edges, loop, start/end, coordinates, radius, color, weights |
| `attributes.g` | Graph/node/edge label callbacks and absent label type; graph offsets/transforms; numeric/string unknown internals; raw XY, NP/3D, mixed 2D/3D routes; all icon/mask forms |
| `strings.g` | Escaped quote/backslash, literal unknown escapes, raw multiline text and escaped newline, empty label |
| `comments.g` | Comments between syntax elements, punctuation, ignored graph internals |
| `one-point.g` | Legacy short route and default-endpoint removal behavior |
| `string-comment.g` | Inclusive scanner comment/string collision |
| `signed-ids.g` | `-1`, zero, signed int minimum/maximum and forward targets |

Each valid fixture is loaded into separate legacy/modern graphs, with and
without graphical labels and with and without an existing named merge graph.
Legacy string loading uses the production `load(..., NO)`; an independent
production `load(path, YES)` checks every fixture against modern parsing of the
same file bytes. Both implementations also construct the real historical
`contrib/out.g`, with and without graphics. Modern historical counts are
independently asserted as 499/499.

Snapshots sort nodes/edges to avoid allocation/iteration order assumptions.
They compare graph name/directedness, numeric node IDs, names, start/end nodes,
endpoint membership, directed topology and parallel multiplicity, weights,
all node coordinates/ranges/colors/icons/masks/shapes, edge route coordinates
and colors, and real custom graph/node/edge label callback payloads. Edge IDs
are excluded because the FF format never assigns them. No display is used.

Additional permanent C++ oracles independently assert known IDs, names,
topology, multiplicities, geometry/bends, start/end designation, weights, label
payloads and icon/mask results. Expectations do not call the legacy parser or
writer to obtain their values. These tests survive eventual legacy removal.

Twenty-four deterministic writer graphs cover both directedness values, 1–24
nodes, loops, parallel edges, disconnection, start/end, 3D geometry, route bends
and fractional/negative weights. Each writer output is independently loaded by
both backends; modern graph snapshots also match the **original source graph**.
Modern reserialization is compared again through both parsers.

Rejection tests repeat 13 malformed cases 20 times, followed by a successful
fresh parse on every cycle. They cover missing headers/names/END, truncated
node/edge internals, bad coordinate cardinality, nested internals, unexpected
suffixes, `GRAPHS`, numeric overflow, incomplete escapes and carriage returns.
They verify failed-result rejection, discarded operations, original graph
snapshots, original node address and original IDs. Further checks cover signed
int bounds, embedded NUL rejection, multiline diagnostic position and 10,000
flat unknown attributes exercising Bison's dynamically growing stack. FF
internals are not a recursively nested format; nested blocks are rejected.

CTest adds three tests when the pilot is enabled:

- `test_ff_legacy_corpus_pass12`
- `test_ff_compatibility_pass12`
- `test_ff_modern_failures_pass12`

## Observed differences and explicit pilot boundaries

No constructed-graph differences were found on the valid differential corpus.
That is bounded evidence, not full production replacement approval.

1. **Malformed input atomicity deliberately differs.** The legacy diagnostic
   command `--legacy-malformed` demonstrates an existing node ID changing
   from 123 to -1 and a partially created second node after missing `END`.
   The same command under ASan/LSan fails with a **4-byte, one-allocation leak**
   from `strdup` of the retained graph name in `FFScanner::ScanValue`.
   This diagnostic is separate from the three success regressions; it is
   explicitly reported as failing under LSan, not hidden with suppression or
   marked as an expected CTest pass. The legacy parser remains untouched.
   Modern failures leave populated graphs intact and leak checks pass.
2. **Numeric range:** the pilot rejects values outside signed `int`, rather
   than relying on legacy `atoi` overflow. Both in-range endpoints are tested;
   no portable legacy result for overflow is claimed.
3. **Diagnostics:** the pilot counts newlines in raw multiline strings and
   comments; the legacy increments only standalone/escaped-newline rules.
   Modern line 4 reporting across a multiline string is tested. Message text
   and legacy undercounting are not compatibility promises.
4. **Unknown-label warnings:** modern suppression is per adapter invocation.
   Legacy suppression is process-global, cleared by string parsing but not
   by stream parsing. Callback results are compared; cross-load stderr warning
   suppression is not identical and is not used as a graph oracle.
5. **Input boundaries:** embedded NUL is explicitly rejected, and scan-bytes
   length is bounded. Modern scanning does not copy the legacy line-oriented
   `getline` feeder, including its fixed-buffer long-line truncation. A long
   modern internals line is tested; parity at legacy feeder limits is unverified.
6. Modern parsing validates all syntax before callbacks. Valid replay operation
   order is preserved, but callback timing relative to scanner reads differs.
   Allocation/callback exceptions during graph replay do not roll back.
   Flex's default fatal allocation handling is not an exception-safety guarantee.

## Tests actually executed

| Configuration | Configure/build | Full CTest result | New FF cases |
| --- | --- | --- | --- |
| Before patch: Debug | PASS | 42/42 PASS | Not present |
| Before patch: ASan/UBSan, leak detection ON | PASS | 40/42 PASS, two existing GUI lifetime failures | Not present |
| After patch: default Debug, pilot OFF | PASS | 42/42 PASS | Not registered by design |
| Pilot Debug | PASS, real Bison/Flex generation | 45/45 PASS | 3/3 PASS |
| Pilot Release (`-O3 -DNDEBUG`) | PASS, real Bison/Flex generation | 43/45 PASS | 3/3 PASS |
| Pilot ASan/UBSan, leak detection ON | PASS, real Bison/Flex generation | 43/45 PASS, same two baseline GUI lifetime failures | 3/3 PASS, no findings |

Release failures are `test_mplex_stl` (`MPlex: no unused index`) and
`test_container_mplex_pass8` (`MPlex: delete low from empty plex`). Their
unchanged test bodies put mutations inside `assert()`; Release removes those
mutations. Both failures were separately reproduced by compiling test sources
read from **Git HEAD** with `-O3 -DNDEBUG` against the unchanged container code.
These targets do not link either parser. They were not disabled or fixed here.

The separate `--legacy-malformed` diagnostic passed its partial-mutation
assertions in Debug and **failed** leak detection under ASan as described
above. It is not included in the 3/3 modern/pilot success claim.

All suites ran the existing Pass 11 modern GML and registered Tk/Xvfb tests;
Debug passed them all. Full sanitizer GUI success is not claimed. No tests
were skipped by CTest. Optional large GUI and layout-corpus tests were OFF.
`git diff --check` passed. New sources/fixtures were also checked for whitespace.

## Commands and reproduction

Actual baseline (run before source edits):

```sh
git status --short --branch
git rev-parse HEAD
git log -1 --format='%h %cs %s'
git tag --list ffgraph-pre-legacy-gml-removal
git ls-files 'fgraph/*Parser*' 'fgraph/*Scanner*'
cmake --list-presets
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure
ctest --preset debug -R '^test_graph_io_pass10$' --output-on-failure
cmake --preset asan
cmake --build --preset asan --parallel 2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
ctest --preset asan --output-on-failure
```

Actual pilot/final commands (CTest ran outside sandbox restrictions):

```sh
cmake -S . -B build/ff-pilot-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DFFGRAPH_ENABLE_MODERN_FF_PILOT=ON
cmake --build build/ff-pilot-debug --parallel 2
ctest --test-dir build/ff-pilot-debug --output-on-failure

cmake --preset release -DFFGRAPH_ENABLE_MODERN_FF_PILOT=ON
cmake --build --preset release --parallel 2
ctest --test-dir build/release --output-on-failure

cmake --preset asan -DFFGRAPH_ENABLE_MODERN_FF_PILOT=ON
cmake --build --preset asan --parallel 2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
ctest --preset asan --output-on-failure

# Focused FF success regressions, with the appropriate test directory:
ctest --test-dir build/ff-pilot-debug \
  -R '^test_ff_(compatibility|legacy_corpus|modern_failures)_pass12$' \
  --output-on-failure

# Explicit legacy malformed-input characterization; sanitizer run FAILS (leak).
build/ff-pilot-debug/tests/modern_ff/test_ff_compatibility_pass12 --legacy-malformed
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
build/asan/tests/modern_ff/test_ff_compatibility_pass12 --legacy-malformed

# Default build retains OFF in its cache; explicitly set OFF when reusing
# a directory that previously enabled the pilot.
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure
ninja -C build/ff-pilot-debug -t query ffgraph
git diff --check
git status --short --branch
```

Session logs are under `/tmp/ff-pass12-*` (ephemeral, not tracked), including
`baseline-{debug,asan}-ctest-unrestricted.log`, `{debug,release,asan}-ctest.log`,
`default-ctest.log` and `legacy-malformed-{debug,asan}.log`.

## Unverified / remaining work

- Modern FF window transforms, offsets, live graphic callbacks, and FF GUI
  File → Load/Merge/Close/Reload through a selectable backend are **unverified**.
  Window operations exist in the adapter but headless tests only cover their
  no-window path. Existing GUI tests exercise the unchanged application/GML.
- No production FF switch, wrapper/API replacement, install integration of the
  pilot, legacy removal, Rome changes or SPQR work is included.
- No interactive WSLg session, fuzz campaign, allocator/callback fault injection,
  huge input-limit allocation, or exhaustive historical FF archive was tested.
- The input feeder, invalid-input policy, diagnostic suppression and callback
  timing differences above must be reviewed before selecting a production backend.
- Fix the independently reproduced GUI lifetime and Release-test issues in
  separate patches; neither full Release nor full sanitizer CTest is green.

This checkpoint establishes actual generated-parser and headless graph semantic
compatibility evidence while preserving the working production FF fallback.
