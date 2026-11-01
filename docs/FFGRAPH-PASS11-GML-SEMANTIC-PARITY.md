# Pass 11 continuation — GML graph semantic parity

**Base:** Pass 11 GML pilot, scanner double-free correction, then Bison nested-list
ownership correction. This patch applies **after all three**. It is incremental;
no checked-in generated parser/scanner has been replaced.

## Goal and implemented boundary

The modern Bison/Flex parser currently produces an ordered stream of
`Begin`, `End`, `Integer`, `Real`, and `String` events. Previously, tests only
verified that stream. The legacy `GTParser` also constructs actual
`FFGraphClass` nodes and edges. To check those semantics, this checkpoint adds:

- `fgraph/modern/gml_graph_adapter.{h,cc}`: an **experimental** event replay
  adapter for an existing `FFGraphClass`. It mirrors legacy `GTParser.y`'s
  *non-scoped* `current_node` and `current_edge`, the integer ID lookup map,
  node and edge labels, x/y/z coordinate updates, edge source/target
  reassignment, and parallel edges and loops. The adapter is **not linked to
  the application importer**.
- `tests/modern_gml/test_gml_graph_parity_pass11.cpp`: differential test that
  reads each real GML fixture through legacy `FFGraphClass::load(..., YES)`
  and through modern `parse()` + the replay adapter. It compares normalized
  graph names, node IDs/names/coordinates, and multiset-like edge tuples
  (source ID, target ID, edge value, name). Sorting ensures traversal-order
  differences do not cause spurious failures, but duplicate edges are retained.
- Root CMake: registers `test_gml_graph_parity_pass11` **only** with
  `FFGRAPH_ENABLE_MODERN_GML=ON`. The default build and default importer are
  unchanged.
- Manually dispatched `.github/workflows/gml-parser-check.yml`: adds separate
  Ubuntu Debug and ASan/UBSan jobs that build the full legacy graph libraries
  together with the modern generated parser and compare graph outputs.

The corpus includes explicit node/edge IDs, negative and real coordinates,
unknown attributes, mixed `{}` / `[]` lists, graph and node/edge names,
escaped quotes, repeated imports, loops, parallel edges, an empty graph,
and legacy unknown-byte skipping. Invalid/truncated input is rejected by the
modern parser and the adapter must not mutate a graph for a **syntax failure**.

## Run on Ubuntu with both parser generators and GUI build dependencies

```sh
sudo apt-get update
sudo apt-get install --yes --no-install-recommends \
  build-essential cmake ninja-build pkg-config bison flex \
  tcl-dev tk-dev libx11-dev libxpm-dev

# Existing isolated pilot should still pass.
cmake -S tests/modern_gml -B build/modern-gml -G Ninja -DBUILD_TESTING=ON
cmake --build build/modern-gml --parallel 2
ctest --test-dir build/modern-gml --output-on-failure

# New: actual graph-output parity through both parser implementations.
cmake -S . -B build/modern-gml-parity -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  -DFFGRAPH_ENABLE_MODERN_GML=ON
cmake --build build/modern-gml-parity \
  --target test_gml_graph_parity_pass11 --parallel 2
ctest --test-dir build/modern-gml-parity \
  -R '^test_gml_graph_parity_pass11$' --output-on-failure -V

# Sanitizer run with separately generated and instrumented objects.
cmake -S . -B build/modern-gml-parity-asan -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  -DFFGRAPH_ENABLE_MODERN_GML=ON -DFFGRAPH_SANITIZERS=ON
cmake --build build/modern-gml-parity-asan \
  --target test_gml_graph_parity_pass11 --parallel 2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
ctest --test-dir build/modern-gml-parity-asan \
  -R '^test_gml_graph_parity_pass11$' --output-on-failure -V
```

## Verification completed in the patch-authoring environment

- The replay adapter and differential test compile as C++17 with
  `-Wall -Wextra -Werror -fsyntax-only`, using the **actual** ffgraph and
  previously supplied Tcl headers.
- The five differential fixtures passed a local executable linked to the
  **real prebuilt ffgraph headless graph libraries** (25 comparisons:
  five cases repeated five times) and the new replay adapter. For this
  local check ONLY, the GML event parser was a temporary hand-written test
  shim, not generated Bison/Flex output. The shim is **not in the patch**.
  This verifies the replay adapter against legacy graph semantics on the
  fixtures; it does **not** establish that the real modern scanner emits
  the same events for all cases.
- An additional local ASan/UBSan run with leak detection passed using the
  corrected Pass 10 `FFGraphClass` and `GTScanner` objects, instrumented
  differential test and adapter, and the remaining prebuilt headless library
  objects. This is **partial instrumentation**; it is not equivalent to a
  fresh all-sources sanitizer build. The unmodified Pass 9 scanner objects
  alone exhibit their known pre-Pass-10 GML leaks and must not be used as
  evidence about this new adapter.
- The actual Bison/Flex graph-parity executable, complete full CMake/CTest,
  and full sanitizer CI were **not** run here: this host still lacks Bison,
  Flex, Tk/Xpm development dependencies and network package installation.
  Normal and sanitizer standalone modern-parser tests were reported passing
  by the user on their Ubuntu host after the earlier ownership fixes.

## Limitations and next gate

The replay adapter is test-only and deliberately mimics the old parser's
single-current-node/edge cursor semantics (including their quirks). It is **not
transactional** for semantic errors (e.g. a syntactically valid edge with an
unknown target): such an error may leave a partially constructed graph. It
rejects a parse that returned a syntax error before any graph mutation.
Error-message equivalence, malformed-file behaviour in the legacy parser,
forward references, duplicate IDs, integer overflow, huge nesting, and
arbitrary unknown attributes still need broader differential coverage.

**Do not enable modern GML for production imports yet.** Once the actual
Bison/Flex graph-output parity and sanitizer tests pass on Ubuntu, expand
the corpus using real historical `.gml` files before selecting the modern
importer with a separate explicit runtime opt-in. After that, migrate
FF/sgraph and Rome independently. Tcl/Tk GUI, file formats, inspector/WSLg,
and excluded SPQR remain untouched.
