# ffgraph — remaining modernisation, verification and maintenance plan

**Handover date:** 1 November 2026  
**Project:** https://github.com/fcarsten/ffgraph  
**Primary platform:** Ubuntu 24.04, C++17, Tcl/Tk GUI, including use under WSL2/WSLg  
**Purpose:** A self-contained starting document for a new ChatGPT development session.  
**Authoritative source:** **The user's current local Git checkout**, not the earlier uploaded archive. Request a fresh source archive or commit/branch reference at the start of the next session.

## 0. Short brief for the next development session

Continue incremental correctness hardening of ffgraph, a 1990s C++ directed-graph library and Tcl/Tk GUI ported to Ubuntu 24.04. The CMake/Ninja build, CTest suite, install and a succession of memory/ownership patches have been tested successfully by the user. The next recommended task is **Pass 8: comprehensive container compatibility/correctness review**. Then review graph invariants and layout algorithms (Pass 9), establish end-to-end memory-safety/regression infrastructure (Pass 10), modernise parser/scanner generation (Pass 11), and finish portability, performance and documentation cleanup (Pass 12). Keep the GUI and existing file formats. Work in small, independently applicable **Git unified-diff patches**, with regression tests, and do not include `.git`, generated binaries or unrelated files. Do not conflate the separately parked inspector/WSLg window-management issue or the incomplete SPQR decomposition with these passes.

**Before editing:** confirm the working-tree state, baseline Git commit, actual current `CMakeLists.txt`, current tests, and availability of representative graph files. The `ffgraph-current.zip` available in the old chat was uploaded **before** the user's subsequent Priority 2 passes; it must not be treated as the fully updated baseline. A clean `git archive --format=zip --output=ffgraph-next.zip HEAD` works for committed state; include a separate `git diff`/`git diff --cached` if changes are uncommitted, or make an archive of the clean working tree excluding `.git` and `build/`.

## 1. Scope, goals and constraints

### Preserve

- The original Tcl/Tk GUI, inspectors and user-visible graph-editing interactions.
- C++17 toolchain compatibility on Ubuntu 24.04. Prefer standards-compliant C++ where practical without gratuitous rewrite.
- Existing sgraph and GML input/output behaviour, including historical extensions; preserve semantics unless correcting a demonstrated bug.
- Reference-counted `FFGraph` public handles and public graph/container APIs where supported by current application code.
- CMake + Ninja/Make builds, CTest, Debug/Release/ASan presets, and install behaviour.

### Working rules

1. Diagnose first; identify concrete conditions for a bug. Make behavioural changes explicit.
2. For compatibility containers, verify *actual historical calling contracts* before changing invalid-handle behaviour or iterator/index semantics.
3. Add a failing regression test for each confirmed defect where feasible; only then fix it.
4. Test representative real graphs, degenerate graphs and input errors. Distinguish library correctness from GUI/window-manager effects.
5. Validate a clean build from a clean tree; avoid local stale outputs masking missing dependencies.
6. Provide a patch generated using Git and verify with `git apply --check` on the relevant baseline. Include a short change log, test commands and results/limitations.
7. Keep experimental SPQR disabled and the inspector/WSLg issue parked unless explicitly requested.
8. Do not state that sanitiser or application tests passed when only syntax checks or standalone tests ran.

## 2. Work completed so far — contextual history

The following is based on the user's reports and patches from the previous conversation; **re-verify against the fresh checkout**.

