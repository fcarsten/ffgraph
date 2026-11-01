# ffgraph generated graph and GML regression corpus

This deterministic, standard-library-only Python generator derives its **valid
attribute syntax** from the experimental `fgraph/modern/modern_gml.y` and
`modern_gml.l` token grammar. It deliberately separates **grammar validity**
from **graph semantic validity** and from **layout quality**.

## Generate a corpus (no compiled dependencies)

Run from the repository root:

```sh
python3 tests/graph_corpus/generate.py --output-dir build/corpus-smoke \
  --profile smoke --seed 20261101 \
  --examples-dir tests/graph_corpus/samples
python3 tests/graph_corpus/generate.py --output-dir build/corpus-smoke \
  --profile smoke --seed 20261101 \
  --examples-dir tests/graph_corpus/samples --check
python3 -m unittest discover -s tests/graph_corpus -p test_generate.py -v
```

Use `--force` to replace a generated directory intentionally. To generate the
larger profiles, choose `standard` or `stress` and a different output directory.
The random families and initial coordinates are derived from SHA-256 of the
specified seed **and case name**: adding cases will not perturb earlier ones.

## Profiles

| Profile | Generated graph cases | Nodes / edges total | Purpose |
|---|---:|---:|---|
| smoke | 18 | 134 / 152 | Fast CI, ownership, loops, multiedges, singleton, empty, coordinates |
| standard | 26 | 443 / 743 | Grids, larger trees, dense/sparse random, articulation, disconnected graphs |
| stress | 30 | 2,634 / 4,060 | 512-node paths, 400-node grids, 1,023-node trees, random graphs |

All profiles additionally contain seven grammar-valid, syntax-only cases and
nine expected-rejection cases. When `--examples-dir` is supplied, five
historical GML inputs are copied as **syntax-valid** cases and `options.gml`
(AGD options, not a graph) is copied as an **expected-rejection** case. The
source examples and original bytes remain in `samples/`. No user data is
regenerated or overwritten.

## Outputs and test contract

- `graph/*.gml`: semantically valid generated graphs, safe for importer and layout
  tests, with stable IDs and coordinates.
- `syntax/*.gml`: only known to satisfy the experimental attribute grammar;
  **do not** try to load or lay them out as graphs.
- `invalid/*.gml`: intentionally malformed with respect to the pilot grammar;
  parser must reject them. They are **not** a security fuzz corpus.
- `manifest.json`: schema version, seed, SHA-256 per fixture, node/edge arrays,
  initial XY coordinates, graph family, connected components, degree sequence,
  isolated nodes, self-loops, multiedge groups and other topology metadata.
- `oracle/*.tsv`: plain tab-separated golden graph snapshots (name, node ID,
  labels, initial positions, edge ID and ordered endpoints), for graph-import
  assertions in C++, Python or other layout frameworks.
- `cases.tsv`: small stable five-column input to C++ CTest drivers.

The oracle describes the **input**, not expected final node positions: spring
layout is nondeterministic and differs by implementation/hardware. The bounded
spring-layout test checks import topology/labels/coordinates **before** layout,
then checks layout completion, finite coordinates and valid edge routing.
Future algorithms may use the corpus to compare crossings, edge-length
variance, layout runtime, component separation and node overlap, but should
specify their own thresholds instead of treating the original coordinates as
layout goldens.

## Build and run integrations

Modern pilot parser **without** the GUI:

```sh
cmake -S tests/modern_gml -B build/modern-gml -G Ninja -DBUILD_TESTING=ON
cmake --build build/modern-gml --parallel 2
ctest --test-dir build/modern-gml --output-on-failure
```

Full ffgraph with modern GML and optional spring tests (requires Tcl/Tk,
X11, Xpm development packages, Bison and Flex):

```sh
cmake -S . -B build/corpus -G Ninja -DBUILD_TESTING=ON \
  -DFFGRAPH_ENABLE_CORPUS_LAYOUT_TESTS=ON
cmake --build build/corpus --target test_graph_corpus_layout_pass11 \
  test_gml_graph_regression_pass11 test_gml_corpus_pass11 --parallel 2
ctest --test-dir build/corpus -R 'graph_corpus|gml_corpus|generated_graph_regression' \
  --output-on-failure
```

For explicitly selected standard/stress layout runs, use the executable with
`CORPUS_DIR MAX_NODES MAX_EDGES ITERATIONS`. Default is 64 nodes, 256 edges,
and 2 iterations; increase those limits cautiously and separately from CI.

## Limits

- This is **systematic coverage**, not exhaustive enumeration of all graphs or
  GML documents.
- Graphs are generated with undirected topology and may contain loops and
  parallel edges. Input GML `directed 0` is intentional.
- `options.gml` is an AGD configuration document, not a graph. It is retained
  as a grammar rejection fixture, not imported as a graph.
- The Bison/Flex GML importer is the only production GML backend; old GT
  parser sources can be retrieved from the pre-retirement Git revision.
- The independent graph TSV oracles are permanent regression references.
  Parser-vs-parser differential checks ended with legacy GT retirement.
- Random seeds and graph families are defined in `generate.py`; retain the
  version and seed when comparing results across revisions.

## Metamorphic import/layout corpus (Pass 11 follow-up)

The separate `metamorphic.py` generator creates paired GML inputs from curated
smoke/standard/stress graph families. Its five transformations are node ID
bijection, reversed node/edge declaration order, reversed undirected edge
endpoints, XY translation (`+64.125, -32.750`), and uniform XY scaling (`2x`).

```sh
python3 tests/graph_corpus/metamorphic.py \
  --output-dir build/metamorphic-smoke --profile smoke --force
python3 tests/graph_corpus/metamorphic.py \
  --output-dir build/metamorphic-smoke --profile smoke --check
python3 -m unittest discover -s tests/graph_corpus -p 'test_*.py' -v
```

Outputs `base/*.gml`, `variant/*.gml`, `cases.tsv`, and `manifest.json` are
reproducible and disposable. The smoke profile contains 11 base graphs and 55
transformation pairs; standard and stress add larger families. The input files
use a deliberately conservative grammar subset (integer IDs, quoted labels,
REAL XY and nested graphics). `--force` has the same guarded replacement
rules as the existing corpus generator.

With `FFGRAPH_ENABLE_CORPUS_LAYOUT_TESTS=ON`, CMake additionally builds and
runs `test_graph_corpus_metamorphic_pass11`, which loads each pair through the
production importer and executes bounded spring layouts for both inputs.
It verifies labelled undirected multigraph structure and initial coordinate
relations **before** layout, then ensures finite node positions, valid edge
routes and preserved labelled graph topology **after** layout. It does **not**
assert exact final coordinates, strict XY equivariance or numeric node/edge ID
stability: the historical spring layout explicitly renumbers `value` fields.
Input graph files are undirected; reverse-edge metamorphism is not applicable
to directed GML semantics. No production parser or layout code is modified.
