# Pass 11 — metamorphic corpus first-build/recovery fix

## Problem

In a modern GML CMake build, Ninja may invoke `tests/graph_corpus/metamorphic.py`
with `--force` and an output directory containing the `base/` and `variant/`
subdirectories or already emitted fixture files, but no `manifest.json`.
`metamorphic.py` delegates generation to `write_corpus()` in
`tests/graph_corpus/generate.py`. The earlier first-build fix only accepted
*completely empty* output directories. A partially populated directory
without a manifest was rejected with `Refusing unsafe --force replacement ...
manifest.json: No such file or directory`.

## Changes

This incremental fix changes **only the shared test-corpus writer, its regression
suite, and these notes**; it does not affect application graph loading or the
GML grammar.

With `--force`, an existing output directory is now accepted when:

- it is completely empty (existing behaviour);
- it has a valid previously generated manifest and the exact expected prior
  file footprint (existing behaviour); or
- its manifest is absent, and **every** existing file matches the exact
  expected bytes for the current seed/profile, and **every** existing directory
  is a parent of one of the generated files.

Unknown/modified files, unknown directories, symlinks, and invalid manifests
remain protected; the generator refuses to delete or overwrite them.
This protects user data even if an output path is accidentally misconfigured.

## Local verification

- Reproduced the original manifest-missing exception with the prior baseline
  when the output contained `base/`, `variant/` or a partial generated fixture.
- Python generator suite: **9/9 tests passed** after the fix, including a new
  regression covering pre-created subdirectories, exact partial output,
  unknown/modified files, a symlink, and a malformed manifest.
- Repeated the CMake-style metamorphic generation command (`smoke --force`) with
  pre-created `base/` and `variant/` subdirectories. Generation, `--check`, a
  second forced regeneration, and the second `--check` all passed.
- The complete top-level CMake build, Bison/Flex generation, and full CTest
  suite were **not executed** on this test host.

## Apply and verify on Ubuntu

Apply after `ffgraph-pass11-modern-gml-default.patch`:

```sh
git apply --check ffgraph-pass11-metamorphic-first-build-fix.patch
git apply ffgraph-pass11-metamorphic-first-build-fix.patch
python3 -m unittest discover -s tests/graph_corpus -p 'test_*.py' -v
cmake --build build/gml-modern-default --target ffgraph --parallel 2
cmake --build build/gml-modern-default --parallel 2
ctest --test-dir build/gml-modern-default --output-on-failure
```

`--target ffgraph` builds only the executable and its dependencies, without
requiring unrelated corpus test targets to finish first. A CMake reconfigure
or clean build is not required for this Python-only correction.

If the output directory contains unknown or changed files and the safety check
continues to refuse replacement, inspect it before removing anything:

```sh
find build/gml-modern-default/gml-differential-metamorphic-smoke -maxdepth 2 -print
```

If it contains only disposable generated test files, you may remove that
specific output directory and re-run the build. Do not delete the entire
CMake build directory merely to work around this generator issue.