| Area | Recorded status |
|---|---|
| Priority 1: 3D identifier and selection-export buffer overflows, XPM loading bounds/termination, `MPlex` boundary deletion and self-append/prepend, unsafe sort | Patched; user reported builds/tests worked |
| CMake migration | CMake/Ninja and CTest working; installed successfully on user's Ubuntu machine |
| Legacy build system | User was advised how to retire old Makefiles/configuration scripts; actual final file removals must be checked in current checkout |
| Tcl test | Restored against real Tcl 8.6 API, after initial mock test compile issues; passed |
| SPQR | `FFSpqr.cc`, `FFSpqrNodeLabel.cc` excluded from normal build; `FFSpqr.ok` retained as an unfinished alternative |
| Priority 2 Pass 1 | Standard containers for selected VLAs, typed sorting, validated CLI integer parsing; one `knoten = 0` vector-conversion error corrected; build passed |
| Priority 2 Pass 2 | Removed global `SIGFPE` suppression; guards for selected numerical edge cases; applied |
| Priority 2 Pass 3 | Fixed `FFGraph` self-assignment lifetime bug; removed selected allocations; graph-ownership test |
| Priority 2 Pass 4 | Tcl command/callback deregistration and lifetime safeguards; graph/node/edge label self-replacement guards; tests |
| Priority 2 Pass 5 | Removed internal graph reference cycles using observers, preserving owning public handles; tests |
| Priority 2 Pass 6 | Correct replacement graph-label initialisation; non-GUI Tcl interpreter cleanup; tests |
| Inspector visibility patch | Addressed deiconify/raise and destroyed windows, but intermittent window/taskbar behaviour still reported; **parked** |
| Priority 2 Pass 7 | Deferred destruction of Tk-initialised Tcl interpreters until safe event-loop boundaries; new Xvfb teardown tests with focused ASan/UBSan checks; user reported working |

The **user reported the current code builds and tests pass**, with no obvious problems during ordinary use. This is meaningful smoke-test evidence, not exhaustive memory or algorithm verification.

### Known deferred topics

- **Inspector/WSLg:** Intermittent inspector state/visibility and GUI responsiveness on WSLg. Standalone Tk tests confirmed deferred visual hide transitions and recurring ~2 s stalls in `iconify; deiconify; raise`. See the separately created `ffgraph-inspector-wslg-investigation-handover.md`. Do not resume in ordinary correctness passes.
- **SPQR:** Two incomplete implementations (`flayout/FFSpqr.cc`, `flayout/FFSpqr.ok`); not a supported production feature. Leave excluded until separately prioritised.
- **Parser generators:** Build currently uses checked-in generated parser/scanner C++; legacy `.y/.l` should eventually migrate to maintained Bison/Flex without changing semantics.

## 3. Initial baseline and reproducibility checks (do these first)

Collect in the user's current Git checkout:

```bash
git status --short
git rev-parse HEAD
git log -1 --format='%h %cs %s'
cmake --list-presets
cmake --preset debug
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
```

Record number and names of CTest tests (`ctest --test-dir build/debug -N`) and failures. Run the optional dependency snapshot script supplied alongside this document; it writes into `build/dependency-report/` by default. Have at least one working sample for each parser format and, if possible, large and pathological graph fixtures. If only a stale ZIP is available, do not generate patches against it and assume they will apply unmodified to the local tree.

### Baseline validation matrix

| Configuration | Expected outcome |
|---|---|
| Debug + CTest | Full clean build and all ordinary tests pass |
| Release + CTest | Clean build, equivalent tested behaviour |
| ASan/UBSan + CTest | No actionable reported issues; isolate any third-party false positives |
| Tk/Xvfb | GUI lifecycle tests run automatically if Xvfb is available |
| Interactive WSLg | Smoke test open/save/layout/close; record separately from deterministic Xvfb tests |
| Install staging | `cmake --install ... --prefix <temp>` installs binaries and Tcl/icon assets; launch away from source directory |

## 4. Pass 8 — compatibility containers and data-structure correctness

**Priority: High. First development pass.**

### Target areas

- `include/container/MPlex.h` and `container/MFPlex.cc`, and any other `MPlex`/`MFPlex` implementations.
- `include/container/VHMap.h`, `VHBag.h`, `VHSet.h`, `VOHSet.h` and `FF*` implementation wrappers.
- AVL containers in `container/` and corresponding `include/` compatibility headers.
- `DLList`, `SLList`, stacks, hash pointer compatibility (`Pix`) and related iteration helpers.

### Questions and candidate defects

1. Are `low`/`high`/`fence`, valid indices, holes, deletion and insertion semantics internally consistent after the STL uplift? Check empty containers, all-deleted containers, sparse indices, and repeated boundary operations.
2. Can `Pix`/iterator handles become dangling or silently refer to unrelated objects after erase/rehash/clear/copy/move? Distinguish expected legacy invalidation from accidental divergence.
3. Do key-access methods expose mutable references to `std::unordered_map` keys (`const_cast`), allowing hash-table invariant violations? Audit all call sites before changing the interface.
4. Do invalid/null `Pix` APIs return a mutable default value, assert, throw or invoke a legacy error handler inconsistently? Verify actual callers and document deliberate behaviour.
5. Is self-assignment, self-append, self-prepend or aliasing safe in all relevant container classes?
6. Are comparison, hashing, equality, duplicate handling, iteration order and `count()`/`OK()` consistent with intended contracts?
7. Are copy operations exception-safe (e.g., `DLList` copy assignment), and is there signed integer overflow near index boundaries?
8. Does `MPlex` implement `Pix` lookups with full scans? Establish whether this is a real hot-path performance regression; avoid premature redesign.

