# Import and layout profiling checkpoint — 2026-11-02

Historical measurements below predate the Spring stability fix. See
[the numerical stability report](FFGRAPH-SPRING-NUMERICAL-STABILITY.md) for the
root cause, correction, regression tests and subsequent verification. The
earlier three Spring failures remain recorded here as the original findings.

## Scope and starting state

The user chose import/layout profiling after retiring all three legacy parsers.
The checkout started clean at `e3c98cf` on `pass12-modern-rome-parser`.
No commits, tags or pushes were made in this checkpoint. SPQR remains excluded
and the parked inspector/WSLg issue was not investigated.

Before edits, CMake configuration/build and Debug **59/59**, Release **61/61**
and strict headless ASan/UBSan/LSan **49/49** passed. Release includes the two
optional external Rome corpus tests. The new profiling driver is opt-in and
does not change the normal app or CTest workload.

## Measurement implementation

`FFGRAPH_BUILD_PROFILER=ON` adds `profile_graphs`, using the production static
libraries and generated parsers. `tools/profile_graphs.cpp` separates:

| Phase | Timed region | Work outside the region |
| --- | --- | --- |
| `import` | Public `FFGraphClass::load(file, YES)`, including reading, parsing, replay and load events | Empty graph/prototype setup, validation, destruction |
| `parse` | In-memory generated parser/scanner and result assignment | File reading; graph replay and validation afterwards |
| `replay` | Production event/record-to-graph adapter, including GML preflight | File reading and parsing once before the runs |
| `layout` | Real Random, Spring or Sugiyama `layout()` | Import, label attachment, incidence snapshot, validation, destruction |

Each case runs in its own process with one warmup followed by fresh graphs for
each measured sample. Spring/Sugiyama initialization bypasses only inspector
creation, following the existing headless tests. No Tk window is created.
Graph and label destruction remains active outside the timed region.

Validation uses exceptions, independent of `NDEBUG`. Generated graphs have
independent ID/endpoint-multiset oracles, including orientation and multiplicity.
Layout checks preserve the node pointer set, edge objects and their endpoints,
and check finite node/route coordinates, graphic labels and directedness.
Numeric node/edge values may change, as documented by the layout algorithms.
Real-file imports check valid membership/routes and repeated topology; this
is **not** an independent full attribute oracle for those files. The existing
semantic and external-corpus CTests remain the stronger compatibility checks.

`tools/profile_graphs.py` generates chain, star and layered graphs in all three
formats, records SHA256s and byte sizes, and runs the driver with timeouts.
It emits `results.jsonl`, raw per-case stdout/stderr, `summary.csv`,
`summary.json`, and a manifest containing hardware, CPU affinity, Git state,
binary hash and CMake configuration. The runner now also records profiling
source hashes. A nonzero child exit, timeout or invalid sample fails the run;
no timing threshold is imposed. A node limit explicitly records skipped layouts.

All synthetic nodes use the default coincident `(100,100,0)` position. Every
destination starts undirected, making the three representations comparable:
Rome/FF set undirectedness and the existing GML adapter retains destination
directedness instead of applying `directed 0`. That behavior was preserved.
Graph names and edge numeric IDs are format-specific and are not cross-format
identity criteria. Random seeds are reset before each run; pointer/container
iteration order and final coordinates are not promised deterministic.

Spring uses normal quality **2**, animation off and edge weights on. Its ceiling
is `max(30, 10000 / node_count)`, with possible earlier convergence. The driver
reports this ceiling, not an actual iteration count. `setMaxIteration()` alone
does not bound quality mode: its separate branch requires a negative quality,
which the public setter rejects. This pre-existing API inconsistency and older
tests' claims of bounded iterations remain follow-up work; they were not changed.
Sugiyama uses its production defaults (deterministic, crossing algorithm 0).

Memory is Linux `/proc/self/status` `VmHWM` in KiB, for the entire case process,
including warmup, parsing retained for replay, validation and teardown effects.
It is **not** a phase-specific allocation total. `getrusage` initially showed
the Python parent's approximately 39 MiB inherited high-water mark on this host;
VmHWM avoids that pre-exec contamination. The driver requires Linux procfs.

## Correctness fix exposed by scale

Before the fix, valid 5,000-node GML chains, stars and layered graphs were
rejected with `memory exhausted`. `attributes : attribute attributes` retained
one stack frame per sibling until list completion, hitting Bison's default
10,000-entry maximum even with very shallow nesting.

`fgraph/modern/modern_gml.y` now uses left recursion,
`attributes : attributes attribute`. Attributes still emit their events in
source order and nested begin/end actions keep their ownership/cleanup rules.
This bounds stack use with respect to sibling count; deeply nested input still
has a stack limit. No format syntax or adapter behavior changed.

Both new regressions were executed **before** the grammar change and failed:

- Parser regression: 20,000 sibling integer attributes, exact event ordering,
  and the same wide list truncated before its close for error cleanup.
- Graph regression: an independently constructed expected snapshot for 6,000
  nodes and 5,999 edges, checking every ID, label, XY/Z and edge endpoint/ID/label.

