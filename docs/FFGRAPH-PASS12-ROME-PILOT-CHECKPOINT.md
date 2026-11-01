# Pass 12 — isolated GNU Bison/Flex Rome pilot

Date: 1 November 2026. Branch: `pass12-modern-rome-parser`.
Starting commit: `11a9304` (`Refactor test assertions to use FF_CHECK macro`).
The checkout was clean. Read the Rome baseline, current roadmap, actual CMake
and legacy grammar/scanner/loading code before implementing the pilot.
No commits, tags or pushes were made in this checkpoint.

## Implementation and scope

The historical sources use Bison++/flex++ class extensions such as `%name` and
`%header`; the checked-in generated files depend on those old interfaces.
Modern GNU tools need translated definitions rather than regeneration of the
unchanged legacy grammar/scanner. The old sources and outputs remain untouched.

Rome generation is modernized behind `FFGRAPH_ENABLE_MODERN_ROME_PILOT`, OFF
by default. Enabling it builds isolated libraries and four additional headless
tests. It does **not** select modern Rome in the application, change
FFGraphClass::load(), replace legacy generated sources or alter public APIs.
FF/sgraph and GML remain modern. SPQR remains excluded; the inspector/WSLg
investigation stays parked.

Files added under `fgraph/modern`:

- `modern_rome.y`: GNU Bison C++ parser, variant semantic values, detailed
  errors, explicit locations and zero expected conflicts. Left-recursive rows
  avoid stack growth with row count while preserving semantic operation order.
- `modern_rome.l`: reentrant Flex scanner with the distinct ffrome_ prefix,
  checked integers, legacy vocabulary/longest-match rules, column-one comments
  and ignored punctuation. Unsupported REAL/KEY/STRING/bracket tokens still
  reject; their text needs no unmanaged allocations.
- `rome_records.{h,cc}`: owned ordered node/edge records with source lines and
  per-invocation state. A scanner guard frees its buffer on ordinary exit or
  C++ unwinding. Failed parses discard partial records; NULs and inputs beyond
  Flex's size limit reject. A final LF is supplied when absent. Location types
  are project-owned, avoiding an untracked generated location-header dependency.
- `rome_graph_adapter.{h,cc}`: experimental replay through existing graph APIs
  and live nodeWithValue lookup at each edge row. Missing endpoints skip edges
  and return warning data with edge ID/endpoints/line. resetIds models successful
  public file-import ID reset; direct Parse semantics retain old IDs.

The isolated adapter emits neither public load events nor ffError: production
lifecycle/error delivery remains the loader's responsibility at integration.
Replay is not transactional if allocation or application callbacks throw.

`tests/modern_rome/CMakeLists.txt` builds standalone without ffgraph/Tk/X11, or
under the optional root target with graph compatibility tests. Generation is
build-local, using Bison `-Wall -Werror` and `%expect 0`. Pilot archives are not
installed or linked into FFGraph/ffgraph. Root CI explicitly enables the pilot
for Debug/Release/ASan coverage, retaining legacy production Rome. BUILD-CMAKE.md
and the roadmap document usage and the next gate. Remote CI was not executed.

## Compatibility corpus and graph expectations

The earlier baseline test and its eight fixtures remain unchanged. Pilot
fixtures add INT_MIN/INT_MAX, lexical quirks, node ID -1, extra fields, indented
comments, REAL rejection and out-of-range IDs/ignored metadata.

The graph driver runs **288 accepted-input combinations**: eight accepted
fixtures × graphic/bare labels × fresh/merge × real legacy stream/string/public
file loading × LF/CRLF/no final LF. It compares actual legacy and modern graph
construction with independent hand-written ID/edge expectations. Public-file
comparisons account explicitly for START_LOAD/END_LOAD; adapter mutations
otherwise produce the same observed event sequence.