### Tests to add

- **State/invariant tests:** table-driven mutations on empty, singleton, sparse, large and all-deleted containers.
- **Model comparison:** compare supported operations with reference `std::map`, `std::set`, `std::list`, etc., after translating the legacy contract; do not demand identical iteration order if not guaranteed.
- **Property-based or deterministic randomised operation sequences:** fixed seeds, recorded operations, invariant checks after each step.
- **Iterator lifetime tests:** clear/erase/insert/rehash/copy; only dereference handles where documented valid.
- **Alias/copy tests:** assignment, self-append/prepend, copy and movement where supported.
- **ASan/UBSan:** instrument all new tests; include edge-case integer tests without purposely invoking undefined behaviour outside explicit isolated test cases.

### Exit criteria

- All confirmed container defects have a minimal reproducer and test.
- Existing valid API semantics preserved or behavioural changes documented.
- Container unit suite passes Debug/Release/ASan and does not use Tcl/Tk.
- Performance-sensitive changes have before/after benchmark figures.

**Deliverable:** `pass8-container-correctness.patch`, test additions, short compatibility/behaviour notes.

## 5. Pass 9 — graph integrity, geometry and layout correctness

**Priority: High.**

### Targets and invariants

- `FFGraphClass`, `FFNodeClass`, `FFEdgeClass`, labels and subgraphs.
- Graph copying and assignment; graph-to-label observer relationships; replacing labels by index/type.
- Edge adjacency/endpoint consistency, incidence counts, node/edge/subgraph membership, deletion side-effects.
- Graphic geometry (`FFGraphicEdge`, `FFPoint`, transforms) and layout algorithms (`FFspringembedder`, `FFSpring`, `FFSugi`, `FFOrtho`, `FFKaese`, etc.).

Define test helpers for graph invariants: every stored edge's endpoints exist, each edge appears in expected incidence collections, node/edge counts agree with enumeration, deleted nodes leave no dangling references, subgraph membership is valid, and label observers point to their current graph. Do not assume all graph types prohibit multiedges or loops—test *documented* behaviour.

### Test matrix

- Graph structures: empty, singleton, path, cycle, tree, disconnected components, high-degree star, dense graph, multiedges, self-loops, nested subgraphs, isolated nodes.
- Operations: create, copy, modify copy, delete node with incident edges, replace/delete labels, clear, reload, create subgraph, export/import, repeat.
- Layouts: run algorithm with time/iteration bounds; verify no invalid memory access, no NaN/Inf coordinates, no stale edge-point handles, and reasonable domain invariants; don't require byte-identical coordinates if algorithm randomness is intentional.
- Geometry: zero-length and coincident edges, empty routing point lists, repeated straighten/route/transform sequences.

### Historical failure to revisit

A previous Valgrind report showed an invalid read in `MFPlex<FFPoint>::length()` called by `FFGraphicEdge::anzPoints()`, `FFGraphClass::straightenEdges()`, and the spring embedder. **Reproduce the original scenario if a graph fixture is available and check whether current code still fails.** Avoid treating earlier ownership/container fixes as proof that the defect is gone.

### Exit criteria

- A reusable graph-invariant test fixture/helper exists.
- Confirmed violations have failing-before/fixed-after regression tests.
- Core layout operations pass instrumented tests on the pathological matrix.
- Existing graph file behaviour and GUI smoke tests remain stable.

**Deliverable:** `pass9-graph-algorithm-correctness.patch`, reproducible fixtures, invariant checks.

## 6. Pass 10 — comprehensive memory safety and GUI lifecycle validation

**Priority: High.**

### Automate representative end-to-end workflows

Build a small test harness that loads graph fixtures, performs graph modifications and several layouts, saves/reloads output, and exits deterministically. Prefer calling library APIs without a display where possible. Add separate Xvfb tests for GUI-only operations.

