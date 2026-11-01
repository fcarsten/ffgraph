# ffgraph — Priority 1 — memory-safety and container corrections

**Historical artifact:** `ffgraph-priority1-fixed.zip`
**Category:** Priority 1 / archive release
**Artifact availability:** retained in the prior conversation; not bundled with this docs-only patch.

> **Historical retrospective — prepared 1 November 2026.** This note documents a patch produced in the earlier ffgraph uplift conversation. It is not a new code change or evidence of fresh execution of that patch. File paths and historical verification are reconstructed from the retained patch/archive and conversation; the current checkout may contain subsequent fixes (including Passes 8 and 9).

## Scope and rationale

This was the first corrective release after the initial static audit. It targeted immediately actionable memory-safety and container defects without changing the Tcl/Tk interface or graph file formats. **It was supplied as a complete source ZIP, not a Git patch.**

## Implemented changes (confirmed by comparing the original and fixed archives)

1. `fgraphic/FFTcl3DWindow.cc`: changed two five-byte node/label identifier buffers to `std::string`; changed a variable-length local array to `std::vector`; replaced an incompatible `qsort` comparator with `std::sort`; guarded empty sort results. This eliminates fixed-length identifier overflow and comparator undefined behaviour in the 3D hide-lines path.
2. `fgraphic/FFTclBasicWindow.cc`: changed selection export from NUL-writing `strcpy` to length-checked `memcpy`, explicitly validating offset, buffer and maximum byte count. This follows the Tk byte-count callback contract.
3. `misc/tkImgPmap.c`: checked seek, size and read results, allocated one extra byte and explicitly NUL-terminated XPM file data before passing it to the Xpm parser.
4. `include/container/MPlex.h`: made `del_low` and `del_high` remove the first/last *present* element even when logically deleted holes exist at the boundaries; changed append/prepend to snapshot input elements so self-append/prepend terminates.
5. `container/MFPlex.cc`: applied the same snapshot-based append/prepend protection to the base implementation.
6. Added `tests/test_mplex_priority1.cpp`, covering deletion with boundary holes and self-append/prepend.

## Files and archival caveat

Changed source: `container/MFPlex.cc`, `fgraphic/FFTcl3DWindow.cc`, `fgraphic/FFTclBasicWindow.cc`, `include/container/MPlex.h`, `misc/tkImgPmap.c`; new test: `tests/test_mplex_priority1.cpp`.

**Important:** Archive comparison also found an unrelated change to `ffgraph3D`: the original archive held a small textual launcher reference while the fixed archive contained an ELF binary. This was not one of the intended fixes. Do **not** treat the entire fixed ZIP as a minimal source patch; keep generated binaries and other unmodified files out of any reconstructed diff.

## Verification evidence and boundaries

The assistant reported passing the existing MPlex/container standalone tests and new boundary/self-append tests. A full Tk/Xpm application build was **not** verified locally. The user subsequently reported that the updated code worked. Do not infer that all GUI paths or memory-safety cases were exercised.

## Suggested ongoing checks

Run the container regression suite under Debug and ASan/UBSan, exercise selection export with exact-size buffers, open XPM assets, and test 3D scenes with identifiers longer than four characters. Later Pass 8 work extended the MPlex/Pix contracts; consult those newer notes for current semantics.

## Reproducing a minimal patch

Use source-aware comparison of the original and corrected ZIPs, excluding compiled binaries, rather than copying the entire release archive into a current repository.
