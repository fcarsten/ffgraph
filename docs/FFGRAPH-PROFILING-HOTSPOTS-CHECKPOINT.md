# Profiling continuation: imports and crossing counts — 2026-11-02

## Scope and checkout

The user requested further profiling after the Spring stability fix. This
checkpoint starts from clean `647e837` on `import_and_layout_profiling`.
Production graph/parser/layout/container implementations remain unchanged.
No commits, tags or pushes were made. SPQR remains excluded and the parked
WSLg inspector issue was not investigated.

The existing headless profiler, its independent topology checks and the
preceding numerical fix are described in `FFGRAPH-IMPORT-LAYOUT-PROFILING.md`
and `FFGRAPH-SPRING-NUMERICAL-STABILITY.md`.

## Tools and implementation

- `tools/profile_graphs.py` adds optional complete bipartite graphs, limited to
  512 nodes because their edge count is quadratic. Existing default shapes and
  workloads are unchanged. Manifests now also hash the import adapters,
  node-lookup source, crossing implementation and Spring implementation.
- `tools/profile_graphs.cpp` adds `--sugi-crossing-alg 0|1` for diagnostics.
  Default **0** retains the production matrix reduction. Explicit **1** uses
  the accepted, retired bubbling selector, which performs no crossing reduction
  in this implementation. Results and commands record the selector. This is
  not a recommended application setting or an equivalent-quality layout.
- `tools/profile_crossings.cpp` calls the real production `number_of_crossings`
  and a separate tool-only column-count prototype, checking both against an
  independent oracle. The application does not link the prototype.
- `tools/profile_sugi_matrices.py` is a gdb Python diagnostic: it records actual
  reduction parameters and counter-call histograms, and captures the first
  matrix state for each of the three largest distinct geometries. Captures
  retain the actual stride. They are ignored build artifacts, not vendored
  external graph data. Diagnostic runs are never used as timing results.
- The existing `FFGRAPH_BUILD_PROFILER=ON` CMake option builds both optional
  executables with the same production library group. Defaults, application
  installation and normal CTest registration are unchanged.

The prototype scans the existing dense matrix twice per row. It counts how
many earlier-row edges lie strictly to the right of the current edge, then
updates column counts after finishing the row. Delayed updates exclude shared
source vertices; strict column comparison excludes shared targets. Only cells
equal to **1** count, preserving the current matrix interpretation. Its work is
`O(rows × columns)` with `O(columns)` extra storage; the current dense-case
counter has `O(rows² × columns²)` work. Buffer allocation is included in timings.

Production returns `int`. Benchmark oracles check the result fits that range
**before** calling it. The prototype accumulates in `uint64_t`; this checkpoint
does not establish a production overflow policy or extend that API's range.

## Measurement conditions

Same i7-13700K / WSL2 host, GCC 13.3, optimized `RelWithDebInfo` (`-O2 -g
-DNDEBUG`) and frame pointers as the earlier checkpoint. Native series ran
sequentially on logical CPU 0 without concurrent ffgraph tests, builds or
instrumented runs. Host load/frequency remain uncontrolled. File workloads
measure warm-cache headless operation, not GUI or cold storage latency.

Graph cases use one warmup, five measured fresh-graph runs and invariant/oracle
checks outside the clock. Each counter measurement uses one warmup and seven
samples; a volatile function pointer prevents hoisting calls out of repeated
samples. Raw nanoseconds are recorded. Very small timings include clock/call
overhead. Independent verification and matrix materialization are untimed.

## Import scaling

**108/108 graph cases passed**, with no skips: two sparse topologies, six sizes
(250–8,000 nodes), three formats and import/parse/replay phases. Five samples
per case give 540 measured samples plus 108 warmups. Selected public-loader
median milliseconds:

| Topology | Nodes | FF | GML | Rome |
| --- | ---: | ---: | ---: | ---: |
| Chain | 1,000 | 11.197 | 1.208 | 18.123 |
| Chain | 2,000 | 44.744 | 2.858 | 70.689 |
| Chain | 4,000 | 183.863 | 5.613 | 290.693 |
| Chain | 8,000 | 773.744 | 12.536 | 1,174.835 |
| Layered | 8,000 | 1,137.403 | 18.230 | 2,256.548 |

Log-log fits over the four 1,000–8,000-node points give exponents **2.04 FF /
1.11 GML / 2.01 Rome** for chains and **2.08 / 1.01 / 2.02** for layered graphs.
These describe this measured range, not general complexity guarantees. Both
topologies have edge counts proportional to vertex count. Source inspection
and the earlier Callgrind attribution explain the result: repeated
`nodeWithValue()` scans dominate FF/Rome replay; GML already uses an ID map.

