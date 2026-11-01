# User-provided historical GML examples

These examples were supplied for the ffgraph regression suite; the generator
copies them **byte-for-byte** with `--examples-dir tests/graph_corpus/samples`.
The original line endings and any non-UTF-8 bytes are preserved. They are not
auto-generated and are deliberately **syntax cases only**, not reference
layouts or semantic graph oracles.

`nase.gml`, `nase2.gml`, `nase3.gml`, `fuenfer.gml`, and `10dm.gml` are
valid attribute documents under the modern pilot's grammar.
`nase2.gml` is a large realistic parser stress case (1,096 nodes and 1,691
edges). `options.gml` is an AGD options document and does **not** conform to
the modern pilot's key/value grammar; it is an expected-rejection fixture,
**not** proof that every historical AGD options variant is invalid.

These examples are preserved as historical compatibility data. Additional
real-world files should be classified and reviewed before being added.
