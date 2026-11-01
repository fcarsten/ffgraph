# ffgraph — Pass 12 handover for Codex: FF/sgraph parser modernisation

**Prepared:** 1 November 2026  
**Target environment:** Ubuntu 24.04 under WSL2/WSLg, GCC or Clang, C++17, CMake/Ninja, Tcl/Tk 8.6, X11 and Xpm  
**Repository:** https://github.com/fcarsten/ffgraph  
**Proposed repository location for this document:** `docs/FFGRAPH-PASS12-FF-PARSER-HANDOVER.md`

> **Start with the user's actual local Git checkout.** This document describes the source reconstructed through the Pass 11 legacy-GML-retirement patch and the results reported in the preceding ChatGPT sessions. The user's branch may be ahead of, or not yet include, that patch. Do not treat old archive contents or prior patch filenames as a substitute for checking Git HEAD and the working tree.

## 1. Mission and scope

Modernise **the FF/sgraph parser and scanner first**: replace the 1990s Bison++/flex++ implementation with supported GNU Bison/Flex generation while preserving the native graph-file format, existing application behaviour and public graph-loading APIs. Use the completed modern GML migration as a model, but **do not blindly reuse its grammar or semantics**. The FF format has different syntax and more extensive graph/UI side effects.

Deliver incremental, reviewable work with meaningful regression coverage. Initially keep the legacy FF parser as a comparison oracle and working fallback. Do **not** switch the production FF parser to modern or delete its checked-in generated sources until real generated-parser tests and semantic equivalence have passed on Ubuntu. Make the final production switch and retirement separate, explicitly verifiable checkpoints unless the user asks otherwise.

**Out of scope:** Rome parser (later Pass 12 stage), Tcl/Tk UI redesign, graph-format redesign, major layout refactors, the parked inspector/WSLg visibility problem and the excluded/incomplete SPQR implementation.

## 2. Where the project stands

The following milestones preceded this handover:

- Passes 8–10: container/Pix and AVL/MPlex correctness, graph invariants and ownership, graph I/O and memory-safety tests; important Tk lifetime fixes.
- Pass 11: modern GNU Bison/Flex GML grammar and reentrant scanner; corrected Flex/Bison token/buffer double frees; graph semantic adapter and preflight; independent generated graph corpus and graph-oracle tests; metamorphic layout tests; production GML integration; actual Tcl GUI File → Load/Merge lifecycle tests.
- The user reported the modern GML corpus profiles (`smoke`, `standard`, `stress`), semantic-safety tests, and GUI lifecycle tests passing in their Ubuntu environment.
- A Pass 11 **legacy GML retirement patch** was delivered, removing `GTParser`/`GTScanner`. The user also created/pushed an annotated **`ffgraph-pre-legacy-gml-removal`** tag preserving the previous version. **Verify in Git whether the retirement patch itself is applied and whether the tag exists; the conversation does not establish the exact current HEAD.**
- The user prefers a Markdown file under `docs/` for every substantive checkpoint, documenting scope, tests actually run, compatibility decisions and unresolved issues.

**Existing GML infrastructure worth reusing:** `fgraph/modern/modern_gml.{y,l}`, `fgraph/modern/gml_events.{h,cc}`, `fgraph/modern/gml_graph_adapter.{h,cc}`, `tests/modern_gml/`, `tests/graph_corpus/`, graph snapshot/oracle helpers and corresponding CMake/CTest/CI wiring. These are architectural examples and regression infrastructure—not an FF-format specification.

## 3. Start by inspecting the real checkout and baseline

From the ffgraph repository root:

```bash
git status --short
git branch --show-current
git rev-parse HEAD
git log -1 --format='%h %cs %s'
git tag --list 'ffgraph-pre-legacy-gml-removal'
git ls-files 'fgraph/*Parser*' 'fgraph/*Scanner*'
cmake --list-presets
```

If the legacy GML files still exist and the modern-only CMake integration is not present, confirm which Pass 11 patches/commits have actually been applied *before* proceeding. Do not silently apply or replay old patches to an unknown branch. Work on a named branch, e.g. `pass12-modern-ff-parser`, and preserve unrelated local edits.

Baseline commands (adapt to any established local presets or dependency differences):

```bash
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure

cmake --preset asan
cmake --build --preset asan --parallel 2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
ctest --preset asan --output-on-failure
```

Also collect a baseline **FF-only** result for `test_graph_io_pass10` and `contrib/out.g`. Record CTest test names/counts, compiler/Bison/Flex versions, skipped tests, and any pre-existing failures. These are baseline commands, **not tests executed while preparing this handover**.

