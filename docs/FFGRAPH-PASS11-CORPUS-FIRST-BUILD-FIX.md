# Pass 11 follow-up — corpus first-build initialisation fix

## Scope

Incremental fix on top of `ffgraph-pass11-graph-corpus.patch`. The two CMake
corpus-generation commands use `--force` because they must support rebuilding.
CMake/Ninja may create the output directory before the generator's first run.
Previously, `write_corpus()` treated any existing directory as a previous
corpus and attempted to open `manifest.json`, which fails for an empty directory.

## Changes

- Accept an **existing, empty directory** with `--force`, treating it as a
  first-time generation rather than a replacement.
- Keep the prior strict safety check for **nonempty directories**: a valid
  generated manifest and an exact known file footprint are required before
  deletion/replacement. Unknown files are never deleted by `--force`.
- Reject output-path symlinks, including dangling symlinks.
- Add a regression test for a pre-created empty output directory, initial
  generation, an idempotent rebuild, `--check`, and protection of unrelated
  files.

No changes to the grammar, generated graph contents, CMake configuration,
production importer, Tcl/Tk interface or file formats.

## Verification on the available test host

- Reproduced both reported failures using the previous generator with
  pre-created empty output directories.
- Python unit tests: **6/6 passed** after the fix.
- Replayed the two CMake generator commands with pre-created directories:
  both succeeded (34 fixtures for layout, 40 fixtures with samples for modern
  parser). Both `--check` commands succeeded, as did both repeated `--force`
  regenerations.
- Checked incremental patch application against the previous corpus source.
- Full project CMake/CTest was **not** run here. This host does not have the
  required Bison/Flex and Tcl/Tk/Xpm development packages.

## Apply and retest (from the ffgraph repository root)

```bash
git apply --check ffgraph-pass11-corpus-first-build-fix.patch
git apply ffgraph-pass11-corpus-first-build-fix.patch

python3 -m unittest discover -s tests/graph_corpus -p test_generate.py -v
cmake --build build/pass11-corpus --parallel 2
ctest --test-dir build/pass11-corpus --output-on-failure
```

You should not need to delete the build directory or reconfigure CMake.
