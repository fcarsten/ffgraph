# Modern Rome production parser and regression tests

GNU Bison/Flex Rome is the sole production implementation. It is generated in
all CMake builds, including BUILD_TESTING=OFF, and used by normal GUI/file loads.
There is no pilot flag, selectable legacy backend or automatic fallback.

The annotated local tag `ffgraph-pre-legacy-rome-removal` preserves the last
legacy sources and live differential harness at `5db5a38`. The sixteen frozen
snapshots in `oracles` were captured from that actual legacy implementation
before deletion; builds/tests cannot regenerate them from the current parser.

| CTest | Coverage |
|---|---|
| test_rome_modern_pass12 | Generated parser/scanner, boundaries, locations, recovery, NULs, long lines, 20,000 rows and concurrent calls |
| test_rome_graph_regression_pass12 | 192 fixture combinations: eight inputs × graphics/bare × fresh/merge × direct/production × LF/CRLF/no final LF; frozen legacy graphs, independent topology, duplicate lookup and callbacks |
| test_rome_generated_oracle_pass12 | 32 source-graph structures with/without graphics, each through direct replay and production loading; 128 API/oracle comparisons |
| test_rome_modern_failures_pass12 | Atomic syntax/range/NUL rejection, correct locations, long-line consumption and balanced production lifecycle events |
| test_rome_corpus_smoke_pass12 / test_rome_corpus_matrix_smoke_pass12 | Independent numeric-row oracle and accepted/empty/malformed external-style files without a private dataset |
| test_rome_production_backend_pass12 | Public dispatch, newline variants, warnings, merge identity, unreadable-file rejection and recovery |
| test_rome_window_oracle_pass12 | Real 2D/3D windows, update suppression/restoration, topology, rejection and merge recovery |
| test_rome_gui_file_workflow_2d_pass12 / test_rome_gui_file_workflow_3d_pass12 | Real Tcl File > Load/Merge/Close/reopen with a stubbed file picker |

The last three rows are registered by root CMake; GUI cases require Xvfb.
All test checks remain active under NDEBUG/Release.

Graph snapshots retain IDs/names, directedness, start/end nodes, adjacency,
endpoint membership, edge direction/multiplicity/weights, custom payloads,
coordinates/ranges/colors/icons/masks/shapes and edge routes. Independent
hand-written expectations and source graphs complement frozen historical data.
Direct replay and production loading use the same parser: their comparisons
are API consistency tests, not independent-parser differential tests.

Duplicate IDs retain live nodeWithValue lookup semantics. Allocation-dependent
physical duplicate selection is checked within each graph; reset-to--1 collisions
use pointer/seed invariants and ID/edge multisets instead of cross-graph physical
identity. No fixture matrix case is skipped.

## External collection

The supplied collection remains outside Git. Configure a machine-local path to
register both optional headless tests, each with a 30-minute timeout:

```sh
cmake --preset release -DFFGRAPH_ROME_CORPUS_DIR=/home/carsten/work/demographs/rome
cmake --build --preset release --parallel 2
ctest --preset release -R '^test_rome_external_' --output-on-failure
```

The driver is `test_rome_graph_regression_pass12`:

```sh
build/release/tests/modern_rome/test_rome_graph_regression_pass12 \
  --corpus /home/carsten/work/demographs/rome
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
build/asan/tests/modern_rome/test_rome_graph_regression_pass12 \
  --corpus-matrix /home/carsten/work/demographs/rome
```

Full mode checks every regular file through the production filename loader
using temporary .rome copies of original bytes. Matrix mode picks sparsest and
densest accepted graphs per top-level family/node-count decade, plus all rejected
files, then exercises graphics/bare, fresh/merge and direct/production entry
points: eight combinations per file (880 for the supplied 110-file selection).
No extension filter, duplicate-file removal or rejection blacklist is used.

A strict independent row reader models ordinary two/four-integer rows, blank
lines and column-one comments. It requires unique node IDs, checks topology,
adjacency, weights and snapshots, and models malformed/range inputs as rejection.
Both API routes must preserve graph state on failure. Permissive lexical quirks
and duplicate-ID behavior retain dedicated synthetic coverage. Set the optional
cache path to an empty value to remove external-data registration from that build.
Ordinary CI still exercises committed fixtures and frozen oracles.

Historical live comparisons and collection details are in
`docs/FFGRAPH-PASS12-ROME-REAL-CORPUS.md`; production retirement/GUI results are in
`docs/FFGRAPH-PASS12-ROME-LEGACY-RETIREMENT.md`.

## Standalone parser-only build

No Tcl/Tk/X11 or graph libraries are required:

```sh
cmake -S tests/modern_rome -B build/modern-rome -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/modern-rome --parallel 2
ctest --test-dir build/modern-rome --output-on-failure
```

For standalone sanitizer testing, use
`-DFFGRAPH_MODERN_ROME_SANITIZERS=ON` and explicit ASAN_OPTIONS/UBSAN_OPTIONS as
above. GNU Bison 3.8+, Flex 2.6+, CMake, C++17 and thread support are required.
Generated sources remain build-local; the production parser archive is installed
alongside FFGraph. See BUILD-CMAKE.md for normal application builds.
