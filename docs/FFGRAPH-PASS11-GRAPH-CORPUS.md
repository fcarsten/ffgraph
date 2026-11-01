# FFGraph Pass 11 — deterministic graph / GML regression corpus

## Purpose and scope

This incremental checkpoint follows the Pass 11 GML semantic-parity patch and
its parser ownership fixes. The **production GML importer and Tcl/Tk UI are
unchanged**. It provides a deterministic generator and checked graph oracles
that can be reused for modern-parser tests, legacy/modern graph parity tests,
container/graph correctness and layout algorithm benchmarks.

## Changes

1. `tests/graph_corpus/generate.py`: grammar-shaped GML emission, topology
   families, reproducible seeded random graphs, independent strict syntax
   validation, stable metadata, expected negative cases and `--check` mode.
2. `tests/graph_corpus/test_generate.py`: generated source/manifest integrity,
   profile counts, determinism, grammar boundaries, oracle consistency,
   safe refusal to overwrite arbitrary directories with `--force`, and
   historical example categorisation.
3. Six user-supplied historical examples are preserved byte-for-byte. Five are
   grammar-valid attribute documents. `options.gml` contains the AGD options
   format and is not valid under the experimental GML key/value parser.
4. `tests/modern_gml/test_gml_corpus_pass11.cpp`: runs all generated fixtures
   through the real opt-in Bison/Flex event parser, including repeated scanner
   construction and malformed inputs.
5. `tests/modern_gml/test_gml_graph_parity_pass11.cpp`: adds corpus-based
   graph-state parity comparisons to the existing semantic parity driver.
6. `tests/test_graph_corpus_layout_pass11.cpp`: checks legacy-imported graph
   name, IDs, labels, initial XY coordinates and full edge multiset against
   independently generated TSV oracles; then applies bounded headless spring
   layout and checks finite geometry and edge route integrity.
7. CMake: Python generator check in the normal test suite; new modern-parser
   corpus tests; and opt-in `FFGRAPH_ENABLE_CORPUS_LAYOUT_TESTS`.

## Coverage

| Profile | Graph fixtures | Total vertices | Total edges |
|---|---:|---:|---:|
| smoke | 18 | 134 | 152 |
| standard | 26 | 443 | 743 |
| stress | 30 | 2,634 | 4,060 |

Examples cover empty and singleton graphs, isolates, paths, cycles, stars,
cliques, grids, trees, ladders, wheels, bipartite graphs, disconnected graphs,
self-loops, duplicate/parallel edges, nonconsecutive IDs, coincident and
negative coordinates, collinear starts, and sparse/dense random graphs.

Each profile adds 7 grammar-valid syntax-only and 9 deliberately malformed
cases. Optionally include six historical inputs with `--examples-dir`.

## Verification actually performed in this environment

- Python unittest generator suite: **5/5 passed** (smoke, standard, stress,
  deterministic reproduction and negative grammar tests).
- Generated smoke with historical examples: **40 cases** (18 graph, 12 syntax,
  10 expected-rejection). `--check` confirmed byte-for-byte reproduction.
- Headless legacy graph load → exact TSV oracle comparison → bounded spring
  layout: **18/18 smoke, 26/26 standard and 30/30 stress graph fixtures
  passed** with the available ffgraph library and Tcl runtime. Standard and
  stress used a single spring iteration with explicitly increased case-size
  limits; smoke used two iterations.
- Existing Pass 11 legacy-vs-modern *graph semantic adapter* test: **18/18
  corpus graph fixtures passed** when fed through the previously validated
  temporary event parser shim. **This is not a real Bison/Flex parity result.**
- Actual Bison/Flex regeneration and new C++ corpus parser test: **not run**;
  this execution environment lacks Bison and Flex. Run on Ubuntu.
- The full project CMake/CTest/ASan matrix: **not run** here; Tk/Xpm
  development dependencies are missing. The headless layout test above was
  linked with the existing previously built ffgraph core library.

## Ubuntu verification

```sh
python3 -m unittest discover -s tests/graph_corpus -p test_generate.py -v
python3 tests/graph_corpus/generate.py --profile smoke \
  --output-dir build/graph-corpus-smoke \
  --examples-dir tests/graph_corpus/samples
python3 tests/graph_corpus/generate.py --profile smoke \
  --output-dir build/graph-corpus-smoke \
  --examples-dir tests/graph_corpus/samples --check

cmake -S . -B build/pass11-corpus -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON \
  -DFFGRAPH_ENABLE_MODERN_GML=ON \
  -DFFGRAPH_ENABLE_CORPUS_LAYOUT_TESTS=ON
cmake --build build/pass11-corpus --parallel 2
ctest --test-dir build/pass11-corpus --output-on-failure
```

For sanitizers, use a separate build tree and `-DFFGRAPH_SANITIZERS=ON`
(with your existing CI and Xvfb settings). Actual layout performance thresholds
and final coordinate quality are not part of the default regression gate.

For usage, schemas, fixture categories and optional larger runs, see
`tests/graph_corpus/README.md`.

## Future work

- Run the generated-parser CTest and parity tests with real Bison/Flex on
  Ubuntu, including ASan/UBSan.
- Extend the suite with directed semantics when ffgraph import/layout
  behaviour is documented, and add more grammar-directed invalid mutations
  and historical graph examples.
- Separate layout **correctness** from layout **quality** metrics (edge
  crossings, overlap, bounding box, connectivity preservation, convergence).
- Consider property-based mutation and differential fuzzing once stable
  independent semantic oracles exist.

SPQR stays excluded, inspector/WSLg is still parked and existing APIs and
formats are unchanged.