Likely Ubuntu development dependencies: `build-essential cmake ninja-build pkg-config bison flex tcl-dev tk-dev libx11-dev libxpm-dev xvfb xauth`. Check `CMakeLists.txt`, `docs/FFGRAPH-DEPENDENCIES.md` and CI instead of guessing package versions. The root project depends on GUI libraries even for many headless tests. Bison 3.8+ and Flex 2.6+ are documented for the modern GML path.

## 4. FF parser inventory and important semantics

| Role | Inspected path / entry point | Notes |
|---|---|---|
| Source grammar | `fgraph/FFParser.y` | Historical Bison++ directives (`%name`, `%header`, `%define LEX`, `%define MEMBERS`, etc.). **Not directly accepted by GNU Bison.** |
| Source scanner | `fgraph/FFScanner.l` | Historical flex++ class/lexer directives and `%header` block; input from `istream` or `String`. **Not directly accepted by GNU Flex.** |
| Checked-in generated code | `fgraph/FFParser.{cc,h}`, `fgraph/FFScanner.{cc,h}` | Used by current CMake build. Keep untouched while establishing the modern pilot. |
| Legacy wrapper | `FFControl` declared in `fgraph/FFScanner.h`, implemented in `fgraph/FFScanner.cc` / `.l` | `Parse(istream&, FFGraphClass*)`, `Parse(const String&, FFGraphClass*)`, error reporting. Preserve externally observable APIs or provide a compatibility wrapper. |
| Production dispatch | `fgraph/FFGraphClass.cc` — `FFGraphClass::load(const String&, BOOL)` | `.g` files go to `FFControl::Parse(stream, this)`; `datei == NO` uses the string parser. The `.gml` branch uses modern GML, and Rome is another branch. |
| Writer / format oracle | `FFGraphClass::_save(...)`, `saveToString`, `saveToFile` | Emits `GRAPH ... = DIRECTED/UNDIRECTED`, internals, numbered nodes/edges and `END`. Saving may **renumber node `value` fields**, so distinguish format semantics from transient in-memory values. |
| Existing FF regression | `tests/test_graph_io_pass10.cpp` | Repeated string/file round-trips, loops and parallel edges, start/end nodes, and `contrib/out.g`. |
| Historical sample | `contrib/out.g` | Extended sgraph format; prior test expects 499 nodes and 499 edges. This is an existing assertion to revalidate, not a promise for every future version. |
| Build integration | `CMakeLists.txt` | `FFGraph` currently includes `fgraph/FFParser.cc` and `fgraph/FFScanner.cc`. Modern GML generation is isolated in `tests/modern_gml/`. |

### FF file format details to preserve

The inspected `FFParser.y` / `FFScanner.l` accept a graph beginning roughly with `GRAPH "name" = DIRECTED` or `UNDIRECTED`, optionally with special internals delimited by `{$` and `$}`, followed by numbered node records and their nested edge lists, ending with `END`. Attributes include:

- **Graph:** graph name, directedness, graph labels `FF_L`, label offset `FF_LO`, scale `FF_SC`, rotations `FF_RX`/`FF_RY`/`FF_RZ`, miscellaneous/unknown internals.
- **Node:** numeric ID (may appear as an edge reference before its own declaration), name, `NP`, `FF_3D`, `FF_C`, `FF_R`, `SN`/`EN`, label types and icon/mask variants (`FF_I`, `FF_NI`, `FF_NM`).
- **Edge:** source implied by containing node, numeric target ID, edge name, `EL`, `FF_3D` route points, color, weight (`FF_WE`, expressed in hundredths), labels and unknown internals.
- **Lexical:** signed decimal integers, quoted strings (including multiline strings in the historical fixture), backslash escapes, whitespace/newlines, `{...}`-style comments, semicolons and commas. The scanner's actual acceptance/rejection rules are authoritative; characterise any surprising behaviour with tests before changing it.

**Critical semantic traps:** The grammar creates nodes when referenced before declaration (`nodeWithValue`/`addNode`), may reuse an existing node with an equal numeric value, and directly invokes graphics, label parsing and window transforms. Multiple edges and self-loops are supported. There is a process-global `unmatchedLabels` set in the legacy scanner/grammar and raw `strdup`/`free` ownership of `IDENTIFIER` tokens. The legacy path resets existing node `value` fields *before* FF parsing in `FFGraphClass::load()`; changing this is a separate compatibility/atomicity decision. A “more standard” grammar can therefore produce different graphs even when it accepts the same text.

### Prior GML migration lessons

