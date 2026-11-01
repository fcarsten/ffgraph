# Pass 11 follow-up — GML semantic-atomicity preflight

## Scope and compatibility baseline

Incremental after `ffgraph-pass11-comprehensive-gml-differential.patch` (and
all preceding Pass 8–11 patches). The experimental
`ffgraph_gml::applyToGraph(const Result&, FFGraphClass&)` previously modified
its destination while processing events and could return a semantic error
*after* adding nodes/edges or changing its name. The production `GTControl`
GML importer is **not** switched or modified by this patch.

## Reproduced failure

The new regression test failed on the preceding baseline with:

```
Pass 11 semantic atomicity: unknown target after other changes: changed graph on failure
```

An input that creates a node and an edge, but ends with an unknown edge target,
was leaving both new objects (and earlier attributes) on the caller's graph.
This could corrupt a caller-owned graph after a reported import failure.

## Changes

1. Add a non-mutating semantic preflight in `fgraph/modern/gml_graph_adapter.cc`.
   It mirrors the original cursor and ID-resolution rules using stable
   in-memory shadow nodes/edges, including default node ID `-1`, lookups in
   the existing destination graph, ID reassignment aliases, repeated edges,
   and the legacy cursor's intentional lack of nesting.
2. Only after the entire event stream passes semantic validation do we replay
   the already-established adapter behaviour into the real destination graph.
   Thus a **returned syntax or semantic rejection** does not mutate that
   graph, while accepted GML retains historical semantics. There is no graph
   replacement or pointer invalidation on rejection.
3. Add `test_gml_semantic_atomicity_pass11` and register it with the main
   opt-in CMake/CTest build and the existing manually triggered GML GitHub
   Actions workflow, in both normal and ASan/UBSan configurations.
4. Update the experimental adapter's API comment to document its actual
   guarantees and limitations.

### Guarantees and exclusions

- A returned semantic failure (unknown source/target, edge without preceding
  node) or unsuccessful syntax parse leaves an existing destination unchanged.
  No graph callbacks are triggered during semantic preflight.
- Successfully parsed, semantically accepted input retains the existing
  append/cursor behaviour (including unusual repeated node-ID aliases).
- This **does not** provide rollback on allocation failures, exceptions thrown
  during graph callbacks, or concurrent graph modification between preflight
  and commit. It is not a general strong exception guarantee. A true
  exception-atomic import would require a separately designed graph
  transaction/ownership abstraction compatible with the Tcl/Tk object model.
- This patch does not change Bison/Flex grammars, generated files, Tcl/Tk
  integration, graph formats, layout algorithms, the parked inspector/WSLg
  issue, or excluded SPQR code.

## Regression coverage

`test_gml_semantic_atomicity_pass11.cpp` constructs real `FFGraph` objects
with graphical labels, pre-existing named nodes/edges and a start node. It
checks 15 consecutive failed imports per case while asserting graph snapshots,
counts and node identities remain unchanged. The cases are:

- Unknown target after earlier name, node and edge changes.
- Unknown source after adding a node.
- Edge before first node after changing the name.
- An invalid *second* edge after a valid first edge.
- A stale ID alias followed by a later unknown target.
- Failed syntax parse with a nonempty partial event stream.

It also verifies successful appending to an existing graph, resolution of the
legacy ID-reassignment alias and a successful empty event stream. The test
does not require scanner-generated input, so the failure modes are exercised
directly even if Bison/Flex are unavailable.

## Local verification on the authoring host

- **Before fix:** The new test compiled against the preceding adapter but
  failed on partial mutation, as expected.
- **After fix:** Six rejection cases (15 repetitions each), valid append,
  reassigned-ID alias, and empty stream passed in an executable using the
  real headless FFGraph library.
- **ASan/UBSan:** The same regression passed with the existing instrumented
  headless core and `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1`.
- **Differential comparison:** Smoke: 18 graph/oracle fixtures and 55
  metamorphic pairs (two repetitions); standard: 26 graphs and 75 pairs (two
  repetitions); stress: 30 graphs and 85 pairs (one repetition). All passed
  against the available real graph library, **but** used a temporary local
  event-parser shim; this is not generated Bison/Flex verification.
- **Full project CMake/CTest:** Not verified here: the authoring host still
  lacks the required Tk/Xpm development packages and Bison/Flex. The
  executable-level checks are not substitutes for the Ubuntu full build.
- The shipped patch contains no temporary event shim, generated sources,
  binaries or generated corpus.

## Ubuntu verification

Apply this patch after the comprehensive differential checkpoint. For an
existing Debug build with `FFGRAPH_ENABLE_MODERN_GML=ON`:

```bash
cmake --build build/gml-diff --parallel 2 \
  --target test_gml_semantic_atomicity_pass11 \
           test_gml_differential_corpus_pass11

ctest --test-dir build/gml-diff --output-on-failure \
  -R '^test_(gml_semantic_atomicity|gml_differential_corpus|gml_differential_metamorphic)_pass11$'
```

For a fresh instrumented build (use the **top-level** CMake sanitizer option):

```bash
cmake -S . -B build/gml-atomicity-asan -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  -DFFGRAPH_ENABLE_MODERN_GML=ON \
  -DFFGRAPH_SANITIZERS=ON
cmake --build build/gml-atomicity-asan --parallel 2 \
  --target test_gml_semantic_atomicity_pass11
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
ctest --test-dir build/gml-atomicity-asan \
  -R '^test_gml_semantic_atomicity_pass11$' --output-on-failure
```

The manually triggered `Modern GML generator pilot` GitHub Actions workflow
also includes the new test in its graph-output-parity job.
