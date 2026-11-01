# Isolated modern Rome pilot

The application still uses legacy Rome. Enable the separate pilot from the
repository root with `-DFFGRAPH_ENABLE_MODERN_ROME_PILOT=ON`; see BUILD-CMAKE.md.
No production backend selector, fallback or public API change is introduced.

Tests registered with the pilot enabled:

| CTest | Coverage |
|---|---|
| test_rome_modern_pass12 | Generated grammar/scanner, integer limits, locations, malformed input/recovery, NUL rejection, long lines, 20,000 rows and concurrent calls |
| test_rome_compatibility_pass12 | Eight accepted fixtures × graphics/bare × fresh/merge × legacy stream/string/public file loading × LF/CRLF/no final LF = 288 comparisons, plus duplicate lookup, ID-changing callback and existing-node reference cases |
| test_rome_generated_oracle_pass12 | 32 deterministic graph structures with/without graphics, each through three legacy entry points; 192 differential comparisons and independent source-graph checks |
| test_rome_modern_failures_pass12 | Legacy partial imports versus modern unchanged destination, rejected-text cleanup, range/NUL rejection and demonstrated legacy long-line truncation |

The existing `test_rome_baseline_pass12` remains registered independently of the
option. All checks stay active in Release and all pilot tests are headless.

`fixtures` adds integer boundaries, lexical quirks, negative ID -1, extra fields,
indented comments, REAL rejection and out-of-range IDs/metadata. The pilot also
uses the eight earlier `tests/rome_fixtures` inputs. These and the generated
graphs are synthetic; no external Rome benchmark collection was available in
the checkout. Overflow/NUL cases are not treated as valid legacy oracles.

Snapshots include ID/name multisets, directedness, start/end nodes, adjacency
counts, endpoint membership, edge direction/multiplicity/weights, custom label
payloads, graphic coordinates/ranges/colors/icons/masks/shapes and edge routes.
Allocation-dependent lookup among duplicate IDs is checked against each graph's
live `nodeWithValue()` result. In the -1 merge-sentinel collision case, per-graph
pointer/seed invariants and independent ID/edge multisets replace a comparison
of physical duplicate identity/geometry. No matrix case is skipped.

The generator constructs source FFGraphs through the graph API and serializes
only Rome-format IDs/endpoints plus ignored metadata. Expected graphs come from
those source objects and hand-written fixture expectations, not parser output.
This catches matching mistakes in the two importers. It does not independently
reimplement the graph/container library.

Standalone build (parser test only; no Tcl/Tk/X11):

```sh
cmake -S tests/modern_rome -B build/modern-rome -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/modern-rome --parallel 2
ctest --test-dir build/modern-rome --output-on-failure

cmake -S tests/modern_rome -B build/modern-rome-asan -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DFFGRAPH_MODERN_ROME_SANITIZERS=ON
cmake --build build/modern-rome-asan --parallel 2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
ctest --test-dir build/modern-rome-asan --output-on-failure
```

GNU Bison 3.8+, Flex 2.6+, CMake, C++17 and a threads-capable C++ runtime are
required. Sanitizer commands use GCC/Clang. Generated output lives in the build
directory. See `docs/FFGRAPH-PASS12-ROME-PILOT-CHECKPOINT.md` for results, semantic
differences and the remaining integration gate.
