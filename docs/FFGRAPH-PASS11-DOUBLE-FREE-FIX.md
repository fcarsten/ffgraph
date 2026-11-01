# Pass 11 follow-up — modern GML scanner double-free

**Applies after:** `ffgraph-pass11-gml-pilot.patch`. This is an incremental
correction to the *opt-in* modern GML parser pilot, not a replacement for the
production `GTParser`/`GTScanner` implementation.

## Reported failure

On Ubuntu, the first `ctest` for `test_gml_modern_pass11` aborted with:

```
free(): double free detected in tcache 2
```

## Root cause

`ffgraph_gml::parse` created an active Flex scan-bytes buffer. Its local
`ScannerGuard` called both `ffgml__delete_buffer` and `ffgml_lex_destroy`.
The current input buffer is already destroyed by Flex's `yylex_destroy`;
deleting it explicitly beforehand leaves a dangling current-buffer entry in
the scanner stack and makes `lex_destroy` attempt to free it again. This is
separate from the grammar's `%destructor` for independently allocated KEY and
STRING token text, which remains necessary for rejected input.

## Changes

- `fgraph/modern/gml_events.cc`: scanner guard now calls only
  `ffgml_lex_destroy`; the automatically owned active input buffer is freed
  once. Explicit buffer deletion is removed.
- `tests/modern_gml/test_gml_modern_pass11.cpp`: repeats valid, empty and
  malformed parser lifecycles to catch cleanup errors and leaks.
- `tests/modern_gml/CMakeLists.txt`: adds optional
  `-DFFGRAPH_MODERN_GML_SANITIZERS=ON` for standalone ASan/UBSan builds.
- `.github/workflows/gml-parser-check.yml`: extends manually triggered
  validation to include the standalone sanitizer test with leak detection.

## Verification on Ubuntu

```sh
cmake -S tests/modern_gml -B build/modern-gml -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build/modern-gml --parallel 2
ctest --test-dir build/modern-gml --output-on-failure

cmake -S tests/modern_gml -B build/modern-gml-asan -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  -DFFGRAPH_MODERN_GML_SANITIZERS=ON
cmake --build build/modern-gml-asan --parallel 2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1 \
  ctest --test-dir build/modern-gml-asan --output-on-failure
```

Run `ctest` after rebuilding: simply rerunning the old binary will reproduce
its previous double-free. CMake should recompile changed sources automatically.

## Scope and verification limitation

This correction follows Flex's documented current-buffer ownership rule.
The user-reported test failure is consistent with the identified double-free,
but the crash was **not reproduced locally**: the environment that produced
this patch does not have Bison or Flex installed and cannot download them
because external package repository DNS resolution fails. The updated C++
wrapper and test are syntax-checked, and the patch is checked against the
Pass 11 pilot baseline. A fully generated build and its CTest result still
need to be verified on Ubuntu.

No changes to the Tcl/Tk GUI, SPQR, production graph parsers, or file formats.
