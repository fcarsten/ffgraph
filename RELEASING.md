# Preparing ffgraph releases

The release candidate identity is **1.0.0-rc1**. `VERSION` is authoritative;
CMake derives its numeric package version and `ffgraph --version` reports the
full identity. Retain historical notices and documentation.

## Build and validate

Follow `BUILD-CMAKE.md` for Ubuntu 24.04 or `BUILD-WINDOWS.md` for Windows
x86-64/MSYS2 UCRT64. Validate the exact candidate commit with Linux
Debug/Release, strict headless sanitizers, GUI runtime sanitizers, optional Xpm
checks, and Windows Debug/Release initial suites.

The installed-library test relocates a prefix containing spaces, then compiles
and runs a separate consumer with `find_package(ffgraph)`. The installed-runtime
test launches the actual 2D/3D apps outside the checkout.

## Assemble packages

Packaging needs Git and Python 3.11+. It reconfigures, builds and stages an
existing **Release**, non-sanitized build; run CTest first. It does not commit,
tag, upload or publish anything. From a clean checkout:

```sh
cmake --preset release
cmake --build --preset release --parallel 2
ctest --preset release --output-on-failure --parallel 2
python3 scripts/package-release.py --build build/release --output build/release-artifacts
```

On Windows, after the native Release build and tests, run in MSYS2 UCRT64:

```sh
python scripts/package-release.py --build build/windows --output build/release-artifacts
```

Outputs: source `.tar.gz`, Windows ZIP or Linux installation `.tar.gz`, a
platform-specific manifest and platform-specific `SHA256SUMS`. Linux binaries
use system Tcl/Tk/X11 and target the documented Ubuntu environment. Windows
bundles native runtime DLLs and script libraries.

Each binary archive has a top-level `ffgraph-1.0.0-rc1` containing `bin`, `lib`,
`include`, `share` and `RELEASE-MANIFEST.json`. Keep these together. Notices
are under `share/licenses/ffgraph`, documentation under `share/doc/ffgraph`.
The manifest records the source commit/state, compiler, configuration, Tk
version and installed file hashes. Clean source archives use Git objects at HEAD.

Add `--preview` to package uncommitted work for review/testing. Such artifacts
include `preview` in their names and record the dirty source state; their source
archive includes unignored working-tree files. Regenerate from the final clean
commit before publishing. Use an ignored output directory such as
`build/release-artifacts`, or a directory outside the checkout.

Verify checksums in the output directory, for example:

```sh
sha256sum -c ffgraph-1.0.0-rc1-windows-x86_64-SHA256SUMS
```

CI assembles and uploads packages after successful Release tests. These are
workflow artifacts; a maintainer separately creates the GitHub prerelease.
Do not present a local cross-build preview as an MSYS2/UCRT64 CI artifact.

## Final Windows 11 desktop check

Use the exact final Release ZIP without development tools on PATH. Record its
checksum, Windows version, display scaling and results. Test both 2D and 3D:

1. Extract into a path with spaces; launch by Explorer outside the checkout.
2. Load representative FF/sgraph, GML and Rome files, save/reopen, and check
   topology, names, labels, graphic properties and routes.
3. Run Spring, Sugiyama and orthogonal layouts; change, dismiss and reopen their
   inspectors. Stop continuous Spring mode.
4. Create icon nodes, load legacy image references/missing-artwork placeholders,
   select colours, zoom empty/populated regions and use file-selector navigation.
5. Copy/paste between windows; close windows and quit normally.
6. Repeat file operations in a non-ASCII directory/user path and check the
   display scaling settings intended to be supported.

Record failed/unsupported cases in the release notes. Native CI and Wine tests
are separate from this interactive check. Toolkit shutdown-leak diagnostics
retain their existing diagnostic status.

## Publish after review

After committing fixes, passing required CI and approving the candidate, tag
the exact verified commit `v1.0.0-rc1`. Attach the matching source archive,
Windows Release ZIP, manifests and checksums to a GitHub **prerelease**, using
`RELEASE-NOTES.md` as its description. Preserve CI package inventories.
No automated publication or signing is configured. Promotion to 1.0.0 needs
a new version update, tests, packages and tag.
