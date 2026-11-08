# Frozen Sugiyama layout oracles

These eight snapshots were recorded before the crossing-counter optimization,
using the unmodified `flayout/FFreduce_crossing.cc` from commit `ae45633`.
Its SHA256 was `82aaa26180029db3ccbee77eeee722b86fcec9e6f42d3adcc8523f1756ca1a25`.
The new test harness and fixtures were compiled against that old implementation
and `build/debug/test_sugiyama_layout --record tests/sugiyama_oracles` was run
before replacing the counter. A second run checked the recorded data unchanged.

The four programmatic fixtures in `tests/test_sugiyama_layout.cpp` cover long
edges/dummies, complete bipartite layers, cycles/loops/parallel edges/an isolated
node, and a star, with explicit contiguous node/edge IDs and labelled, weighted
edges. Each is captured with dimension parameter 2 and 3. The existing algorithm
produces the same output for these two settings; this does not establish a new
3D layout capability.

Snapshots retain graph name/directedness, original node IDs and labels, node
XYZ/W, edge endpoints/names/weights and every route point. Temporary numeric IDs
used as layout scratch space are excluded. Permanent comparisons allow a small
float-rounding tolerance; identity/attributes/routes remain checked.

Do not regenerate these expectations with the optimized counter. The recorder
refuses an existing destination directory. Any deliberate oracle update needs
its own independent baseline and explanation. These files contain generated
fixtures, not the private external graph collection.

| File | SHA256 |
| --- | --- |
| `bipartite-2.layout` | `2c5538b2444fd293c4a62c229ad52485a7d0486934b0da4bd0970fff97df3941` |
| `bipartite-3.layout` | `2c5538b2444fd293c4a62c229ad52485a7d0486934b0da4bd0970fff97df3941` |
| `cycle-parallel-isolated-2.layout` | `0d953333e0b4167fdd4f58fc2d0c50183755dbfc71e1e29d858523f841f8f201` |
| `cycle-parallel-isolated-3.layout` | `0d953333e0b4167fdd4f58fc2d0c50183755dbfc71e1e29d858523f841f8f201` |
| `long-edges-2.layout` | `c74f614cbb7de922055a77660c056a0f9a57485f349448b872d5831ebfc7e02f` |
| `long-edges-3.layout` | `c74f614cbb7de922055a77660c056a0f9a57485f349448b872d5831ebfc7e02f` |
| `star-2.layout` | `45f8122e7df864eb329b07dcef4032e6e096af2fc04ae24e55ed5db0928031f0` |
| `star-3.layout` | `45f8122e7df864eb329b07dcef4032e6e096af2fc04ae24e55ed5db0928031f0` |
