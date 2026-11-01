# Pass 11 — comprehensive GML differential corpus (incremental checkpoint)

## Scope and baseline

Apply after **Pass 11 metamorphic-layout** (and all preceding Pass 8–11 patches).
This checkpoint leaves the production `GTControl` importer, Tcl/Tk GUI, file
formats, FF/Rome parsers, parked inspector/WSLg issue and excluded SPQR code
unchanged. The modern Bison/Flex parser remains opt-in.

The test builds each graph twice from identical GML bytes:

1. checked-in legacy GTControl scanner/parser via `FFGraph::load`, and
2. newly generated Bison/Flex parser → event stream → experimental
   `ffgraph_gml::applyToGraph` adapter.

The shared, format-independent `tests/graph_corpus/graph_snapshot.h` compares
sorted **multisets**, including duplicate edges, in a representation that
retains graph name, node IDs/labels/XYZ, directed edge endpoints, edge IDs and
labels. It rejects foreign/null edge endpoints and non-finite coordinates.
The generated graph corpus also has an **independent TSV oracle**; both paths
must agree with it, not merely with each other. TSV values are converted to
`FFCoord` to reflect ffgraph's actual coordinate representation (`float` in
this build). Exact numeric equality is used *after* that conversion.

Each case is repeated twice by default, constructing fresh graphs and
regenerating the modern event stream. Failures report the fixture, source
path and first divergent field. A comparator self-test checks that deliberate
changes to graph/node labels, coordinates, edge orientation, edge IDs and
parallel-edge multiplicity are detected.

## Corpus selection

`FFGRAPH_GML_DIFFERENTIAL_PROFILE` is a cache value used only with
`FFGRAPH_ENABLE_MODERN_GML=ON` and `BUILD_TESTING=ON`:

| Profile | Generated graph files | Metamorphic pairs | Other fixtures |
| --- | ---: | ---: | --- |
| smoke (default) | 18 | 55 | 12 syntax-only, 10 expected-rejection |
| standard | 26 | 75 | 12 syntax-only, 10 expected-rejection |
| stress | 30 | 85 | 12 syntax-only, 10 expected-rejection |

The six byte-preserved historical examples are included among the syntax-only
and expected-rejection fixtures (the AGD `options.gml` example is not a graph).
**Syntax-only files are deliberately not passed through FFGraph::load**:
valid attribute documents are not necessarily graphs. Expected-rejection
fixtures are tested against the modern parser, including repeated attempts
to apply rejected results to an existing graph, which must remain unchanged.

Metamorphic pairs are checked as separate graph inputs against each importer.
This test ensures differential parity for original and transformed graphs;
`test_graph_corpus_metamorphic_pass11` remains responsible for asserting the
between-variant topology/coordinate transformation laws and bounded layout
properties. Combining these two tests exercises both properties.

## Files

- `tests/graph_corpus/graph_snapshot.h` — reusable graph and independent TSV
  snapshot/oracle comparison helpers.
- `tests/graph_corpus/test_graph_snapshot_pass11.cpp` — standalone comparator
  regression tests.
- `tests/modern_gml/test_gml_differential_corpus_pass11.cpp` — end-to-end
  corpus and metamorphic comparison driver.
- `CMakeLists.txt` — opt-in profile, generator targets and CTest registration.
- `.github/workflows/gml-parser-check.yml` — extend existing opt-in parity job
  to exercise the new smoke tests in Debug and ASan/UBSan; add a standard
  Release-profile job.

All generated GML, TSV and JSON files go into the **build tree**, never into
source directories or the patch.

## Ubuntu build and test

Install the existing ffgraph development dependencies (including Tcl/Tk,
X11/Xpm) plus Bison 3.8+, Flex 2.6+, Python 3, CMake and Ninja. Then:

```sh
cmake -S . -B build/gml-diff -G Ninja \
  -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug \
  -DFFGRAPH_ENABLE_MODERN_GML=ON \
  -DFFGRAPH_GML_DIFFERENTIAL_PROFILE=smoke
cmake --build build/gml-diff --parallel 2 \
  --target test_gml_differential_corpus_pass11 test_graph_snapshot_pass11
ctest --test-dir build/gml-diff \
  -R '^test_(graph_snapshot|gml_differential_corpus|gml_differential_metamorphic)_pass11$' \
  --output-on-failure
```

For standard and stress profiles, configure **separate build directories**
with `-DFFGRAPH_GML_DIFFERENTIAL_PROFILE=standard` or `stress`. For a full
sanitizer run, use a fresh build directory and add `-DFFGRAPH_SANITIZERS=ON`:

```sh
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
ctest --test-dir build/gml-diff-asan \
  -R '^test_(graph_snapshot|gml_differential_corpus|gml_differential_metamorphic)_pass11$' \
  --output-on-failure
```

The CI workflow is manually dispatched; newly added jobs have **not** been
executed on GitHub during creation of this patch.

## Verification completed in the authoring environment

- Python generator suite: **8/8** passed.
- C++17 strict syntax checks on the new harness and comparator: passed.
- Standalone comparator executable: passed.
- Executable-level **smoke** comparison: 18 graph/oracle fixtures,
  12 syntax-only, 10 negative; 55 metamorphic pairs; **2 repeats** each.
- Executable-level **standard** comparison: 26 graph/oracle fixtures,
  75 metamorphic pairs; **2 repeats** each.
- ASan/UBSan **smoke** comparison: passed with leak detection **disabled**.
- Workflow YAML parsed successfully.
- Full-project CMake configuration: blocked by missing Tk development metadata
  (`pkg-config tk`); Bison/Flex are also unavailable.

**Critical qualification:** The local executable-level checks used the
previously established **headless Pass 9 ffgraph core** and a **temporary
parser shim**. The shim is *not in this patch*. These checks validate the
new harness, ffgraph adapter, graph oracle and legacy import path, but are
**not** runtime verification of the modern generator-produced Bison/Flex
scanner/parser, nor a full Pass 11 sanitizer build. The actual generated-parser
comparisons must be run on Ubuntu or in the opt-in GitHub Actions job.

## Known limits / subsequent work

- Differential equality is stricter than undirected graph isomorphism: edge
  endpoint orientation and numeric IDs are compared directly. Metamorphic
  invariants are verified separately.
- Syntax error rejection must not mutate a graph. **Semantic errors after a
  successful parse may still partially mutate the experimental adapter's
  target**; the adapter is not yet transactional and is not for production use.
- This suite does not compare legacy vs modern diagnostic text or historical
  recovery behaviour on malformed graph structures. The legacy importer does
  not supply a suitable structured error return API. Such parity should be
  assessed separately, without assuming all malformed inputs are safe to
  feed through the historical importer.
- The generated corpus is broad and deterministic, not exhaustive or a
  substitute for fuzz testing. Stress is separate from the default CI gate.
