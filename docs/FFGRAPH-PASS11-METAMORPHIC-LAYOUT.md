# FFGraph Pass 11 — metamorphic GML import/layout regression

## Scope

This patch follows the graph corpus and its first-build fix. It is an **opt-in,
test-only extension**, not a production GML parser switch. The purpose is to
measure which invariants survive harmless changes to the input representation,
without comparing nondeterministic spring-layout coordinates byte-for-byte.

## New files and integration

- `tests/graph_corpus/metamorphic.py` generates deterministic GML input pairs
  from the existing independent graph corpus. The generated files are **not**
  committed to Git.
- `tests/graph_corpus/test_metamorphic.py` validates transformations, topology,
  coordinate relations, checksums, repeated generation, and safe `--force`
  replacement. Existing Python generator tests are retained.
- `tests/test_graph_corpus_metamorphic_pass11.cpp` uses the real legacy importer
  and the real headless spring algorithm to validate each pair.
- Root `CMakeLists.txt` registers opt-in generation, an executable and a CTest
  case under the existing `FFGRAPH_ENABLE_CORPUS_LAYOUT_TESTS` switch.
- Python CTest discovery includes both `test_generate.py` and
  `test_metamorphic.py` (also in the standalone modern-GML project).
- The manually triggered `gml-parser-check.yml` job includes the new C++
  metamorphic test in both normal and ASan/UBSan configurations. That CI
  change has **not** been run on GitHub here.
- `tests/graph_corpus/README.md` documents the new format and usage.

## Metamorphic relations

| Transformation | Expected relation **before** layout |
|---|---|
| `relabel_ids` | Bijection of numeric node IDs; node labels and edges by labels unchanged |
| `reorder_declarations` | Reversed node and edge declaration order; identities and topology unchanged |
| `reverse_edges` | Source/target swapped for every edge, valid only for undirected graphs |
| `translate_xy` | Every XY becomes `(x + 64.125, y - 32.750)` |
| `scale_xy` | Every XY becomes `(2x, 2y)` |

Each pair preserves graph name, vertex labels, edge IDs/labels, the undirected
edge multiset (including loops and parallel multiplicity) and graph size. The
post-layout oracle checks finite XY, valid edge routes, node labels and graph
incidence, rather than exact pixel locations.

**Historical compatibility note:** `FFSpring::layout()` explicitly states that
it changes node and edge `value` fields. The initial version of this test
correctly detected those changes; the revised test recognises the documented
contract. Numeric IDs are checked during **import**, not after layout.

## Local verification

- Generator Python tests: **8/8 passed** (six original + two metamorphic).
- Smoke corpus: **11 original graphs, 55 transformed pairs**, deterministic
  regeneration and independent Python grammar checks passed.
- Headless C++ GML import plus bounded spring layout for **55/55 smoke pairs**
  and **75/75 standard pairs** passed against the available real ffgraph core
  library and Tcl runtime.
- A separate **ASan/UBSan smoke run passed 55/55 pairs** using the existing
  sanitizer-instrumented Pass 9 core. `ASAN_OPTIONS=detect_leaks=0` was used
  because that prebuilt core predates the Pass 10 scanner leak fix. This is
  **not** a leak-detected validation of the latest combined sources.
- The local linked libraries were built during Pass 9 and therefore are
  **not** end-to-end verification of the latest Pass 10 scanner memory fix or
  modern Bison/Flex parser. The full project CMake/CTest build still requires
  missing development dependencies on this host.

## Ubuntu commands

```sh
python3 -m unittest discover -s tests/graph_corpus -p 'test_*.py' -v
cmake -S . -B build/pass11-corpus -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  -DFFGRAPH_ENABLE_MODERN_GML=ON \
  -DFFGRAPH_ENABLE_CORPUS_LAYOUT_TESTS=ON
cmake --build build/pass11-corpus --parallel 2
ctest --test-dir build/pass11-corpus \
  -R 'test_graph_corpus_(generator|layout|metamorphic)_pass11|test_gml' \
  --output-on-failure
```

For a sanitizer run, use a fresh build directory with
`-DFFGRAPH_SANITIZERS=ON`, `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1`
and `UBSAN_OPTIONS=halt_on_error=1`. For generated corpus inspection:

```sh
python3 tests/graph_corpus/metamorphic.py \
  --output-dir build/metamorphic-smoke --profile smoke --force
python3 tests/graph_corpus/metamorphic.py \
  --output-dir build/metamorphic-smoke --profile smoke --check
```

## Limitations and future work

- This validates the **legacy importer** (and the existing isolated Bison/Flex
  tests separately). It does not establish parity for the new modern event
  parser against these metamorphic pairs.
- The spring-layout regression checks safety and topology, **not** layout
  quality, affine equivariance, convergence or reproducibility of final XY.
- Directed edges need their own corpus with explicit orientation invariants.
- Profile stress cases should run separately from the default CTest smoke gate.
- Keep production GUI, input APIs and formats unchanged; SPQR stays excluded
  and the inspector/WSLg issue remains parked.