Snapshots cover names/IDs, directedness, start/end nodes, adjacency counts,
endpoint membership, edge orientation/multiplicity/weight, custom label payloads,
graphic coordinates/ranges/colors/icons/masks/shapes and complete edge routes.
Further cases check existing merge-node references before new rows, duplicate
lookup and a real label callback changing an earlier node's ID between rows.
A cached parser ID map would give the wrong graph for that callback. Nested
parses from callbacks and four concurrent workers with separate successful/
rejected calls are also exercised.

Duplicate IDs are ambiguous in legacy Rome. The pilot uses the same live
lookup. When reset old IDs collide with imported -1, separate graphs can select
different physical duplicates because of allocation-dependent iteration. All
matrix cases still run: such cases check each graph's lookup result, seed
identity/state, endpoint pointers and independent ID/edge multisets instead of
asserting identical physical duplicate selection/route geometry.

The generated corpus constructs **32 deterministic source-graph structures**,
each with/without graphic labels, and writes Rome records from their actual
IDs/endpoints. Three legacy entry points produce **192 differential imports**.
Additional comparisons use those source graphs as independent expectations,
outside both parsers' semantic code. Cases include empty graphs, isolated nodes,
negative IDs, parallel edges, loops and varied topology up to 63 nodes/252 edges.
The parser-only suite also processes 20,000 rows, a 65,536-character comment,
integer boundaries and repeated malformed-input/recovery sequences.

No external Rome benchmark collection was found in the checkout. Fixtures and
generated graphs are synthetic. There is no modern Rome GUI import path yet;
existing GUI tests establish preserved production behavior only.

## Semantic differences

| Operation/input | Legacy observation | Isolated pilot |
|---|---|---|
| Valid supported rows | Force undirectedness, ignore second integer, preserve name, resolve endpoints in row order | Matches tested graph construction, merges, labels and callbacks |
| Missing/forward endpoints | Warn on stderr and skip edge, without user error | Skip edge and return structured warnings; no automatic stderr delivery |
| Syntax error after valid prefix | Force undirectedness; public file path resets IDs; retain prefix nodes/edges | Reject before replay; retain destination IDs, directedness, contents and callbacks |
| Syntax location after first line | Reports line 1 | Reports actual line and column |
| Integer outside int range, even ignored metadata | Uses atoi; unsuitable as a defined compatibility oracle | Explicit range rejection with no partial records/mutation |
| Overlong comment followed by two node rows | Probe silently stops after truncated comment, constructing zero nodes | Consumes full comment and constructs both nodes |
| Embedded NUL | Feeder uses strlen; no reliable full-input compatibility contract | Explicit preflight rejection |
| Ignored punctuation/unmatched quote without complete string token | E.g. `"2 0` becomes node 2 | Preserved; complete STRING tokens still reject |

Unchanged-on-syntax-rejection is deliberate **pilot policy**, not a production
switch made here. Integration must make the difference explicit, including
ID-reset timing and GUI load lifecycle. No semantic fallback exists. Out-of-range
atoi and embedded-NUL legacy behavior were not used as correctness oracles.

## Tests actually executed

Environment: Ubuntu 24.04, GCC 13.3.0, CMake 3.28.3, GNU Bison 3.8.2, Flex 2.6.4,
system Tcl/Tk 8.6.14, Ninja and real Xvfb. LeakSanitizer/Xvfb required execution
outside sandbox tracing/display restrictions. No leak suppression was added.

