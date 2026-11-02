# Pass 12 — supplied Rome collection verification

Current production status: [modern-only Rome and legacy retirement](FFGRAPH-PASS12-ROME-LEGACY-RETIREMENT.md).
The results below describe an earlier checkpoint with legacy still present.

Date: 2 November 2026. Branch: `pass12-modern-rome-parser`.
Starting commit: `3f2f16102e4597ea70b56e024d48c5505a65d9d2`.
The checkout was clean. The user supplied the directory
`/home/carsten/work/demographs/rome` from another system and requested testing
the isolated modern parser with it. No commits, pushes or tags were made.

## Problem and scope

The pilot had only synthetic fixtures and generated source-graph oracles.
Real files could expose format assumptions or graph-construction differences
not represented there. The supplied collection now closes that coverage gap.

No parser/scanner, graph adapter, production loading API or backend selection
changed. Rome remains legacy in the application; the modern implementation is
still an isolated opt-in pilot. FF/GML behavior, SPQR and the parked inspector
issue are unaffected. Dataset files were read only and remain outside Git.

## Collection inventory

The directory contains **13,767 files**, 13,260,235 bytes of content and 13,385
distinct SHA-256 content hashes. All 382 duplicate copies were tested rather
than deduplicated. The archive/provenance of the collection has not been
independently authenticated; these results describe the files supplied here.

| Family | Files | Accepted graphs | Nodes in accepted graphs | Edges in accepted graphs |
|---|---:|---:|---:|---:|
| gfinali | 11,581 | 11,531 | 603,373 | 796,194 |
| random | 909 | 909 | 49,950 | 81,670 |
| north | 1,277 | 1,277 | 41,032 | 57,578 |
| Total | **13,767** | **13,717** | **694,355** | **935,442** |

Accepted graphs contain 10–108 nodes; the largest edge count is 241. Metadata
fields use 0 and 1. The inventory found no duplicate node/edge IDs, self-loops,
duplicate ordered endpoint pairs or missing/forward endpoints in the numeric
rows. Those cases remain covered by the synthetic tests. Filenames/directory
node counts were not used as expected graph sizes: actual row data determines
the independent oracle.

The complete raw-file manifest fingerprint is:

`a09ea82623d21f26289c0b7e05e977ed38d93061414902eb2487b27c22f33941`

Each sorted manifest line contains the file-byte SHA-256, two spaces, its
relative POSIX path and LF, encoded as UTF-8. The inventory and selected paths are retained
locally in ignored build artifacts:

- `build/rome-real-corpus-inventory.json`
- `build/rome-real-corpus-matrix-paths.txt`

## Malformed files are tested, not skipped

Both parsers reject **50 files**, all under gfinali:

- 40 contain `Errore nel file di ingresso: carattere # non trovato !!!`.
- 7 contain `Devi specificare il file del diagramma e nodi_finali!!!`.
- 3 have a final three-integer edge row, missing its target field.

The first 47 are tool-error messages rather than complete graphs. The remaining
three preserve these independently checked legacy prefixes:

| Relative file | Malformed line | Legacy prefix nodes / edges |
|---|---:|---:|
| gfinali/graficon39nodi/grafo7417.39 | 243: `138 0 104` | 104 / 137 |
| gfinali/graficon40nodi/grafo7223.40 | 246: `135 0 10` | 110 / 134 |
| gfinali/graficon41nodi/grafo7312.41 | 154: `48 0 70` | 105 / 47 |

Legacy rejection still changes directedness, resets existing IDs on public file
loads and retains valid prefixes. Modern rejection retains no partial records
and leaves destination state/callbacks unchanged. The new parser also reports
the actual rejected line. These are the documented pilot policy differences;
there was no new successful-input compatibility difference in this collection.
No source graph was repaired or normalized for testing.

## Harness implementation

`test_rome_compatibility_pass12.cpp` now accepts:

- `--corpus DIR`: recursively read every regular file, with no extension filter.
  Compare real legacy stream parsing and modern replay on fresh bare graphs.
- `--corpus-matrix DIR`: select sparsest/densest accepted graphs per top-level
  family and actual node-count decade, plus every rejected file. Sorted paths
  resolve ties. Exercise graphic/bare labels, fresh/merge destinations and all
  three legacy entry points (stream, string, public file load).

Public-file cases use temporary `.rome` copies of the original bytes, avoiding
the loader's historical substring-based filename dispatch. Source files stay
unchanged.

For this dataset the matrix contains **110 files**: 60 accepted representatives
and all 50 rejected files, producing **1,320 comparisons**. Every family and
each represented node-count decade participates. A merge seed uses INT_MIN to
avoid aliasing ordinary real-file IDs. Original synthetic tests retain seed
41 and their dedicated duplicate-ID/negative-sentinel cases.

An independent strict row model reads two/four-integer records without consuming
modern-parser results. It models source order and valid prefixes, checking node
IDs, edge IDs/endpoints/direction/multiplicity, degree counts, membership, graph
counts and weights. Successful graphs also receive the existing full snapshots:
names, start/end nodes, custom payloads, geometry/ranges/colors/icons/masks/shapes
and edge routes. File load lifecycle events are compared separately. Rejected
modern imports must preserve the initial full snapshot and emit no mutations.

The external oracle requires ordinary numeric rows, unique node IDs, blank lines
and column-one comments. Unsafe out-of-range values fail preflight before legacy
atoi is invoked, including integers too large for long long. Permissive lexical
quirks remain covered by the existing synthetic tests. Error reports identify
the file and graphics/merge/entry-point combination.