Test lifecycle sequences: repeated graph window create/close, inspector create/show/hide/close, subwindow create/destroy, multiple interpreters, Tcl callbacks deleting windows, interrupted or nested Tcl events, repeated graph load/save, and clean application shutdown.

### Tools and conditions

```bash
cmake --preset asan
cmake --build --preset asan --parallel
ctest --preset asan --output-on-failure

# Separate debug run for leak attribution:
valgrind --tool=memcheck --leak-check=full \
  --show-leak-kinds=definite,indirect \
  --track-origins=yes ./build/debug/ffgraph
```

- Run deterministic GUI tests under `xvfb-run` with `xauth` installed; optionally exercise the real WSLg GUI as a separate smoke test.
- ASan and Valgrind are complementary, not interchangeable; don't launch Valgrind over an ASan binary by default.
- Triaging: distinguish ffgraph-owned allocations from Tcl/Tk/X11 library caches; document any suppression files with rationale.
- Use CI for headless tests; optional Ubuntu matrix. Handle ASan leaks at process exit carefully where Tcl/Tk libraries retain global state.
- Confirm CMake builds still work without Xvfb installed: GUI test registration should be optional, not block regular builds.

### Exit criteria

- Repeatable library and GUI lifecycle test targets exist.
- No known ffgraph-owned UAF/double-free/out-of-bounds defects in tested workflows.
- Any residual leaks have allocation stacks and documented ownership/suppression decisions.
- Automated run is reproducible by a new developer with documented packages.

**Deliverable:** `pass10-memory-regression.patch`, CI/test documentation, optional sanitizer suppressions.

## 7. Pass 11 — parser/scanner regeneration and file-format regression

**Priority: Medium; separate migration because of semantic risk.**

### Preparation

- Inventory `.y`/`.l`, checked-in generated `.cc`/`.h` and input formats: sgraph, GML, and other parser paths present in the tree (e.g. `RomeParser`/`GTParser`/`FFParser` paths).
- Collect accepted sample inputs and expected parse results or canonical graph outputs; include malformed and truncated files.
- Preserve input features, diagnostic/error recovery behaviour and encoding expectations as much as possible.

### Migration

1. Maintain current checked-in generated code as a functioning baseline.
2. Convert one grammar/scanner pair at a time to maintained GNU Bison/Flex interfaces (or another explicitly justified generator), adjusting C++ parser integration carefully.
3. Use CMake generator functions/explicit custom commands, generating into the **build** directory with target dependencies; regeneration should be opt-in until validated.
4. Compare old and new parser outputs on the same corpus, including location/diagnostic behaviour where reasonable.
5. Introduce parser fuzzing or targeted malformed-input tests under ASan/UBSan when practical.
6. Only retire generated legacy sources and obsolete generator assumptions once new sources and tests are proven, and keep a documented bootstrap path.

### Exit criteria

- Maintained tools reproducibly regenerate parsers from committed grammar sources.
- No regression in supported input formats on the corpus.
- Clean build documented with and without regeneration, if both modes remain supported.

**Deliverable:** separate parser migration patch(es), corpus, generated-code policy.

## 8. Pass 12 — remaining portability, performance, dependencies and documentation

**Priority: Medium after correctness.**

### Portability/API

- Audit remaining GCC-only extensions, implicit conversions, legacy pointer casts, outdated Tcl interfaces and compiler warnings.
- Check `include/String.h` unsigned-to-signed conversion/`dec()` edge cases (`ULLONG_MAX` etc.).
- Check integer overflow in `MPlex` index arithmetic and robust file-size arithmetic.
- Improve exception safety incrementally where callers would benefit.
- Examine old generated/configuration headers (`config.h`, `ffConfig.h`, `ffGlobdefs.h`) for stale host assumptions, including hard-coded runtime paths.

### Performance

- Benchmark graph/container construction, mutation, traversal, layout, and selected GUI workflows at representative sizes. Suggested scales: 100, 1,000, 10,000 nodes and (where feasible) 100,000 nodes; choose edge density appropriately.
- Capture wall time and memory, compiler/build type, machine and input seed.
- Investigate `Pix` map scans first if they prove hot in profiles; only change implementations after correctness and contract testing.

### Packaging and developer experience

