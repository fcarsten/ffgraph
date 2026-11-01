# Pass 11 follow-up 2 — GML parser nested-list KEY ownership

**Apply after:** `ffgraph-pass11-gml-pilot.patch`, then
`ffgraph-pass11-doublefree-fix.patch`. This patch is incremental; it does not
replace either earlier patch.

## Symptom

The opt-in standalone modern GML test still aborts with
`free(): double free detected in tcache 2` after the Flex input-buffer fix.

## Root cause and correction

The nested-list production in `fgraph/modern/modern_gml.y` previously freed
its `KEY` pointer in the *midrule action*, immediately after `BEGIN_LIST`.
That KEY remains on the Bison parse stack until the complete list production
reduces. If nested input is malformed, Bison discards the incomplete rule and
invokes `%destructor { std::free($$); } <text>` on the still-stacked KEY, freeing
it again. A concrete trigger is `graph [ node [ id 1` (missing list endings).

The correction retains the KEY through recursive parsing and frees it only
in the **final action after END_LIST**, when reduction succeeds. When parsing
fails, Bison's destructor remains responsible. The original begin/end event
ordering and successful-input semantics are preserved.

This issue is independent of the earlier Flex scan-buffer double-free; **both**
fixes are required.

## Changes in this patch

- `fgraph/modern/modern_gml.y`: relocate KEY cleanup from the list-opening
  midrule action to the final successful list reduction.
- `tests/modern_gml/test_gml_modern_pass11.cpp`: repeat nested malformed
  inputs, including multiple unmatched list levels, missing attribute values,
  and a dangling key after a completed nested list. The failure sequence is followed
  by a successful parse with a fresh scanner. These paths exercise Bison
  `%destructor` while the relevant list KEY is on the parse stack.
- This documentation file.

## Rebuild and verify on Ubuntu

Remove stale generated build output for the standalone pilot, then rebuild with
real Bison and Flex:

```sh
cmake -S tests/modern_gml -B build/modern-gml -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build/modern-gml --clean-first --parallel 2
ctest --test-dir build/modern-gml --output-on-failure -V

cmake -S tests/modern_gml -B build/modern-gml-asan -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  -DFFGRAPH_MODERN_GML_SANITIZERS=ON
cmake --build build/modern-gml-asan --clean-first --parallel 2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1 \
  ctest --test-dir build/modern-gml-asan --output-on-failure -V
```

If there is still a crash, capture the first ASan report with a direct run:

```sh
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:abort_on_error=1 \
  ./build/modern-gml-asan/test_gml_modern_pass11
```

## Verification and scope

The patch is checked for clean application to the **previous Pass 11 scanner
fix** and is statically reviewed against Bison's documented destructor rules.
Bison/Flex are unavailable in the current build environment, so generated
parser/scanner compilation, CTest and ASan execution **could not be verified
here**. Success must be confirmed with the commands above on Ubuntu.

This modifies only the opt-in GML pilot grammar and its regression test. It
leaves the shipping GML/sgraph/Rome parsers, graph formats, Tcl/Tk GUI, parked
inspector/WSLg issue and excluded SPQR code untouched.
