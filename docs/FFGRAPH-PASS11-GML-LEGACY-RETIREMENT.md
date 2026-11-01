# Pass 11 — Retire Bison++ GT GML parser

## Scope

This is an incremental patch **after** the Pass 11 GUI lifecycle/readiness patch.
The modern GNU Bison/Flex GML parser was already the default; this checkpoint
removes the temporary fallback and its checked-in generated legacy sources.
It does **not** change FF or Rome parsers, graph formats, layout algorithms,
Tk GUI commands, the parked inspector/WSLg issue, or excluded SPQR code.

### Changed implementation

- Delete `fgraph/GTParser.{cc,h,y}` and `fgraph/GTScanner.{cc,h,l}` from the
  active source tree. The historical files remain retrievable from Git commits.
- `FFGraphClass::load()` always dispatches `.gml` input through modern `parse()`
  followed by the semantic-preflight `applyToGraph()` adapter. Existing-node
  ID resets still occur only after successful semantic validation. FF (`.g`)
  and Rome dispatch remain unchanged.
- Make modern Bison/Flex regeneration mandatory in top-level CMake, including
  when `BUILD_TESTING=OFF`. Remove `FFGRAPH_GML_BACKEND` selection and the
  `FFGRAPH_ENABLE_MODERN_GML` opt-in. Reject a stale **legacy** backend cache
  selection rather than silently building a different backend.
- Drop the legacy fallback CI job, retain the Debug/ASan full build jobs and
  the standalone parser jobs, and point GML CI at the post-retirement oracles.
- Remove the old `FFGRAPH_TEST_MODERN_GML` conditional: semantic rejection is
  now mandatory production behaviour.

### Regression strategy after legacy retirement

Old differential test drivers linked directly against `GTControl` and can no
longer run. Their replacements deliberately make **different claims**:

- `test_gml_graph_regression_pass11`: curated real graph structures, generated
  corpus independent TSV oracles, repeatability, and rejection without mutation.
- `test_gml_corpus_oracle_pass11`: the production `FFGraphClass::load()` path
  and direct Bison/Flex event replay agree, **and** each graph is compared to
  independently generated TSV topology/coordinate/label oracles.
- `test_gml_metamorphic_oracle_pass11`: production/direct replay agree across
  generated metamorphic fixture pairs; separate graph-corpus metamorphic tests
  verify input transformations and layout invariants.
- Existing corpus parser, graph I/O, ownership, semantic atomicity, and
  Tcl/Tk GUI file workflow tests remain registered.

**Do not interpret production/direct replay equality alone as an independent
parser compatibility proof.** Historical legacy-vs-modern comparisons from
previous commits provide that evidence; the independent TSV oracles provide
ongoing graph-semantic regression coverage.

## Preserve the historical reference

**Before applying the retirement patch**, ensure your working Pass 11 GUI
checkpoint is committed (a Git tag points at a commit, not at uncommitted
changes). Avoid blindly staging unrelated local files.

```sh
git status --short
# Commit the intended current state in your normal workflow, then:
git tag -a ffgraph-pre-legacy-gml-removal \
  -m 'Last checkpoint with GTParser/GTScanner'
# Optional: make the tag recoverable from remote clones
git push origin ffgraph-pre-legacy-gml-removal

# Recovery example after removal:
git show ffgraph-pre-legacy-gml-removal:fgraph/GTParser.y
```

If that tag already exists, keep it rather than overwriting it; locate another
verified pre-removal commit if you need a different checkpoint.

## Apply from WSL

```sh
git apply --check /mnt/c/Users/User/Downloads/ffgraph-pass11-gml-legacy-retirement.patch
git apply /mnt/c/Users/User/Downloads/ffgraph-pass11-gml-legacy-retirement.patch
```

## Build and tests (Ubuntu 24.04)

GNU Bison **>= 3.8** and Flex **>= 2.6** are now required. Ensure the existing
Ubuntu Tcl/Tk, X11 and Xpm dependencies are installed.

```sh
# The -U flags remove old cache entries from your existing build directory.
cmake -S . -B build/gml-modern-default -G Ninja \
  -U FFGRAPH_GML_BACKEND -U FFGRAPH_ENABLE_MODERN_GML \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  -DFFGRAPH_GML_DIFFERENTIAL_PROFILE=standard
cmake --build build/gml-modern-default --parallel 2
ctest --test-dir build/gml-modern-default --output-on-failure

# Selected graph/GML/GUI checks:
ctest --test-dir build/gml-modern-default \
  -R '^test_(gml_.*|graph_corpus_.*|graph_io_pass10|load_window_lifetime_pass11|gui_file_workflow_pass11)$' \
  --output-on-failure
```

Use separate build directories for `smoke`, `standard`, and `stress` if desired.
`FFGRAPH_GML_DIFFERENTIAL_PROFILE` retains its historical name so previously
used configuration commands continue to work; it now selects **oracle**
fixtures rather than legacy-vs-modern differential tests.

For sanitizers:

```sh
cmake -S . -B build/gml-retirement-asan -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  -DFFGRAPH_SANITIZERS=ON -DFFGRAPH_GML_DIFFERENTIAL_PROFILE=stress
cmake --build build/gml-retirement-asan --parallel 2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
ctest --test-dir build/gml-retirement-asan --output-on-failure
```

### Local verification of this patch

- Reconstructed the tree through the **GUI lifecycle readiness** checkpoint.
- Checked Python graph corpus suite: **9/9 tests passed**.
- Strict C++17 syntax-check of `FFGraphClass.cc` and all three changed GML
  test drivers using the available Tcl header bundle: **passed**.
- Git patch whitespace/check and clean application back to an untouched copy
  of that checkpoint: see patch packaging validation.
- Checked GitHub Actions YAML parsing and target-name references.
- **Not run here**: real GNU Bison/Flex regeneration, full CMake/CTest, GUI
  runtime or leak-detected ASan. This environment lacks the Bison/Flex and
  Tk/Xpm development toolchain, so your Ubuntu checks are essential.

## Follow-up

With the parser cleaned up, Pass 12 should modernise FF and Rome **separately**,
using a format-specific corpus and verified golden observations before changing
their production parser implementations.