1. Explicitly document ownership of scanner buffers and token strings. Avoid a manual buffer delete followed by Flex scanner destruction of the same active buffer.
2. Bison `%destructor` may run on discarded partially reduced symbols during syntax-error unwinding. Do not free semantic values in mid-rule actions if those values may later be destroyed again; use RAII/value types where possible.
3. Compare constructed graphs, not merely parse success or attribute events; include independent expected values so tests cannot compare a backend with itself accidentally.
4. Preserve graph/window lifetimes across Tcl callbacks. GML loading once exposed a graph-handle ownership bug in File → Load; GUI tests caught it where parser tests did not.
5. The generated-corpus tooling previously needed fixes for fresh or interrupted build directories (`--force` with no manifest). Reuse the fixed generator/ownership checks; do not introduce parallel unsafe cleanup logic.
6. Distinguish semantic rejection from allocation/exception failure. A preflight avoids known semantic partial mutations but does not automatically provide a strong exception guarantee during graph commit.

## 5. Proposed Pass 12 milestones

### Stage A — Characterise the legacy FF contract **before changing production**

- Inventory all references to `FFControl` and its generated headers, including historical Makefiles and installed headers.
- Read the full FF grammar and scanner and build a **format-specific** matrix of legal and illegal input; do not assume every GML graph maps losslessly into FF.
- Use `contrib/out.g`, writer-generated graphs, and several small hand-written `.g` files covering empty/singleton/disconnected, forward references, loops, parallel edges, directedness, start/end nodes, 2D/3D geometry and edge bends, named labels, icons/masks, escaped quotes, multiline strings, unknown fields and malformed/truncated input.
- Record legacy outputs and error behaviour in *independently specified* fixtures. Compare normalised graph structure and explicitly relevant side effects; avoid assuming node order or iteration order unless required by the format.
- Add a dedicated FF baseline regression executable and CTest registration, runnable without a display where possible. Keep Tk tests separate.

### Stage B — Modern FF Bison/Flex pilot, out of the application path

- Create modern `fgraph/modern/modern_ff.{y,l}` (names illustrative), a small parse-result/event representation and a standalone `tests/modern_ff/` CMake/CTest target. Prefer a pure parser/reentrant scanner and explicit input context; consider RAII types instead of owning raw `char*`.
- Generate source into the **build tree** with Bison/Flex, using clearly separated parser/scanner symbol names and headers. Do not overwrite checked-in legacy `FFParser.*`/`FFScanner.*` during the pilot.
- Keep the lexical and grammar rules faithful to the legacy language first. Preserve line diagnostics, token escape handling, EOF handling and reentrant repeated parses.
- Test successful and malformed inputs, deeply nested inputs, invalid numeric ranges, repeated parse attempts, scanner reset and memory ownership under ASan/UBSan (including leak detection).

### Stage C — Semantic adapter and real differential tests

- Bridge modern parse results to the real `FFGraphClass` using the established public graph operations. Plan carefully around `FFGraphWindow`, optional graphic labels and callbacks; an event-only parser is **not** a production replacement.
- While legacy FF exists, run each valid fixture through the legacy and modern implementations to separate graphs. Compare node/edge counts, directedness, topology and edge multiplicities, node/edge names and IDs (where meaningful), start/end nodes, labels, geometry, weights, and defined GUI-side settings. Supplement with writer-generated round-trips.
- Test malformed FF files against already populated graphs; define desired failure atomicity deliberately and report any differences from the historical implementation. Verify no crash, double free, leak or dangling graph/window references.
- Add a stable set of **format-independent graph snapshots / golden oracles** that will survive later removal of the legacy parser.
- Keep both the file-based `load("... .g", YES)` path and `load(serialized, NO)` path under test. The latter is easy to overlook.

### Stage D — Integration, then retirement (separate review points)

- Only after successful differential and sanitizer checks, integrate as a selectable production backend (legacy default or documented temporary fallback, as appropriate to project stage). Test File → Load, Merge, Close and reloading under Tk/Xvfb.
- When confirmed on Ubuntu in Debug, Release, ASan/UBSan and representative interactive GUI use, switch the modern backend to default and finally remove the old parser and obsolete Makefile/CI references. Keep permanent golden/oracle tests.
- Create and push a stable **pre-FF-retirement** tag when the last known-good legacy-FF version has been committed. Do not overwrite the existing pre-GML-removal tag. Only tag after confirming current HEAD and whether removal is truly next.
- Leave Rome parser migration for a later checkpoint.

## 6. Testing and exit criteria