They now pass in Debug, Release and strict sanitizers. The graph check extends
`tests/modern_gml/test_gml_graph_regression_pass11.cpp`; the event check extends
`test_gml_modern_pass11.cpp`. The profiling matrix also exercises the larger
files through the actual public file-loading API.

## Measured workloads and results

Host: Intel Core i7-13700K, WSL2 Linux `6.18.54.1-microsoft-standard-WSL2`, GCC
13.3.0, CMake 3.28.3, Valgrind 3.22.0. The measured build uses
`RelWithDebInfo` (`-O2 -g -DNDEBUG`) and `-fno-omit-frame-pointer`. Timing runs
were pinned to logical CPU 0 and ran without concurrent ffgraph builds/tests
or profiler instrumentation. Host load, CPU frequency and filesystem caches
were not controlled. Inputs were generated/copied before warmup: these are
warm-cache measurements, not cold storage benchmarks.

Five samples per case; selected **median milliseconds** below. Separate phase
runs have different allocator/cache/result lifetimes; their medians should not
be treated as exactly additive.

| Topology | Nodes / edges | FF import | GML import | Rome import |
| --- | ---: | ---: | ---: | ---: |
| Chain | 100 / 99 | 0.222 | 0.157 | 0.249 |
| Chain | 1,000 / 999 | 11.102 | 1.540 | 17.673 |
| Chain | 5,000 / 4,999 | 305.668 | 7.338 | 418.025 |
| Star | 5,000 / 4,999 | 449.717 | 7.082 | 420.340 |
| Layered | 5,000 / 9,789 | 388.801 | 11.048 | 840.172 |

The 5,000-node layered case has parse-only times **1.806 / 3.705 / 1.353 ms**
and replay-only times **364.797 / 6.454 / 822.948 ms** (FF / GML / Rome).
Whole-process import peaks for that case were approximately **24.6 / 27.7 /
22.4 MiB** respectively. Allocator/pointer placement affects lookup order and
variation; the manifest/raw samples retain the limits of this measurement.

| Loaded graph | Nodes / edges | Random | Spring | Sugiyama |
| --- | ---: | ---: | ---: | ---: |
| Generated GML chain | 1,000 / 999 | 0.216 | 283.332 | 2.221 |
| Generated GML layered | 1,000 / 1,906 | 0.330 | 285.399 | 11.963 |
| Historical `contrib/out.g` | 499 / 499 | 0.119 | 72.725 | 496.736 |
| Real Rome `grafo10106.100` | 100 / 119 | 0.039 | 9.592 | 4.606 |
| Real Rome `north/g57/g.57.26` | 57 / 241 | 0.060 | 7.186 | 470.762 |

Also imported historical `/home/carsten/work/graphs/falk2.g` and `nase2.gml`
(each 1,096 nodes / 1,691 edges): **30.797 ms FF**, **2.681 ms GML**. Equal
counts do not establish that these historical files are controlled equivalents.
Their layouts were skipped by the documented 1,000-node ceiling. External
datasets were copied into ignored build artifacts for stable dispatch and were
not edited or committed.

The complete native matrix records **156 passed cases, 3 failed, 33 skipped**.
Passed cases yield 780 measured samples plus 156 warmups. The 33 skips are
the 27 generated 5,000-node layouts and six historical-file layouts over the
node ceiling. All generated import/parse/replay cases now pass. The three
failures are the same Spring numerical issue in each format, below.

## CPU attribution

Six successful Callgrind captures restrict instrumentation to the timed region
using optional Valgrind client requests (`--instr-atstart=no`). The headers are
optional at build time; this host has them. If absent, Callgrind region markers
compile to no-ops and this capture command will not collect useful data.
Instruction percentages below are **Ir**, not wall-time percentages; inclusive
figures include callees and must not be added to their children.

- Layered 1,000-node replay: `FFGraphClass::nodeWithValue()` accounts for
  **85.89% inclusive FF instructions**, **94.72% Rome**. Both adapters repeatedly
  scan all graph nodes. `VOHSet` Pix validation/iteration adds repeated hash
  membership lookups inside those scans. GML already uses an ID map and its
  5,000-node replay profile is led by allocation (`_int_malloc`: 24.71% self).
- Layered 1,000-node Spring: `compute_forces_edges()` accounts for **93.96%
  inclusive**, 42.47% self; point arithmetic is prominent among its callees.
- Real 57-node / 241-edge Sugiyama: crossing reduction accounts for **99.81%
  inclusive**; `number_of_crossings()` is **69.35% self**, `b_r()` 15.36%,
  `b_c()` 14.87%. Node/edge count alone does not predict layout cost.
- Layered 1,000-node Random: `moveTo()` is 63.80% inclusive, largely graphic
  endpoint/route notifications. Its absolute time is small here.

`perf` software sampling also captured **819 user CPU-clock samples**, zero
lost, for 5,000-node Rome replay. This WSL perf binary lacks libelf and DWARF
unwind support: its report shows raw ELF-relative IPs and no usable symbolized
call graph. `addr2line -Cfipe` on the retained measured executable maps the hot
IPs to the container's `std::_Hashtable` lookup code, consistent with Callgrind.
No installation of extra tools or host perf settings was needed. A fully
symbolized perf call graph and hardware-counter measurements remain unverified.