| Check | Result |
|---|---|
| Pre-edit configure/build and Debug/Release CTest | 50/50 each |
| Pre-edit full strict ASan/UBSan/leaks | 46/50; four established toolkit leaks |
| Standalone generated parser, Debug | 1/1 passed; no Tk/X11 dependencies |
| Standalone generated parser, strict ASan/UBSan/leaks | 1/1 passed |
| Pilot-enabled Debug configure/build and full CTest | **54/54 passed** |
| Pilot-enabled Release configure/build and full CTest | **54/54 passed**, checks active |
| Pilot-enabled strict headless ASan/UBSan/leaks | **47/47 passed** |
| Pilot-enabled GUI runtime ASan/UBSan, leaks off | **7/7 passed** |
| Pilot-enabled full strict ASan/UBSan/leaks | **50/54 passed**, same four toolkit leaks |
| Fresh default-OFF configure/build/full CTest | **50/50 passed**, option actually OFF |
| Fresh BUILD_TESTING=OFF, pilot ON configure/build | PASS; app and both pilot archives built, zero CTests |
| CTest label/Xvfb audit | PASS; 47 headless + 7 GUI when ON; 43 + 7 when OFF |
| nm of pilot-enabled app/FFGraph and fresh default app | Legacy RomeControl present; ffgraph_rome symbols absent |
| Generator/compiler diagnostics for pilot | No warnings/errors/conflicts in executed builds |
| CI YAML parse, git diff --check | PASS locally; remote workflow **not executed** |

No ASan invalid-access or UBSan runtime error appeared. The four strict failures
retain their established toolkit allocation totals:

- test_load_window_lifetime_pass11: 9,367 bytes / 304 allocations.
- test_ff_window_oracle_pass12: 53,396 bytes / 1,970 allocations.
- test_gui_file_workflow_pass11: 59,697 bytes / 2,208 allocations.
- test_ff_gui_file_workflow_pass12: 59,697 bytes / 2,208 allocations.

GUI runtime checking with leaks off is a separate result; it does not make the
strict suite green. No default/pilot CTest was disabled. The earlier dependency
investigation remains in the verification-hardening report; no package or
shutdown finalizer changed here.

## Executed commands

From the repository root; baseline used the same presets before enabling pilot:

```sh
cmake --preset debug -DFFGRAPH_ENABLE_MODERN_ROME_PILOT=ON
cmake --build --preset debug --parallel 2
ctest --preset debug -R '^test_rome_' --output-on-failure
ctest --preset debug --parallel 2
cmake --preset release -DFFGRAPH_ENABLE_MODERN_ROME_PILOT=ON
cmake --build --preset release --parallel 2
ctest --preset release --parallel 2
cmake --preset asan -DFFGRAPH_ENABLE_MODERN_ROME_PILOT=ON
cmake --build --preset asan --parallel 2
ctest --preset asan-headless -R '^test_rome_' --output-on-failure
ctest --preset asan-headless --parallel 1
ctest --preset asan-gui --parallel 1
ctest --preset asan --parallel 1  # Four known toolkit leak failures

cmake -S . -B build/rome-default-off -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/rome-default-off --parallel 2
ctest --test-dir build/rome-default-off --output-on-failure --parallel 2
cmake -S . -B build/rome-pilot-no-tests -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=OFF -DFFGRAPH_ENABLE_MODERN_ROME_PILOT=ON
cmake --build build/rome-pilot-no-tests --parallel 2
ctest --test-dir build/rome-pilot-no-tests -N
```

Standalone Debug/sanitizer commands in `tests/modern_rome/README.md` were also
executed. Ephemeral logs use `/tmp/ff-rome-pilot-*.log`. Generated sources,
binaries, parser-produced expected snapshots and local logs are not added to Git.

## Remaining verification and next gate

Modern Rome GUI testing awaits opt-in production integration. Real external
Rome graphs, legacy-controller reuse, I/O failure during loading, allocation/
callback exceptions, TSan, inputs near Flex's int size cap, alternate compilers/
platforms and optional large GUI/layout stress profiles remain unverified.
Concurrent ASan/UBSan tests do not substitute for TSan. OOM/fatal Flex allocation
paths were not injected. Installed runtime/portability work and the existing
CMake minimum-versus-LINK_GROUP mismatch remain later tasks.

Next, review these differences, add a selectable production Rome backend with
legacy still default, and test real GUI File/Load/Merge/Close behavior,
error/lifecycle handling and 2D/3D windows. Retain FF/GML and all Rome tests.
Default promotion and legacy retirement require a later checkpoint.