- Use `CMakeLists.txt` as machine-readable dependency source of truth and `CMakePresets.json` for supported build configurations.
- Add a maintained `docs/BUILDING.md` or `DEPENDENCIES.md` covering Ubuntu 24.04 `apt` packages, tests, installation, runtime resource paths and optional dev tools.
- Record dependency discovery versions/paths and link closure using a machine-generated **environment snapshot**; do not commit every developer's local snapshot as though it were a lockfile.
- Add a CI job for clean build and headless tests, pinning the CI environment/container image where warranted.
- Verify installation works when the executable is started **outside the source directory**, with `FF_LIBDIR` and `FF_ICONDIR` handled correctly. Earlier source sets `FF_LIBDIR` by default to `./lib/ffgraph`; this is a known portability/install concern to re-check in current code.
- Replace obsolete statements in legacy `INSTALL`/`BUILD-CMAKE.md` and remove stale Make-era files if still tracked. Preserve license/third-party attribution and project provenance.

### Exit criteria

- Clean checkout → documented packages → configure → build → test → install → run succeeds on Ubuntu 24.04 without relying on source working directory.
- Warnings and performance results documented; no unreviewed functional regressions.
- Versioned developer-facing build/dependency instructions exist, plus optional per-machine environment snapshot.

**Deliverable:** focused cleanup patch(es), developer documentation, baseline performance record.

## 9. Suggested implementation discipline and review gates

For each pass:

1. **Inventory:** select relevant files, current tests, and affected APIs; describe intended invariants.
2. **Reproduce:** minimise a failing example or demonstrate a contract/performance discrepancy.
3. **Fix:** prefer smallest safe change, avoiding adjacent architecture churn.
4. **Verify:** compile, run targeted tests, run full CTest, then sanitiser tests as relevant.
5. **Review patch:** `git diff --check`, inspect staged paths, `git apply --check` against intended baseline, avoid unrelated changes or generated outputs.
6. **Hand off:** provide `.patch`, commands, test evidence, unresolved risks and one clear next pass.

Example patch workflow in a scratch checkout:

```bash
git diff --check
git diff --binary BASE_COMMIT > ffgraph-passN.patch
# In a separate clean checkout at BASE_COMMIT:
git apply --check ffgraph-passN.patch
```

Prefer `git format-patch -1` for an actual commit, because it also records commit message and authorship; either format handles file additions/deletions when produced from Git. Do not use a hand-written patch with guessed line counts.

## 10. Data needed at the start of a new session

- **Required:** fresh archive or accessible commit/branch of the current complete source tree, including all accepted patches; preferably clean.
- **Useful:** representative sgraph/GML files; large graphs and the graph from the old `MFPlex<FFPoint>::length()` failure if available.
- **Useful:** `ctest -N`, recent Debug/ASan results, `cmake --list-presets`, a local dependency environment report, and relevant build/run output.
- **Constraints:** preserve GUI and current file-format semantics; avoid changes to `.git` and unrelated assets; deliver Git patches and focused regression tests; do not start with inspector/WSLg or SPQR unless explicitly requested.

### Suggested opening message for the next chat

> Please continue hardening ffgraph using the attached `FFGRAPH-NEXT-SESSION-PLAN.md` and the attached **latest** source archive. Start with Pass 8 (container correctness/compatibility), particularly MPlex/MFPlex, hash/AVL containers and Pix semantics. Preserve the Tcl/Tk GUI and existing APIs/formats. Make conservative, tested changes and provide a Git-compatible patch containing only modified/new/deleted files. The inspector/WSLg issue is parked and SPQR remains excluded. Check the baseline CMake/CTest/ASan configuration before changes, and report what you could actually verify.

## 11. Companion references

- `FFGRAPH-DEPENDENCIES.md` — dependency taxonomy, minimum setup, runtime resources, provenance and reproducibility recommendations.
- `scripts/capture-build-environment.sh` — captures local installed tool/library versions and resolved paths into an ignored `build/dependency-report/` directory; run in the user's Ubuntu environment.
- `ffgraph-inspector-wslg-investigation-handover.md` — parked GUI issue, created in the previous chat; attach separately if resuming it.

**Accuracy note:** This is a development handover and proposed work plan, not an assertion that every candidate issue is currently present. Verify exact code and test state against the up-to-date checkout before patching.