Bulk corpus tests turn off the redundant nested parse performed on every ADD_NODE
by the existing test Watch label. Event collection remains active; dedicated
synthetic nested/concurrent parser tests remain unchanged. This avoids millions
of unnecessary scanner invocations without weakening their separate coverage.

`tests/modern_rome/CMakeLists.txt` registers two small corpus-reader smoke CTests
using new synthetic `corpus_smoke` fixtures. They cover accepted/empty inputs,
tool-error text, truncated edges and both driver modes without external files.
An optional `FFGRAPH_ROME_CORPUS_DIR` cache path registers full/matrix external
CTests with 30-minute timeouts. An invalid configured path is a configure error;
an empty collection fails at runtime. Normal CI needs no private dataset or
machine-specific path. The original tests remain enabled.

README/build instructions and the current roadmap document the new commands.
The earlier pilot report links here while retaining its historical results.

## Tests actually executed

Environment: Ubuntu 24.04, GCC 13.3.0, CMake 3.28.3, Bison 3.8.2, Flex 2.6.4,
Ninja and system Tcl/Tk 8.6.14. ASan/UBSan runs used explicit halt-on-error and
leak detection. Xvfb/LeakSanitizer execution required running outside sandbox
display/tracing restrictions. No suppressions or dependency exclusions changed.

| Check | Actual result |
|---|---|
| Pre-edit Debug CMake/build/full CTest | 54/54 passed |
| Pre-edit strict headless ASan/UBSan/leaks | 47/47 passed |
| Entire collection, Debug | 13,767/13,767 expected outcomes; 13,717 accepted and 50 rejected |
| Entire collection, Release | Same outcomes; full graphs and independent topology checks match |
| Entire collection, strict ASan/UBSan/leaks | Same outcomes; no sanitizer error or leak report |
| Stratified real-corpus matrix, Debug | 1,320/1,320 comparisons passed |
| Stratified real-corpus matrix, strict ASan/UBSan/leaks | 1,320/1,320 passed |
| Final full Debug CTest, external path unset | **56/56 passed** |
| Final full Release CTest, external path set | **58/58 passed**, including both external corpus tests |
| Final strict headless ASan/UBSan CTest, path unset | **49/49 passed**, including both corpus-reader smoke tests |
| Unsafe legacy-conversion probe | Exit 1 with preflight diagnostic; oversized integer never sent to legacy parser |
| Empty-corpus probe | Exit 1 with explicit empty-directory diagnostic |
| Whitespace and CTest label audit | Passed |

Debug/Release full CTest includes the existing seven production GUI tests.
Corpus sanitizer tests are headless even with graphics labels: they create no
Tk windows. This checkpoint did not re-run unsuppressed GUI leak diagnostics;
the previously isolated toolkit issues and current nonblocking CI policy remain
as documented. No remote workflow was run or pushed for this patch.

## Reproduction commands

From the repository root:

```sh
cmake --preset release -DFFGRAPH_ENABLE_MODERN_ROME_PILOT=ON \
  -DFFGRAPH_ROME_CORPUS_DIR=/home/carsten/work/demographs/rome
cmake --build --preset release --parallel 2
ctest --preset release -R '^test_rome_external_' --output-on-failure
ctest --preset release --parallel 2

cmake --preset debug -DFFGRAPH_ENABLE_MODERN_ROME_PILOT=ON
cmake --build --preset debug --parallel 2
build/debug/tests/modern_rome/test_rome_compatibility_pass12 \
  --corpus /home/carsten/work/demographs/rome
build/debug/tests/modern_rome/test_rome_compatibility_pass12 \
  --corpus-matrix /home/carsten/work/demographs/rome
ctest --preset debug --parallel 2

cmake --preset asan -DFFGRAPH_ENABLE_MODERN_ROME_PILOT=ON
cmake --build --preset asan --parallel 2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
build/asan/tests/modern_rome/test_rome_compatibility_pass12 \
  --corpus /home/carsten/work/demographs/rome
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
build/asan/tests/modern_rome/test_rome_compatibility_pass12 \
  --corpus-matrix /home/carsten/work/demographs/rome
ctest --preset asan-headless --parallel 1
git diff --check
```

Repeat the raw manifest fingerprint independently:

```sh
python3 - <<'PY'
from pathlib import Path
import hashlib
root = Path('/home/carsten/work/demographs/rome')
manifest = hashlib.sha256()
for path in sorted(p for p in root.rglob('*') if p.is_file()):
    digest = hashlib.sha256(path.read_bytes()).hexdigest()
    manifest.update((digest + '  ' + path.relative_to(root).as_posix() + '\n').encode())
print(manifest.hexdigest())
PY
```

Local logs use `/tmp/ff-rome-real-*.log`, including separate baseline, complete
Debug/Release/ASan, matrix and final CTest results. The final ASan full-collection
log is `/tmp/ff-rome-real-full-asan-final.log`.

## Remaining limits and next step

Passing this finite corpus does not establish correctness for arbitrary input,
allocation/callback exceptions or other platforms. The full collection was
checked through direct legacy stream construction and modern replay; all three
entry points, graphics and merge combinations were checked on the documented
matrix, not every file. These tests do not exercise a modern Rome GUI path,
which does not exist yet. External data are not available in remote CI by default.

The next checkpoint remains opt-in production integration with legacy Rome
retained, followed by real GUI File/Load/Merge/Close and 2D/3D lifecycle testing.
No default promotion or legacy removal was performed here.