A replacement index has important semantic constraints. `FFNodeClass::setValue`
directly assigns the ID without notifying an index. Existing regression labels
mutate IDs during graph construction, and duplicates resolve according to the
live container order, which may change after insertion/rehashing. Merge ID
resets also create collisions. An adapter-local naive map would return different
nodes or miss callback mutations. That optimization remains separate work.

## Actual Sugiyama workload

gdb captured a warmup and one subsequent run for each graph. Both runs agreed
on the following parameters; counter calls are **per layout**, not the combined
diagnostic total. `maxL` is the maximum layer index, so levels equal `maxL+1`.

| Input | Real nodes / edges | Real + dummy nodes | Levels | Maximum layer width | Counter calls |
| --- | ---: | ---: | ---: | ---: | ---: |
| Real Rome `north/g57/g.57.26` | 57 / 241 | 395 | 29 | 107 | 2,604 |
| Bundled `contrib/out.g` | 499 / 499 | 947 | 10 | 265 | 261 |
| Complete bipartite generated GML | 128 / 4,096 | 128 | 2 | 64 | 3 |

The Rome graph has 338 dummy nodes and sparse matrices up to 105×107 with
111 set cells. The bundled FF graph includes a 265×233 matrix with 265 set
cells. Repeated scans over mostly empty matrices explain the cost better than
original node count or input-file size.

**14/14 native graph cases passed in each selector**, without skips. Twelve
complete bipartite format/size combinations (16/32/64/128 nodes) and the two
historical graphs give 28 cases, 140 measured samples and 28 warmups. Median
headless Sugiyama milliseconds:

| Input | Default reduction | Diagnostic bypass |
| --- | ---: | ---: |
| Generated GML, 16 nodes / 64 edges | 0.0473 | 0.0451 |
| Generated GML, 64 nodes / 1,024 edges | 1.7862 | 1.5593 |
| Generated GML, 128 nodes / 4,096 edges | 22.2321 | 19.2616 |
| `contrib/out.g` | 490.0457 | 2.0548 |
| Real Rome 57 nodes / 241 edges | 471.4878 | 1.0192 |

Bypass removes the **entire crossing-reduction stage**, including barycentric
sorting. It changes the layout and is only a cost-isolation experiment. Its
ratios are not an optimization speedup. Dense two-layer input spends a larger
share elsewhere, despite having many more edges.

An existing selector discrepancy surfaced during validation: the constructor's
comment and `FFSugiInsp::command("noneRed")` use **2**, but `setCrossingAlg`
accepts only **0/1**. A first tool smoke run using 2 failed explicitly in all
three formats. The diagnostic option now uses accepted selector 1 and records
it. No production setter or GUI behavior was changed; the GUI control itself
was not manually exercised. This finding is separate from the parked WSLg issue.

## Prototype correctness and isolated performance

Every invocation verifies all **65,536 binary 4×4 matrices** and **128
deterministic rectangular/nonbinary matrices** (including empty dimensions).
The independent oracle enumerates edge pairs with reversed endpoint order;
complete benchmark matrices additionally use `choose(rows,2) × choose(cols,2)`.
Padding/dummy rows contain sentinel ones, so indexing into them would change
the result. Input buffers are checked unchanged.

The native sweep covers **40 synthetic matrices / 80 implementation records**
(square and rectangular, diagonal/reversed/sparse/complete), widths 8–128.
All agree with the oracle. Six captured real matrices add **12 records**;
all three methods agree exactly. Selected per-call median microseconds:

| Matrix | Set cells | Crossings | Production | Tool prototype |
| --- | ---: | ---: | ---: | ---: |
| Complete 16×16 | 256 | 14,400 | 4.830 | 0.226 |
| Complete 64×64 | 4,096 | 4,064,256 | 1,222.446 | 2.811 |
| Complete 128×128 | 16,384 | 66,064,384 | 20,670.813 | 12.910 |
| Rome capture 105×107 | 111 | 649 | 232.974 | 9.679 |
| FF capture 233×229 | 233 | 3,226 | 2,382.137 | 33.812 |
| FF capture 265×233 | 265 | 20,861 | 3,766.413 | 39.819 |

All six real captures show roughly **24–95× faster counter calls**. These
ratios do not establish whole-layout speedups. One region-only Callgrind check
on the captured 265×233 matrix records **83,319,780 production instructions**
versus **685,397 prototype instructions**. The real function, not a copied
reference implementation, supplies the production measurement.

## Checks actually run and remaining work

- Debug baseline **60/60** passed before the tools changed. Post-change CMake
  configure/build and **Debug 60/60**, **Release 62/62** CTests passed, including
  their GUI tests and the two optional external Rome tests configured in Release.
