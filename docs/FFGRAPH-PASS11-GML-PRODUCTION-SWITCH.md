# Pass 11 — modern GML production backend (default)

## Scope

Incremental after `ffgraph-pass11-gml-semantic-atomicity.patch`, and all earlier
Pass 8–11 patches. The GNU Bison/Flex parser and event-to-graph adapter are now
selected in `FFGraphClass::load()` for `.gml` files **by default in CMake**.
The public `FFGraphClass::load()` signature and other format dispatch remain
unchanged. The Tcl/Tk GUI, Rome and FF parsers, and graph serializers are not
changed. The inspector/WSLg issue remains parked; SPQR remains excluded.

## Backend selection and dependencies

- `FFGRAPH_GML_BACKEND=modern` (new default): use Bison/Flex event parsing and
  semantic preflight/commit. Requires Bison 3.8+, Flex 2.6+ at CMake build time;
  generated files stay in the build tree.
- `FFGRAPH_GML_BACKEND=legacy`: use the existing `GTControl` parser at runtime.
  This remains a short-term fallback for diagnostic comparisons.
- `FFGRAPH_ENABLE_MODERN_GML=ON` (default): build the modern parser and its
  differential test targets, whether or not it is the selected backend.
- If generators are unavailable, use `-DFFGRAPH_GML_BACKEND=legacy
  -DFFGRAPH_ENABLE_MODERN_GML=OFF`.
- `BUILD_TESTING=OFF` with the modern backend does **not** compile the old GT
  scanner/parser objects. When tests are enabled, they remain compiled so that
  the real legacy-vs-modern differential suite can run.
- The historical Makefiles remain unchanged and retain the legacy GML importer;
  the default switch applies to **CMake** builds only.

For a clean, normal CMake build:

```sh
sudo apt-get install build-essential cmake ninja-build pkg-config bison flex \
  tcl-dev tk-dev libx11-dev libxpm-dev
cmake -S . -B build/gml-modern-default -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build/gml-modern-default --parallel 2
ctest --test-dir build/gml-modern-default --output-on-failure
```

To keep parity tests while selecting the old runtime importer:

```sh
cmake -S . -B build/gml-legacy-fallback -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  -DFFGRAPH_GML_BACKEND=legacy -DFFGRAPH_ENABLE_MODERN_GML=ON
cmake --build build/gml-legacy-fallback --parallel 2 \
  --target test_gml_production_backend_pass11 \
           test_gml_differential_corpus_pass11 test_graph_io_pass10
ctest --test-dir build/gml-legacy-fallback --output-on-failure \
  -R '^test_(gml_production_backend_pass11|gml_differential_corpus_pass11|graph_io_pass10)$'
```

**Existing CMake build directories:** the prior configuration cached
`FFGRAPH_ENABLE_MODERN_GML=OFF` by default. Reconfigure explicitly with
`-DFFGRAPH_ENABLE_MODERN_GML=ON -DFFGRAPH_GML_BACKEND=modern`, or use a fresh
build directory. Reusing a cached `OFF` with the modern backend produces an
intentional configure error instead of silently falling back.

## Import contract and remaining limits

For GML files, `load()` now reads bytes from the opened stream, parses to an
ordered event stream, validates graph references, and only then commits to the
same graph object. It reports invalid input through the existing `ffError`
interface. Existing node ID resets (historically part of `load()`) happen
**only after successful syntax and semantic validation**. Rejected input thus
preserves node IDs, counts and pointer identities. Successful files preserve
historical ID reset and graph-appending behaviour.

This is not a strong exception guarantee: resource exhaustion or exceptions
from GUI/graph callbacks *during commit* are not rolled back. Also, the graph
start/end load events still fire even if parsing fails. The existing extension
matching and UI event handling are unchanged.

The two Bison/Flex-vs-GT differential test drivers now instantiate
`GTControl` **explicitly**, rather than using `FFGraphClass::load()` as their
reference path. Otherwise, switching the default would accidentally compare
the modern parser against itself and give a false parity pass.

## Regression checks added

`test_gml_production_backend_pass11` invokes `FFGraphClass::load()` with real
`.gml` files. Both backend choices test repeated valid imports, pre-existing
node identity and ID reset, node/edge counts, parallel edges and self-loops.
In modern mode only, it also repeatedly feeds syntax-invalid GML and a
semantically invalid edge reference into a nonempty graph, verifying no
partial mutation. Tests are automatically registered by CTest when the modern
GML parser target is enabled. The CI Debug and ASan jobs now install Bison/Flex;
a dedicated CI job exercises the `legacy` fallback.

## Verification performed on the patch-authoring host

- **Passed:** strict C++17 syntax checking for the changed graph source,
  adapter, new production test, and both corrected differential drivers.
- **Passed:** actual modern-selected `FFGraphClass::load()` / graph adapter
  executable regression, built against the available ffgraph headless core.
  **Important:** a temporary, test-only event parser was used instead of
  generated Bison/Flex code; this validates integration, not real generator
  output. The shim is **not** included in the patch.
- **Passed:** actual legacy-selected `FFGraphClass::load()` executable
  regression, linking the existing GT parser/scanner.
- **Passed:** direct-GT differential fixtures (five-graph parity cases and
  the smoke corpus: 18 graph fixtures, 12 syntax-only and 10 rejected cases),
  with the available headless library and a temporary event-parser shim.
- **Passed:** the modern-selected integration test with ASan/UBSan and leak
  detection on the newly built objects; the linked core archive was not
  fully sanitizer-instrumented.
- **Passed:** top-level CMake configuration in legacy/no-generator mode with
  configure-only Tk and Xpm dependency shims.
- **Blocked:** top-level modern CMake configure: Bison missing on this host.
  The host also lacks regular Tk/Xpm development packages. Consequently no
  full CMake/CTest, full-program GUI, or generated-parser sanitizer run was
  carried out locally. The new GitHub Actions jobs have not been executed.
- **Passed:** clean incremental patch application checks, and whitespace checks.

## Retiring GTParser/GTScanner

Yes: Git history can recover the old parser *if its commits remain reachable*.
Before deletion, commit the working GML migration and create an annotated tag
from the correct commit (and push the tag to the remote):

```sh
# Once the previous checkpoint is committed, BEFORE applying the switch:
git tag -a ffgraph-pre-modern-gml -m 'Legacy GML production baseline'
git push origin ffgraph-pre-modern-gml

# Later retrieve one old file without switching the whole checkout:
git show ffgraph-pre-modern-gml:fgraph/GTParser.y
# Or restore the legacy sources in a separate worktree / branch:
git switch -c investigate-old-gml ffgraph-pre-modern-gml
```

A tag preserves accessibility through rebases and branch deletion, assuming
it is not itself deleted and the remote retains it. Git history is not a
replacement for regression fixtures and test coverage.

Suggested retirement gates: run Debug, Release, full ASan/UBSan + leak checks,
all generated corpus profiles, historical GML examples, and real GUI loads on
Ubuntu. Once they pass and the modern importer has been exercised routinely,
remove the GT source/object files, the runtime backend switch, then separately
retire or replace the old Makefile path. Keep historical source in Git and
retain the GML differential oracles even after the old implementation is gone.