## Remaining Spring failure and priorities

A 1,000-node star with all nodes at their normal initial `(100,100,0)` position
produces nonfinite coordinates in Spring quality 2. All three importers build
the expected topology first; Random/Sugiyama pass it. Spring prints `Breaking ...`
and the driver's finite-coordinate guard fails during warmup, returning nonzero.
The failure also reproduces under strict ASan/UBSan/LSan without a memory/UB
diagnostic. This does not establish a numerical root cause or a general fix.
No Spring failure was suppressed and no layout algorithm was changed.

Recommended next changes, each with focused regression coverage:

1. Isolate/fix the coincident-star numerical failure before changing Spring's
   performance or initial positions. Clarify its iteration-control API separately.
2. Address repeated FF/Rome ID scans while preserving duplicate-ID selection,
   forward references, merge resets and label callback mutations. A naive cached
   ID map is not semantically equivalent for all supported inputs. Avoid a broad
   container rewrite as the first performance patch.
3. Investigate repeated crossing counts in Sugiyama using the dense real graph,
   preserving deterministic ordering and layout invariants.

GUI rendering, inspector creation, animation, 3D layout, Ortho/Naive/Kaese,
cold file I/O, larger layouts and performance on other hosts were not profiled.
Installed asset lookup/Close-versus-Quit behavior and portability remain the
separate planned checkpoint. Full strict GUI leak diagnostics were not repeated;
the known fontconfig shutdown findings from the retirement checkpoint remain.

## Verification and reproduction

After the grammar fix: **Debug 59/59**, **Release 61/61**, strict headless
**ASan/UBSan/LSan 49/49**, GUI sanitizer runtime **10/10** with the existing
`detect_leaks=0` GUI preset. This last result is not a GUI leak-freedom claim.
The new driver additionally passed **108 cases** (sizes 12/100, all three
shapes/formats, all four phases/three layouts), two samples plus warmup each,
under strict sanitizers. No sanitizer suppression or failing-test removal was
introduced. Git whitespace checks passed.

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug
cmake --preset release -DFFGRAPH_ROME_CORPUS_DIR=/home/carsten/work/demographs/rome
cmake --build --preset release --parallel 2
ctest --preset release
cmake --preset asan -DFFGRAPH_BUILD_PROFILER=ON
cmake --build --preset asan --parallel 2
ctest --preset asan-headless
ctest --preset asan-gui

cmake -S . -B build/profile -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DBUILD_TESTING=OFF -DFFGRAPH_BUILD_PROFILER=ON \
  -DCMAKE_CXX_FLAGS=-fno-omit-frame-pointer
cmake --build build/profile --target profile_graphs --parallel 2

# Output must be a new directory. Full matrix returns 1 for the known star issue.
taskset -c 0 python3 tools/profile_graphs.py --binary build/profile/profile_graphs \
  --output build/profile-import-layout-local --sizes 100 1000 5000 --samples 5 \
  --real-ff contrib/out.g --real-ff /home/carsten/work/graphs/falk2.g \
  --real-gml /home/carsten/work/graphs/nase2.gml \
  --real-rome /home/carsten/work/demographs/rome/gfinali/graficon100nodi/grafo10106.100 \
  --real-rome /home/carsten/work/demographs/rome/north/g57/g.57.26

ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
python3 tools/profile_graphs.py --binary build/asan/profile_graphs \
  --output build/profile-sanitizer-local --sizes 12 100 --samples 2

# Minimal failing layout (using an already-generated full matrix).
build/profile/profile_graphs --phase layout --layout spring --repeat 1 \
  --input build/profile-import-layout-local/inputs/star-1000.g \
  --oracle build/profile-import-layout-local/inputs/star-1000.tsv

# A representative region-only profile; requires optional Valgrind headers.
valgrind --tool=callgrind --instr-atstart=no \
  --callgrind-out-file=build/profile-rome.callgrind \
  build/profile/profile_graphs --phase replay --repeat 1 \
  --input build/profile-import-layout-local/inputs/layered-1000.rome \
  --oracle build/profile-import-layout-local/inputs/layered-1000.tsv
callgrind_annotate --auto=no --inclusive=yes build/profile-rome.callgrind
```

Local timing artifacts: `build/profile-import-layout-final/`; strict driver
artifacts: `build/profile-final-asan-smoke/`; final optimized driver smoke:
`build/profile-final-smoke/` (**54/54 cases**); six Callgrind captures, perf data,
annotations and the measured executable: `build/profile-cpu/`. The timing
manifest's binary SHA256 matches that retained executable. Subsequent CLI help
and provenance-only additions do not change timed operations. Earlier
`profile-import-layout2` was exploratory, overlapped builds, and contains the
pre-fix GML failures; its timings were not used above. Logs use
`/tmp/ff-profile-*.log`. Build artifacts, external files and profiler output
are intentionally outside Git.