- Strict ASan/UBSan/LSan passed the complete small oracle suite and the
  synthetic sweep through width 32 (24 matrices / 48 implementation records).
  All six captured matrices also passed under strict sanitizers.
- Strict sanitizer graph runs passed **6/6 default-mode** and **6/6 bypass-mode**
  Sugiyama cases (16/64-node complete bipartite graphs in FF/GML/Rome), with one
  measured run plus warmup each. There were no sanitizer suppressions.
- Native graph series passed **136/136** in total, without failures or skips.
  CPU-counter checks and native measurements ran separately. Whitespace,
  binary/source/input hashes and identical selector-workload hashes were checked.
  `nm` confirms the prototype is absent from the normal application binary.
  Full application sanitizer/strict GUI leak suites were not repeated in this
  profiling-only checkpoint; the targeted strict sanitizer runs above are the
  actual sanitizer evidence.

The next suggested production patch is the isolated crossing counter, with
permanent tests and whole-layout verification of geometry/routing and graph
semantics on the captured/historical inputs. Include explicit zero/one-row or
column handling and consider small-case overhead. Preserve the current cell
interpretation; handle the existing `int` range deliberately. Measure the
actual resulting app/core, rather than extrapolating these microbenchmarks.

GUI drawing, interactive latency, 3D performance, animation, alternative
toolchains, cold file I/O and production integration of the prototype remain
unverified. No speedup is claimed for the unchanged application. Installation/
portability and the Spring iteration-control API remain separate follow-ups.

## Reproduction and artifacts

```sh
cmake -S . -B build/profile -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DBUILD_TESTING=OFF -DFFGRAPH_BUILD_PROFILER=ON \
  -DCMAKE_CXX_FLAGS=-fno-omit-frame-pointer
cmake --build build/profile --target profile_graphs profile_crossings --parallel 2

taskset -c 0 python3 tools/profile_graphs.py --binary build/profile/profile_graphs \
  --output build/profile2-imports-local --sizes 250 500 1000 2000 4000 8000 \
  --shapes chain layered --phases import parse replay --samples 5

taskset -c 0 python3 tools/profile_graphs.py --binary build/profile/profile_graphs \
  --output build/profile2-sugi-local --sizes 16 32 64 128 --shapes bipartite \
  --phases layout --layouts sugi --samples 5 --real-ff contrib/out.g \
  --real-rome /home/carsten/work/demographs/rome/north/g57/g.57.26
# Repeat in a fresh directory with --sugi-crossing-alg 1 for the diagnostic bypass.

build/profile/profile_crossings --check-only
taskset -c 0 build/profile/profile_crossings --max-width 128 --repeat 7 > build/crossings-local.jsonl

FFGRAPH_PROFILE_OUTPUT=build/matrices-local gdb -q -batch \
  -x tools/profile_sugi_matrices.py --args build/profile/profile_graphs \
  --phase layout --layout sugi --repeat 1 \
  --input build/profile2-sugi-local/inputs/real-rome-0.rome
# Captured text has rows, columns, stride on its first line, then active cells.
taskset -c 0 build/profile/profile_crossings --repeat 7 \
  --matrix build/matrices-local/matrix-105-107-108.txt

cmake --preset asan -DFFGRAPH_BUILD_PROFILER=ON
cmake --build build/asan --target profile_graphs profile_crossings --parallel 2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
build/asan/profile_crossings --max-width 32 --repeat 1

ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
python3 tools/profile_graphs.py --binary build/asan/profile_graphs \
  --output build/profile2-asan-local --sizes 16 64 --shapes bipartite \
  --phases layout --layouts sugi --samples 1
# Repeat in a fresh directory with --sugi-crossing-alg 1 for the other mode.

# Region-only instruction profile; optional Valgrind headers were available here.
valgrind --tool=callgrind --instr-atstart=no --callgrind-out-file=build/counter-local.cg \
  build/profile/profile_crossings --repeat 1 --implementation production \
  --matrix build/matrices-local/matrix-105-107-108.txt
# Repeat with --implementation prefix for the prototype.

cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug
cmake --preset release -DFFGRAPH_ROME_CORPUS_DIR=/home/carsten/work/demographs/rome
cmake --build --preset release --parallel 2
ctest --preset release
```

Actual artifacts are `build/profile2-imports/`, `build/profile2-sugi-default/`,
`build/profile2-sugi-bypass/`, `build/profile2-results/`, the two
`build/profile2-capture-{rome,out}-final/` directories and
`build/profile2-capture-bipartite/`. Results include raw samples, manifests,
captured matrices and Callgrind files. The strict graph artifacts are
`build/profile2-asan-default/` and `build/profile2-asan-bypass/`; counter logs and
CTest output use `/tmp/ff-profile2-*`. External files were not edited. Artifacts
and captured external data are intentionally not committed.
