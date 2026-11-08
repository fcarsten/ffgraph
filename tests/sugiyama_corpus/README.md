# Sugiyama graph corpus

Two default headless CTests exercise 31 graphs under six configurations, each
with a reference and an optimized layout on the **same graph objects**:

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug -R '^test_sugiyama_corpus_' --output-on-failure
ctest --preset debug -R '^test_sugiyama_corpus_' -V
```

The verbose run prints JSON metrics per graph/configuration: real node/edge
counts, dummy-expanded layer count/width, initial/final adjacent-layer crossings,
verified permutation calls and checked matrices. Release and strict sanitizer
presets run these tests too:

```sh
ctest --preset release -R '^test_sugiyama_corpus_'
LSAN_OPTIONS= ctest --preset asan-headless -R '^test_sugiyama_corpus_'
```

## Inputs

`real/` contains 13 unmodified copies from `../graphs`, `../demographs` and
`contrib/out.g`. `cases.tsv` records pre-test node/edge counts. `provenance.json`
records original paths, byte sizes and SHA-256 hashes. Neither external directory
is needed to build/run the tests; these copies are intended for ordinary CI.
The selection includes `b4_alledges_2d.g`, dataflow, hierarchy, cluster, a larger
tree, planar, 3D-model and GML examples. Original filenames/graph labels remain
inside their copied contents; a few fixture filenames are shortened.

The 18 synthetic cases are constructed explicitly in
`../test_sugiyama_corpus.cpp`: empty/singleton/isolated graphs; self-loops; a deep
chain with single-/two-dummy long edges; wide fans in either direction; alternating
layer widths; reversed matching; ties/zero slots; dense bipartite; parallel,
antiparallel and cyclic edges; strong components; disconnected components; three
seeded layered graphs; and duplicate/negative/sparse working IDs. Node sizes and
edge names/weights vary. Seeds and construction rules are fixed.

## Configurations and assertions

| Variant | Dimension | Orientation | Grid | Size mode | Additional setting |
| --- | --- | --- | --- | --- | --- |
| 0 | 2D | Top down | On | Uniform | Defaults |
| 1 | 3D | Top down | On | Uniform | Dimension control |
| 2 | 2D | Left to right | Off | Individual | Offsets 31,47 |
| 3 | 2D | Top down | Off | Individual | Start-node flag off |
| 4 | 3D | Left to right | On | Individual | Leaves down |
| 5 | 2D | Top down | On | Individual | No crossing reduction |

The test-only reduction object compiles the production source with verification
enabled. Every optimized `b_r`/`b_c` call is checked against the frozen bubble
sorter on copied incoming matrices, including partner metadata/padding. A second
whole-layout run uses the frozen sorter with the otherwise current algorithm.
Original node/edge IDs and node coordinates are restored between runs; global
Sugiyama straightens and rebuilds edge routes itself.

Assert exact node coordinates/edge routes, finite coordinates, object identity,
incidence, names/weights, node sizes/colors, directedness and start/end nodes.
Calculate crossings with an independent edge-inversion oracle and require equal
reference/optimized totals with no increase from reduction. Independent anchors:
reversed matching has 66 initial crossings and zero after reduction; K24,24 has
76,176 crossings under every ordering. The no-reduction variant must keep its
crossing total and perform no barycenter permutation calls.

This tests adjacent-layer crossing counts after dummy expansion and removal of
loops/parallel edges. It is not a geometric intersection count for arbitrary
routed segments. Real-graph totals can vary with legacy allocation/traversal
order across processes, so each run compares the same initialized graph objects
instead of freezing process-dependent drawings. The original eight committed
full-layout oracles still run separately and remain unchanged.

Some real files carry `FFSubGraphLabel` payloads. The default/headless registry
does not enable that label; its existing diagnostics remain visible. These tests
exercise the resulting flat graphs, not nested external subgraph loading.
No SPQR implementation or new GUI layout option is enabled.

See [the checkpoint](../../docs/FFGRAPH-SUGIYAMA-CORPUS-EXPANSION.md) for actual
verification results and limits.