| Area | Minimum evidence before replacing production FF parser |
|---|---|
| Baseline | Recorded Git revision, compiler/generator versions, Debug/ASan test summary and original FF tests |
| Grammar/scanner | Real GNU Bison/Flex generation and compile, not only source syntax checks or stubs |
| Format coverage | Valid and intentionally invalid `.g` corpus, stable expected outputs, edge cases above |
| Differential parity | Legacy and modern **independently invoked** on identical bytes; graph snapshots compare; intentional deviations documented |
| Round trips | Save → parse → save/parse for both file and string paths, without topology corruption |
| Semantic and lifetime safety | Repeated imports and failures; ASan/UBSan with leak detection, no new actionable errors |
| GUI | Tk/Xvfb File → Load/Merge/Close/Reload; no disappearing windows or unexpected current-graph mutation |
| Builds | Debug, Release and ASan CMake/CTest on Ubuntu; generated sources stay in build tree |
| Change hygiene | Small Git-compatible diffs, regression tests, build/CI updates, a Markdown verification note with exact commands/results |

Do not claim the new FF implementation is compatible based solely on a test-only parser shim or an event-stream test. If full build/test facilities are missing, state exactly what was exercised and what remains unverified.

## 7. Suggested local commands

These commands are suggestions for **Codex to run from the user's repository** after checking its current configuration:

```bash
# Review the source of truth and parser callers.
rg -n 'FFControl|FFParser|FFScanner' fgraph include CMakeLists.txt tests
sed -n '1,460p' fgraph/FFParser.y
sed -n '1,250p' fgraph/FFScanner.l
sed -n '1120,1450p' fgraph/FFGraphClass.cc

# Confirm versioned tools.
bison --version | head -1
flex --version

# Exercise existing FF I/O baseline (check exact registered test names first).
ctest --test-dir build/debug -N | grep -Ei 'graph_io|parser|graph'
ctest --test-dir build/debug -R 'graph_io_pass10' --output-on-failure
```

If a generated `.patch` needs to be applied under WSL, **the user specifically requests absolute Windows Downloads paths**, for example:

```bash
git apply --check /mnt/c/Users/User/Downloads/ffgraph-pass12-ff-parser-pilot.patch
git apply /mnt/c/Users/User/Downloads/ffgraph-pass12-ff-parser-pilot.patch
```

If Codex edits the checkout directly, prefer reviewing `git diff --check`, `git diff --stat` and `git status --short`; do **not** tell the user to apply a patch that is already applied. All source files and tests should be version-controlled, but generated build artefacts, local caches and downloaded archives should not be added to the patch.

## 8. Collaboration / reporting expectations for Codex

1. Verify current HEAD and baseline before changes; distinguish user-reported prior test passes from tests run by Codex.
2. Present a concise implementation plan, then execute the first **useful checkpoint** in the same working session: baseline FF tests, focused fixture corpus and, if practical, the isolated modern grammar/scanner pilot.
3. Avoid modifying `RomeParser`, `RomeScanner`, Tcl/Tk GUI semantics, layout algorithms, SPQR, or unrelated containers.
4. Keep APIs and file formats compatible. Make behaviour changes only with failing-before/passing-after tests and rationale.
5. For every checkpoint, create `docs/FFGRAPH-PASS12-FF-*.md` with the exact scope, files, test commands, results and remaining limitations.
6. Report branch/commit, source changes, tests actually run and any follow-up user commands. Do not assume the user has already applied a patch or that a successful configure equals a successful build.

## 9. Other documentation to read first

- `docs/FFGRAPH-PASS11-GML-LEGACY-RETIREMENT.md`
- `docs/FFGRAPH-PASS11-GML-PRODUCTION-SWITCH.md`
- `docs/FFGRAPH-PASS11-GML-SEMANTIC-ATOMICITY.md`
- `docs/FFGRAPH-PASS11-GUI-LIFECYCLE-RETIREMENT-READINESS.md`
- `docs/FFGRAPH-PASS11-DOUBLE-FREE-FIX.md` and `docs/FFGRAPH-PASS11-PARSER-OWNERSHIP-FIX.md`
- `docs/FFGRAPH-PASS10-VERIFICATION.md`
- `docs/FFGRAPH-DEPENDENCIES.md`, `BUILD-CMAKE.md`, `CMakePresets.json`
- `tests/test_graph_io_pass10.cpp`, `contrib/out.g`, and the current `CMakeLists.txt`

**End-of-handover boundary:** No FF parser migration code has been created as part of preparing this document. The first Pass 12 deliverable should establish the actual baseline and format coverage, then introduce a strictly opt-in modern FF parser/scanner without altering the working application by default.
