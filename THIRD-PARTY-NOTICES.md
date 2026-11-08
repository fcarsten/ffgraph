# Third-party notices

ffgraph preserves historical author notices in its sources. The project license
is in `LICENSE` (GNU GPL version 3); retained GNU library/container notices are
in `container/COPYING`. The legacy Tk-derived code has its notice in
`misc/license.terms`. Converted image assets retain their historical provenance;
the conversion manifest is `tests/image_assets_manifest.tsv`.

Installed packages include these texts under `share/licenses/ffgraph` (or the
configured CMake data directory). Historical notices are retained; modernization
does not assign new authorship to the original algorithms, code or artwork.

Windows packages also bundle native runtime components. Their upstream notice
snapshots are included in `licenses/` and installed with the application:

| Component | Typical bundled files | Notice |
| --- | --- | --- |
| Tcl 8.6 | `tcl86.dll`, `lib/tcl8.6` | `licenses/tcl.txt` |
| Tk 8.6 / ttk | `tk86.dll`, `lib/tk8.6` | `licenses/tk.txt` |
| GCC runtime / libstdc++ | `libgcc_s_seh-1.dll`, `libstdc++-6.dll` | `LICENSE` and `licenses/gcc-runtime.txt` |
| MinGW-w64 runtime | Runtime code linked into executables/DLLs | `licenses/mingw.txt`, `licenses/mingw-runtime.txt` |
| winpthreads | `libwinpthread-1.dll` | `licenses/winpthreads.txt` |
| zlib | `zlib1.dll` | `licenses/zlib.txt` |

Where available, Windows installation additionally copies the actual toolchain
package notices from `share/licenses` into `toolchain/` under the notice
directory. The release manifest records every bundled file's SHA256 so the
binary contents can be identified. Review additional runtime dependencies if a
different toolchain or Tcl/Tk build is used. Linux packages use system Tcl/Tk,
X11 and optional Xpm rather than bundling their shared libraries.

Notice snapshot provenance (verbatim upstream files retrieved 8 November 2026):

- [Tcl 8.6.16](https://github.com/tcltk/tcl/blob/core-8-6-16/license.terms)
- [Tk 8.6.16](https://github.com/tcltk/tk/blob/core-8-6-16/license.terms)
- [GCC runtime exception 3.1](https://github.com/gcc-mirror/gcc/blob/releases/gcc-13.3.0/COPYING.RUNTIME)
- [MinGW-w64](https://github.com/mingw-w64/mingw-w64/blob/v12.0.0/COPYING)
- [MinGW-w64 runtime](https://github.com/mingw-w64/mingw-w64/blob/v12.0.0/COPYING.MinGW-w64-runtime/COPYING.MinGW-w64-runtime.txt)
- [winpthreads](https://github.com/mingw-w64/mingw-w64/blob/v12.0.0/mingw-w64-libraries/winpthreads/COPYING)
- [zlib 1.3.1](https://github.com/madler/zlib/blob/v1.3.1/LICENSE)

These snapshot versions identify the notice sources, not a dependency lock.
Use the release manifest and CI package inventory to identify the build used.
Publish the matching ffgraph source archive and build instructions with binary
releases. Upstream component sources are available from the linked projects;
MSYS2 package recipes are at <https://github.com/msys2/MINGW-packages>.
